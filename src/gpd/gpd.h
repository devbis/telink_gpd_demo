/*
 * gpd.h - Reusable Green Power Device (GPD) transmitter library.
 *
 * A GPD does not join a Zigbee network: it broadcasts Green Power Data
 * Frames (GPDF) as broadcast, unaddressed-source MAC frames, bypassing the
 * Zigbee NWK/ZDO/APS stack. This library hand-builds the GPDF NWK
 * header/CCM* MIC itself and hands the frame to the stack's MAC-level
 * CGP-DATA.request primitive, cGp_dataReq() (zigbee/gp/cGP_stub.h), posted
 * via a properly pool-allocated buffer - see gpd_frame.c's file header for
 * the full history of why (gpDataReq(), the higher-level GP-DATA.request
 * primitive, turned out to be a receive/relay-triggered queue, not a direct
 * send primitive at all, regardless of how it's called).
 *
 * The caller is responsible for having already brought up the radio HW
 * (zb_init(), or equivalent MAC/PHY init) and the real GP module
 * (gp_init(), zigbee/gp/gp.c) before calling gpd_init(). This library does
 * not itself require bdb_init()/network join - it transmits at the MAC
 * layer, independent of NWK/ZDO join state.
 */
#ifndef SRC_GPD_GPD_H_
#define SRC_GPD_GPD_H_

#include "tl_common.h"

/* GP security levels, matching the Zigbee GP spec / cGP_stub.h GP_SEC_LEVEL_*.
 * GPD_SEC_LEVEL_NONE sends a clear-text frame (no frame counter, no MIC).
 * GPD_SEC_LEVEL_FC_MIC sends frame counter + CCM* MIC, computed with
 * securityKey. GPD_SEC_LEVEL_FC_MIC_ENC is NOT implemented (gpd_security.c
 * only computes a MIC, it doesn't encrypt the payload) - treated the same
 * as GPD_SEC_LEVEL_FC_MIC. */
typedef enum {
    GPD_SEC_LEVEL_NONE       = 0,
    GPD_SEC_LEVEL_FC_MIC     = 2,
    GPD_SEC_LEVEL_FC_MIC_ENC = 3,
} gpd_secLevel_e;

typedef struct {
    u32 srcId;                  /* GPD SrcID (application id 0b000 addressing) */
    u8  securityKey[16];        /* Used to compute the CCM* MIC - see gpd_secLevel_e */
    gpd_secLevel_e securityLevel;
    u8  channel;                 /* IEEE 802.15.4 channel, 11-26 */
    u8  autoCommissioning;       /* Not currently used - this library always
                                  * sends autoCommissioning=0 in the NWK frame
                                  * control. */
} gpd_config_t;

/* One-time setup: stores the config and moves the MAC layer's own notion of
 * the "current" channel to cfg->channel (via the stack's
 * tl_zbMacChannelSet())
 * since our transmit path sends on whatever channel the MAC is currently
 * configured for - it takes no per-call channel argument. */
void gpd_init(const gpd_config_t *cfg);

/* Sends a minimal GP Commissioning frame (command 0xE0) with no extended
 * options (options byte = 0x00). */
void gpd_sendCommissioning(u8 deviceId);

/* Sends an arbitrary GPD command frame (e.g. On/Off Toggle,
 * GPDF_CMD_ID_ONOFF_TOGGLE = 0x22, with no payload). */
void gpd_sendCommand(u8 cmdId, const u8 *payload, u8 payloadLen);

#endif /* SRC_GPD_GPD_H_ */
