/*
 * gpd_frame.c - hand-builds the GPDF NWK header and CCM* MIC, then hands the
 * request to the local MAC-level CGP adapter in gpd_mac.c.
 *
 * The request is stored in a normal SDK buffer and later converted to
 * MCPS-DATA.request by gpd_mac.c. Keeping it asynchronous is important:
 * the MAC confirmation owns the buffer until transmission has completed.
 */
#include "zcl_include.h"
#include "gp.h"
#include "gpd_internal.h"

gpd_state_t g_gpd;

/* Max ASDU (gpdCmdId + payload) this library builds a frame for - callers
 * only ever pass a couple of bytes of payload. */
#define GPD_GPDU_MAX_ASDU_LEN   8

static void gpd_transmit(u8 cmdId, const u8 *payload, u8 payloadLen)
{
    zb_buf_t *zbuf;
    cgp_data_req_t *req;
    u8 *gpdu;
    u8 gpduLen;
    u8 asduLen = (u8)(payloadLen + 1); /* cmdId + payload */
    bool secured = (g_gpd.cfg.securityLevel != GPD_SEC_LEVEL_NONE);
    gp_nwkHdrFrameCtrl_t nwkFrameCtrl;
    gp_extNwkFrameCtrl_t extNwkFrameCtrl;
    u8 *p;
    static u8 gpMpduHandle = 0;

    if (payloadLen > GPD_GPDU_MAX_ASDU_LEN - 1) {
        return;
    }

    zbuf = zb_buf_allocate();
    if (zbuf == NULL) {
        return;
    }

    /* The request occupies the beginning of the pool buffer. The MAC
     * confirmation path receives the same buffer and releases it later. */
    req = (cgp_data_req_t *)zbuf;
    TL_SETSTRUCTCONTENT(*req, 0);
    /* The MAC builder prepends its 802.15.4 header immediately before
     * req->gpMpdu.  Put the GPDF in the buffer's tail using the same
     * allocator as the SDK, so that the prepended header cannot overwrite
     * the request fields at the beginning of zb_buf_t. */
    gpdu = (u8 *)tl_bufInitalloc(zbuf, GPD_GPDU_MAX_ASDU_LEN + 16);

    *((u8 *)&nwkFrameCtrl) = 0;
    nwkFrameCtrl.frameType = GP_NWK_FRAME_TYPE_DATA;
    nwkFrameCtrl.protocolVer = GP_ZB_PROTOCOL_VERSION;
    nwkFrameCtrl.autoCommissioning = 0;
    nwkFrameCtrl.nwkFrameCtrlExtension = 1;

    *((u8 *)&extNwkFrameCtrl) = 0;
    extNwkFrameCtrl.appId = GP_APP_ID_SRC_ID;
    extNwkFrameCtrl.securityLevel = g_gpd.cfg.securityLevel;
    extNwkFrameCtrl.securityKey = GP_SEC_SHARED_KEY;
    extNwkFrameCtrl.rxAfterTx = 0;
    extNwkFrameCtrl.direction = 0;

    p = gpdu;
    *p++ = *((u8 *)&nwkFrameCtrl);
    *p++ = *((u8 *)&extNwkFrameCtrl);
    *p++ = (u8)(g_gpd.cfg.srcId);
    *p++ = (u8)(g_gpd.cfg.srcId >> 8);
    *p++ = (u8)(g_gpd.cfg.srcId >> 16);
    *p++ = (u8)(g_gpd.cfg.srcId >> 24);

    if (secured) {
        *p++ = (u8)(g_gpd.secFrameCounter);
        *p++ = (u8)(g_gpd.secFrameCounter >> 8);
        *p++ = (u8)(g_gpd.secFrameCounter >> 16);
        *p++ = (u8)(g_gpd.secFrameCounter >> 24);
    }

    *p = cmdId;
    if (payloadLen != 0) {
        memcpy(p + 1, payload, payloadLen);
    }

    if (secured) {
        u8 mic[4];

        gpd_security_computeMic(g_gpd.secFrameCounter, gpdu, (u8)(p - gpdu),
                                 p, asduLen, mic);
        memcpy(p + asduLen, mic, sizeof(mic));
        g_gpd.secFrameCounter++;
    }

    gpduLen = (u8)((p - gpdu) + asduLen + (secured ? sizeof(u32) : 0));

    req->gpMpdu = gpdu;
    req->gpMpduLen = gpduLen;
    req->srcPanId = g_zbMacPib.panId;
    req->dstPanId = 0xffff;
    req->srcAddrMode = ADDR_MODE_NONE;
    req->dstAddrMode = ADDR_MODE_SHORT;
    req->dstAddr.shortAddr = 0xffff;
    req->gpMpduHandle = gpMpduHandle++;
    req->txOptions.useCSMACA = 1;
    req->txOptions.useMACACK = 0; /* broadcast: never acked */

    /* The local adapter is a task callback. Ownership of zbuf transfers to
     * the MAC request/confirm pipeline; it must not be freed here. */
    ev_timer_taskPost(gpd_macDataReq, zbuf, GP_TX_OFFSET);
}

void gpd_init(const gpd_config_t *cfg)
{
    g_gpd.cfg = *cfg;
    g_gpd.secFrameCounter = 0;
    g_gpd.macSeqNum = 0;
    g_gpd.inited = 1;

    /* Moves the MAC layer's own notion of the "current" channel to ours,
     * so our GPDF transmits on our target channel. */
    tl_zbMacChannelSet(cfg->channel);
}

void gpd_sendCommissioning(u8 deviceId)
{
    u8 payload[2];
    payload[0] = deviceId;
    payload[1] = 0x00; /* options: no extended options, no key, security level 0 */

    gpd_transmit(GPDF_CMD_ID_COMMISSIONING, payload, sizeof(payload));
}

void gpd_sendCommand(u8 cmdId, const u8 *payload, u8 payloadLen)
{
    gpd_transmit(cmdId, payload, payloadLen);
}
