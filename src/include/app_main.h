#ifndef SRC_INCLUDE_APP_MAIN_H_
#define SRC_INCLUDE_APP_MAIN_H_

#include <stdint.h>

#include "tl_common.h"
#include "zcl_include.h"
#include "bdb.h"
#include "ota.h"
#include "gp.h"


#include "app_utility.h"
#include "app_endpoint_cfg.h"
#include "app_bootloader.h"
#include "gpd/gpd.h"
#include "app_gpd_demo.h"


typedef struct {
    uint8_t keyType; /* CERTIFICATION_KEY or MASTER_KEY key for touch-link or distribute network
                        SS_UNIQUE_LINK_KEY or SS_GLOBAL_LINK_KEY for distribute network */
    uint8_t key[16]; /* the key used */
} app_linkKey_info_t;

typedef struct {
    ev_timer_event_t *bdbFBTimerEvt;

    uint32_t short_poll;
    uint32_t long_poll;
    uint32_t current_poll;

    uint32_t timeout;

    app_linkKey_info_t tcLinkKey;
} app_ctx_t;

extern app_ctx_t g_appCtx;

extern const zcl_specClusterInfo_t g_appEp1ClusterList[];
extern const af_simple_descriptor_t app_ep1Desc;

void app_zclProcessIncomingMsg(zclIncoming_t *pInHdlrMsg);

status_t app_basicCb(zclIncomingAddrInfo_t *pAddrInfo, uint8_t cmdId, void *cmdPayload);
status_t app_identifyCb(zclIncomingAddrInfo_t *pAddrInfo, uint8_t cmdId, void *cmdPayload);

void app_leaveCnfHandler(nlme_leave_cnf_t *pLeaveCnf);
void app_leaveIndHandler(nlme_leave_ind_t *pLeaveInd);
bool app_nwkUpdateIndicateHandler(nwkCmd_nwkUpdate_t *pNwkUpdate);


#endif /* SRC_INCLUDE_APP_MAIN_H_ */
