/*
 * gpd_frame.c - sends GP commands via the stack's own GP-DATA.request
 * primitive, gpDataReq() (zigbee/gp/dGP_stub.h), rather than hand-building
 * the GPDF NWK header/security and driving the lower-level cGp_dataReq()
 * (zigbee/gp/cGP_stub.h) or raw radio registers directly.
 *
 * History (why this is the third implementation of this file):
 *   1. Hand-built 802.15.4 MAC header + GPDF NWK header/MIC, driving
 *      ZB_RADIO_TRX_SWITCH/ZB_RADIO_TX_START directly. Byte-correct but
 *      never reached the air on real hardware, while this exact
 *      board/toolchain's normal MAC traffic (BeaconReq during BDB network
 *      steering) transmitted fine - the closed-source MAC transmit path
 *      does something beyond a plain ZB_RADIO_TX_START that isn't
 *      reproducible from the raw register API.
 *   2. Hand-built GPDF NWK header/MIC, handed to cGp_dataReq() (a MAC-level
 *      raw send primitive). This DID reach the air and z2m's
 *      zh:controller:greenpower correctly decoded our SrcID from the
 *      Commissioning frame - but cGp_dataReq() queues the request with no
 *      bounded lifetime, so it retransmitted forever (verified: still
 *      flooding after 15+ seconds from a single call, only getting
 *      faster/worse with useCSMACA disabled).
 *   3. (this version) gpDataReq() is the proper GP-DATA.request service
 *      primitive: it takes an explicit gpTxQueueEntryLifetime (dGP_stub.h)
 *      bounding how long the queued entry is retried, and builds the GPDF
 *      NWK header/security itself from appId/gpdId - no more hand-rolled
 *      NWK header or CCM* MIC computation in this file.
 */
#include "zcl_include.h"
#include "gp.h"
#include "gpd_internal.h"

gpd_state_t g_gpd;

/* How long a queued GPDF is retried before being dropped (ms). Bounds the
 * runaway retransmission seen with cGp_dataReq() (see file header). */
#define GPD_TX_QUEUE_LIFETIME_MS   200

static void gpd_transmit(u8 cmdId, const u8 *payload, u8 payloadLen)
{
    gp_data_req_t req;
    static u8 gpepHandle = 0;

    TL_SETSTRUCTCONTENT(req, 0);
    req.gpdAsdu = (u8 *)payload;
    req.gpdAsduLen = payloadLen;
    req.gpTxQueueEntryLifetime = GPD_TX_QUEUE_LIFETIME_MS;
    req.gpdId.srcId = g_gpd.cfg.srcId;
    req.txOptions.useGpTxQueue = 0; /* immediate one-shot send, no persistent queue */
    req.txOptions.useCSMACA = 1;
    req.txOptions.useMACACK = 0; /* broadcast: never acked */
    req.action = 1;
    req.appId = GP_APP_ID_SRC_ID;
    req.endpoint = 0;
    req.gpdCmdId = cmdId;
    req.gpepHandle = gpepHandle++;

    gpDataReq(&req);

    /* KNOWN ISSUE: gpDataReq() retransmits this frame continuously on its
     * own after this single call (confirmed via gpd_transmit_callCount -
     * our own code only calls this once per gpd_sendCommissioning()/
     * gpd_sendCommand()). Neither txOptions.useGpTxQueue=0 nor an explicit
     * gpTxQueueMaintenceClear() (immediately or after a short delay)
     * produced a single bounded transmission - clearing immediately
     * suppresses the send entirely (0 packets on air), and delaying it
     * still didn't stop the flood. z2m's zh:controller:greenpower does
     * correctly recognize the resulting frames (matching SrcID, correct
     * COMMISSIONING parse) - this is a real, working GP frame, just sent
     * far more often than intended. Likely needs the real zigbee/gp/gp.c
     * module properly initialized (gp_init()/gpStubCbInit(), excluded from
     * this build) to manage the queue/retry state correctly. */
}

void gpd_init(const gpd_config_t *cfg)
{
    g_gpd.cfg = *cfg;
    g_gpd.secFrameCounter = 0;
    g_gpd.macSeqNum = 0;
    g_gpd.inited = 1;

    /* Moves the MAC layer's own notion of the "current" channel to ours,
     * so gpDataReq() (which sends on whatever channel the MAC is currently
     * configured for - it has no per-call channel argument) transmits on
     * our target channel. */
    rf_setChannel(cfg->channel);
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
