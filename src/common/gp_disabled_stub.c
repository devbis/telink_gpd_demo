/* Green Power is disabled, but the SDK's prebuilt ZDO code still references
 * this optional Device Announce callback symbol. */

#include "tl_common.h"

typedef bool (*gpDeviceAnnounceCheckCb_t)(u16 sinkNwkAddr, addrExt_t sinkIeeeAddr);

/* gp_base.o also keeps these optional Green Power attributes as external
 * references. Keep them in the disabled configuration, but never expose or
 * use them from the application. */
u8 zclGpAttr_gpSharedSecKeyType = 0;
u8 zclGpAttr_gpSharedSecKey[16] = {0};

static bool gp_disabled_device_announce_check(u16 sinkNwkAddr, addrExt_t sinkIeeeAddr)
{
    (void)sinkNwkAddr;
    (void)sinkIeeeAddr;
    return FALSE;
}

gpDeviceAnnounceCheckCb_t g_gpDeviceAnnounceCheckCb = gp_disabled_device_announce_check;

/* zigbee/zcl/zll_commissioning/zcl_zll_commissioning.c is excluded from this
 * build (TOUCHLINK_SUPPORT is off), but the prebuilt Zigbee stack lib's
 * NWK inter-PAN data path (tl_zbNwkInterPanDataReq) references this global
 * unconditionally. Normally defined as `u8 deviceInfoRsp = 0;` in that file. */
u8 deviceInfoRsp = 0;
