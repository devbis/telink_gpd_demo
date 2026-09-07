/* zigbee/zcl/zll_commissioning/zcl_zll_commissioning.c is excluded from this
 * build (TOUCHLINK_SUPPORT is off), but the prebuilt Zigbee stack lib's
 * NWK inter-PAN data path (tl_zbNwkInterPanDataReq) references this global
 * unconditionally. Normally defined as `u8 deviceInfoRsp = 0;` in that file. */

#include "tl_common.h"

u8 deviceInfoRsp = 0;
