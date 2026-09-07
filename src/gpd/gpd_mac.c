/*
 * gpd_mac.c - minimal CGP-to-MAC adapter for the standalone GPD.
 *
 * The Telink SDK exposes the CGP-DATA.request primitive only through the
 * router library. A GPD does not need that library's GP proxy or ZCL layer,
 * so this file keeps the small adapter locally: it translates our prepared
 * GPDF request to the ordinary 802.15.4 MCPS-DATA.request primitive.
 */
#include "zb_common.h"
#include "cGP_stub.h"
#include "dGP_stub.h"

#include "gpd_internal.h"

/*
 * The prebuilt ZDO service has an optional GP alias-conflict hook. There is
 * no proxy table in this firmware, so announcements must never be claimed by
 * the GPD. Keeping the hook defined also lets us use the end-device library
 * without linking the SDK GP proxy implementation.
 */
static bool gpd_deviceAnnounceCheck(u16 nwkAddr, addrExt_t ieeeAddr)
{
    (void)nwkAddr;
    (void)ieeeAddr;
    return FALSE;
}

gpDeviceAnnounceCheckCb_t g_gpDeviceAnnounceCheckCb =
    gpd_deviceAnnounceCheck;

/*
 * Translate a prepared CGP request in-place.
 *
 * The event buffer starts with the request structure. The MAC task expects
 * its MCPS request at the same address and owns the buffer until the data
 * confirmation callback releases it. Copying the CGP fields before writing
 * the MCPS structure avoids corrupting the source request during conversion.
 */
s32 gpd_macDataReq(void *arg)
{
    cgp_data_req_t cgp;
    zb_mscp_data_req_t *mcps = (zb_mscp_data_req_t *)arg;

    memcpy(&cgp, arg, sizeof(cgp));
    memset(mcps, 0, sizeof(*mcps));

    mcps->dstPanId = cgp.dstPanId;
    mcps->srcAddr.addrMode = cgp.srcAddrMode;
    memcpy(&mcps->dstAddr.addr, &cgp.dstAddr, sizeof(cgp.dstAddr));
    mcps->dstAddr.addrMode = cgp.dstAddrMode;
    mcps->msduHandle = cgp.gpMpduHandle;
    mcps->msduLength = cgp.gpMpduLen;
    mcps->msdu = cgp.gpMpdu;

    /* MCPS option bit 0 enables CSMA/CA; bit 3 requests MAC ACK. */
    if (cgp.txOptions.useCSMACA) {
        mcps->txOptions |= BIT(0);
    }
    if (cgp.txOptions.useMACACK) {
        mcps->txOptions |= BIT(3);
    }

    /* The MAC confirmation path releases the event buffer. */
    tl_zbMacMcpsDataRequestProc(arg);

    /* Returning -1 removes this one-shot timer event. */
    return -1;
}
