/*
 * gpd.h - Reusable Green Power Device (GPD) transmitter library.
 *
 * A GPD does not join a Zigbee network: it broadcasts Green Power Data
 * Frames (GPDF) as broadcast, unaddressed-source MAC frames, bypassing the
 * Zigbee NWK/ZDO/APS stack. This library hands the GPD command to the
 * stack's own GP-DATA.request primitive, gpDataReq() (zigbee/gp/dGP_stub.h,
 * exported by the router/coordinator Zigbee libs), which builds the GPDF
 * NWK header/security itself, rather than driving radio registers or a
 * hand-built GPDF directly. See gpd_frame.c's file header for the history
 * of why (two earlier approaches - raw register pokes, then the lower-level
 * cGp_dataReq() - were tried and ruled out).
 *
 * KNOWN ISSUE: gpDataReq() retransmits each request continuously on its own
 * (confirmed: our own code calls it exactly once per gpd_sendCommissioning()/
 * gpd_sendCommand()) - no combination of txOptions or an explicit
 * gpTxQueueMaintenceClear() call found so far produces a single bounded
 * transmission. The resulting frames ARE correctly recognized by z2m's
 * zh:controller:greenpower (matching SrcID, correct COMMISSIONING parse) -
 * this is a genuine, spec-conformant GPDF, just sent far more often than
 * intended. Likely needs the real zigbee/gp/gp.c module properly
 * initialized (gp_init()/gpStubCbInit(), excluded from this build) to
 * manage the retry/queue state correctly - not yet done here.
 *
 * The caller is responsible for having already brought up the radio HW
 * (zb_init(), or equivalent MAC/PHY init) before calling gpd_init(). This
 * library does not itself require bdb_init()/network join - gpDataReq() is
 * a MAC-layer primitive, independent of NWK/ZDO join state.
 */
#ifndef SRC_GPD_GPD_H_
#define SRC_GPD_GPD_H_

#include "tl_common.h"

/* GP security levels, matching the Zigbee GP spec / cGP_stub.h GP_SEC_LEVEL_*.
 * NOT currently honored by gpd_sendCommand(): gpDataReq() applies whatever
 * security policy the (uninitialized, in this build) GP module defaults to,
 * with no per-call security level parameter exposed. Kept here for a future
 * version that either initializes the real GP module or reintroduces
 * manual GPDF construction with explicit security-level control. */
typedef enum {
    GPD_SEC_LEVEL_NONE       = 0,
    GPD_SEC_LEVEL_FC_MIC     = 2,
    GPD_SEC_LEVEL_FC_MIC_ENC = 3,
} gpd_secLevel_e;

typedef struct {
    u32 srcId;                  /* GPD SrcID (application id 0b000 addressing) */
    u8  securityKey[16];        /* Not currently used - see gpd_secLevel_e note */
    gpd_secLevel_e securityLevel; /* Not currently used - see gpd_secLevel_e note */
    u8  channel;                 /* IEEE 802.15.4 channel, 11-26 */
    u8  autoCommissioning;       /* Not currently used (gpDataReq() builds the
                                  * NWK frame control itself) */
} gpd_config_t;

/* One-time setup: stores the config and moves the MAC layer's own notion of
 * the "current" channel to cfg->channel (via the stack's rf_setChannel())
 * since gpDataReq() sends on whatever channel the MAC is currently
 * configured for - it takes no per-call channel argument. */
void gpd_init(const gpd_config_t *cfg);

/* Sends a minimal GP Commissioning frame (command 0xE0) with no extended
 * options (options byte = 0x00). */
void gpd_sendCommissioning(u8 deviceId);

/* Sends an arbitrary GPD command frame (e.g. On/Off Toggle,
 * GPDF_CMD_ID_ONOFF_TOGGLE = 0x22, with no payload). */
void gpd_sendCommand(u8 cmdId, const u8 *payload, u8 payloadLen);

#endif /* SRC_GPD_GPD_H_ */
