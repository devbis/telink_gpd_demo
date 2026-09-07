/**********************************************************************
 * INCLUDES
 */
#include "tl_common.h"
#include "zcl_include.h"

#include "app_main.h"

/**********************************************************************
 * LOCAL VARIABLES
 */
#ifdef ZCL_IDENTIFY
static ev_timer_event_t *identifyTimerEvt = NULL;
#endif

/**********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      app_zclProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message. This endpoint only
 *          exposes Basic/Identify, so foundation Read/Write/Report/Default
 *          Response messages are not expected in practice; this is a no-op
 *          placeholder required by zcl_init().
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  None
 */
void app_zclProcessIncomingMsg(zclIncoming_t *pInHdlrMsg)
{
    (void)pInHdlrMsg;
}

#ifdef ZCL_BASIC
/*********************************************************************
 * @fn      app_basicCb
 *
 * @brief   Handler for ZCL Basic command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t app_basicCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload)
{
	(void)pAddrInfo;
	(void)cmdPayload;

	if(cmdId == ZCL_CMD_BASIC_RESET_FAC_DEFAULT){
		//Reset all the attributes of all its clusters to factory defaults
	}

	return ZCL_STA_SUCCESS;
}
#endif	/* ZCL_BASIC */

#ifdef ZCL_IDENTIFY
int app_zclIdentifyTimerCb(void *arg)
{
	if(g_zcl_identifyAttrs.identifyTime <= 0){
		identifyTimerEvt = NULL;
		return -1;
	}
	g_zcl_identifyAttrs.identifyTime--;
	return 0;
}

void app_zclIdentifyTimerStop(void)
{
	if(identifyTimerEvt){
		TL_ZB_TIMER_CANCEL(&identifyTimerEvt);
	}
}

/*********************************************************************
 * @fn      app_zclIdentifyCmdHandler
 *
 * @brief   Handler for ZCL Identify command.
 *
 * @param   endpoint
 * @param   srcAddr
 * @param   identifyTime - identify time
 *
 * @return  None
 */
void app_zclIdentifyCmdHandler(u8 endpoint, u16 srcAddr, u16 identifyTime)
{
	(void)endpoint;
	(void)srcAddr;

	g_zcl_identifyAttrs.identifyTime = identifyTime;

	if(identifyTime == 0){
		app_zclIdentifyTimerStop();
	}else{
		if(!identifyTimerEvt){
			identifyTimerEvt = TL_ZB_TIMER_SCHEDULE(app_zclIdentifyTimerCb, NULL, 1000);
		}
	}
}

/*********************************************************************
 * @fn      app_identifyCb
 *
 * @brief   Handler for ZCL Identify command.
 *
 * @param   pAddrInfo
 * @param   cmdId
 * @param   cmdPayload
 *
 * @return  status_t
 */
status_t app_identifyCb(zclIncomingAddrInfo_t *pAddrInfo, u8 cmdId, void *cmdPayload)
{
	if(pAddrInfo->dstEp == APP_ENDPOINT1){
		if(pAddrInfo->dirCluster == ZCL_FRAME_CLIENT_SERVER_DIR){
			switch(cmdId){
				case ZCL_CMD_IDENTIFY:
					app_zclIdentifyCmdHandler(pAddrInfo->dstEp, pAddrInfo->srcAddr, ((zcl_identifyCmd_t *)cmdPayload)->identifyTime);
					break;
				default:
					break;
			}
		}
	}

	return ZCL_STA_SUCCESS;
}
#endif	/* ZCL_IDENTIFY */
