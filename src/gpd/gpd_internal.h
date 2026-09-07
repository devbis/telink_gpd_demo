/*
 * gpd_internal.h - state and helpers shared between gpd_frame.c and
 * gpd_security.c. Not part of the public API (see gpd.h).
 */
#ifndef SRC_GPD_GPD_INTERNAL_H_
#define SRC_GPD_GPD_INTERNAL_H_

#include "gpd.h"

typedef struct {
    gpd_config_t cfg;
    u32 secFrameCounter;
    u8  macSeqNum;
    u8  inited;
} gpd_state_t;

extern gpd_state_t g_gpd;

/* gpd_security.c: computes a 4-byte CCM* MIC over `asdu` (the plaintext GPD
 * command id + payload), authenticating `aad` (the clear GPDF NWK header
 * bytes) as additional data. `frameCounter` is folded into the CCM* nonce
 * together with the configured SrcID. Returns RET_OK (per aes_ccmAuthTran)
 * on success. */
u8 gpd_security_computeMic(u32 frameCounter, const u8 *aad, u8 aadLen,
                            const u8 *asdu, u16 asduLen, u8 *micOut);

#endif /* SRC_GPD_GPD_INTERNAL_H_ */
