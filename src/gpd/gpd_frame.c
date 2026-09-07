/*
 * gpd_frame.c - hand-builds the GPDF NWK header + CCM* MIC and hands the
 * frame to cGp_dataReq() (zigbee/gp/cGP_stub.h), the MAC-level CGP-DATA.
 * request primitive, instead of the higher-level gpDataReq()
 * (zigbee/gp/dGP_stub.h).
 *
 * History (why this is the fourth implementation of this file):
 *   1. Hand-built 802.15.4 MAC header + GPDF NWK header/MIC, driving
 *      ZB_RADIO_TRX_SWITCH/ZB_RADIO_TX_START directly. Byte-correct but
 *      never reached the air on real hardware.
 *   2/3. cGp_dataReq()/gpDataReq() called directly with a stack-local
 *      request struct, instead of a buffer from zb_buf_allocate(). Both are
 *      task-callback-style functions (`void fn(void *arg)`, meant to be
 *      posted via tl_zbTaskPost()/ev_timer_taskPost() with a pool-allocated
 *      zb_buf_t, not called synchronously with a stack struct - the
 *      recovered source of the precompiled router lib (see
 *      llvm-tc32-arm-based/libzigbee/src/{cGP_stub,dGP_stub}.c) confirms
 *      both `zb_buf_free((zb_buf_t *)arg)` and the buffer being retained
 *      for later async processing). Passing a stack struct where a pool
 *      buffer's header (zb_buf_hdr_t/next, right after
 *      buf[ZB_BUF_SIZE]) is expected is undefined behaviour once anything
 *      downstream treats `arg` as a real zb_buf_t - this is the most
 *      likely explanation for every inconsistent symptom seen across this
 *      file's history (runaway retransmission, total silence, and even an
 *      unrelated-looking early-boot hang once, all from byte-identical
 *      source): garbage buffer-pool bookkeeping is undefined behaviour, not
 *      a deterministic bug.
 *   4. (this version) A real zb_buf_t is allocated via zb_buf_allocate().
 *      cgp_data_req_t is aliased onto its first bytes (matching the
 *      convention above - the pointer handed to cGp_dataReq() *is* the
 *      zb_buf_t*), the GPDF bytes are placed further into the same
 *      buffer's data area, and the request is posted via
 *      tl_zbTaskPost(cGp_dataReq, buf) rather than called directly.
 *      cGp_dataReq() (per the recovered source) unconditionally calls
 *      tl_zbMacMcpsDataRequestProc() - a real, single MAC data request, no
 *      internal retry/queue logic of its own - and the buffer is freed
 *      later by the MAC data-confirm path (cGpDataCnfHandler() ->
 *      zb_buf_free()), not by this file.
 *
 *      Note: gpDataReq() (the higher-level GP-DATA.request primitive used
 *      in revision 3) was separately found, from the same recovered source,
 *      to not be a direct-send primitive at all even when called
 *      correctly: it only stores the request in a single-entry queue
 *      (gpTxQueue) for a *later* gp_gpdfTransSchedule() call to pick up -
 *      and that function is only ever invoked reactively, from a GPDF
 *      Maintenance frame actually received over the air (e.g. a sink's
 *      channel request). A one-way transmit-only GPD like this one has no
 *      such inbound trigger, so gpDataReq() would never transmit anything
 *      here regardless of the stack-vs-buffer issue above.
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

    /* cGp_dataReq()/the MAC data-confirm path treat the pointer they're
     * given as the zb_buf_t itself (see file header) - alias the request
     * struct onto the buffer's own data area rather than a local variable. */
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

    /* cGp_dataReq is a task callback (see file header) - post it, don't
     * call it directly. Ownership of zbuf transfers to the MAC data
     * request/confirm pipeline; it must not be freed here. Scheduled via
     * ev_timer_taskPost() with GP_TX_OFFSET (20ms, dGP_stub.h), matching
     * the SDK's own gp_gpdfGenerate() - not posted as an immediate
     * tl_zbTaskPost() task. */
    ev_timer_taskPost((ev_timer_callback_t)cGp_dataReq, zbuf, GP_TX_OFFSET);
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
