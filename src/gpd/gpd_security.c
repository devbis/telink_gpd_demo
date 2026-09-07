/*
 * gpd_security.c - GP security level 2 (frame counter + MIC, no payload
 * encryption) support: CCM* nonce construction and MIC computation.
 *
 * Uses the SDK's generic aes_ccmAuthTran() (zigbee/ss/security_service.h),
 * which is exported from the router/coordinator Zigbee stack lib and needs
 * no NWK key-table setup - confirmed against its disabled reference call
 * site in apps/common/module_test.c (MODULE_TEST_CCM).
 *
 * Nonce layout, per the Zigbee GP spec's AES-CCM* nonce for GPD frames
 * (cross-checked against the NXP zigate coordinator's GP key-transport
 * nonce construction, which uses the same shape for the IEEE/SrcID case):
 *   SrcID (4 bytes, little-endian), repeated twice (8 bytes)
 *   || Security Frame Counter (4 bytes, little-endian)
 *   || Security Control byte (1 byte, 0x05)
 * = 13 raw bytes; aes_ccmAuthTran() does CCM* L/M padding internally.
 */
#include "zb_common.h"
#include "gpd_internal.h"

#define GPD_NONCE_LEN           13
#define GPD_NONCE_SEC_CONTROL   0x05

static void gpd_security_buildNonce(u32 srcId, u32 frameCounter, u8 *nonceOut)
{
    nonceOut[0] = (u8)(srcId & 0xFF);
    nonceOut[1] = (u8)((srcId >> 8) & 0xFF);
    nonceOut[2] = (u8)((srcId >> 16) & 0xFF);
    nonceOut[3] = (u8)((srcId >> 24) & 0xFF);
    nonceOut[4] = nonceOut[0];
    nonceOut[5] = nonceOut[1];
    nonceOut[6] = nonceOut[2];
    nonceOut[7] = nonceOut[3];
    nonceOut[8] = (u8)(frameCounter & 0xFF);
    nonceOut[9] = (u8)((frameCounter >> 8) & 0xFF);
    nonceOut[10] = (u8)((frameCounter >> 16) & 0xFF);
    nonceOut[11] = (u8)((frameCounter >> 24) & 0xFF);
    nonceOut[12] = GPD_NONCE_SEC_CONTROL;
}

u8 gpd_security_computeMic(u32 frameCounter, const u8 *aad, u8 aadLen,
                            const u8 *asdu, u16 asduLen, u8 *micOut)
{
    u8 nonce[GPD_NONCE_LEN];

    gpd_security_buildNonce(g_gpd.cfg.srcId, frameCounter, nonce);

    return aes_ccmAuthTran(4, (u8 *)g_gpd.cfg.securityKey, nonce,
                            (u8 *)asdu, asduLen, (u8 *)aad, aadLen, micOut);
}
