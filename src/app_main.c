#include "app_main.h"

app_ctx_t g_appCtx = {
        .bdbFBTimerEvt = NULL,
        .short_poll = POLL_RATE * 3,
        .long_poll = POLL_RATE * LONG_POLL,
};

/* bdb_init() is intentionally never called (see user_init()): this device
 * is a Green Power Device using cGp_dataReq() (a MAC-layer primitive,
 * independent of NWK/ZDO join state) to transmit, so it never needs to join
 * a Zigbee network. The ZDO indication callbacks below are still registered
 * because zb_init() wires them up unconditionally. */
const zdo_appIndCb_t appCbLst = {
    bdb_zdoStartDevCnf,//start device cnf cb
    NULL,//reset cnf cb
    NULL,//device announce indication cb
    app_leaveIndHandler,//leave ind cb
    app_leaveCnfHandler,//leave cnf cb
    app_nwkUpdateIndicateHandler,//nwk update ind cb
    NULL,//permit join ind cb
    NULL,//nlme sync cnf cb
    NULL,//tc join ind cb
    NULL,//tc detects that the frame counter is near limit
};

/**********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      stack_init
 *
 * @brief   This function initialize the ZigBee stack and related profile. If HA/ZLL profile is
 *          enabled in this application, related cluster should be registered here.
 *
 * @param   None
 *
 * @return  None
 */
void stack_init(void)
{
    /* Initialize ZB stack (MAC/PHY bring-up) */
    zb_init();

    /* Register stack CB */
    zb_zdoCbRegister((zdo_appIndCb_t *)&appCbLst);
}

/*********************************************************************
 * @fn      user_app_init
 *
 * @brief   This function initialize the application(Endpoint) information for this node.
 *
 * @param   None
 *
 * @return  None
 */
void user_app_init(void)
{

#if ZCL_POLL_CTRL_SUPPORT
    af_powerDescPowerModeUpdate(POWER_MODE_RECEIVER_COMES_PERIODICALLY);
#else
    af_powerDescPowerModeUpdate(POWER_MODE_RECEIVER_COMES_WHEN_STIMULATED);
#endif

    /* Initialize ZCL layer */
    /* Register Incoming ZCL Foundation command/response messages */
    zcl_init(app_zclProcessIncomingMsg);

    /* register endPoint */
    af_endpointRegister(APP_ENDPOINT1, (af_simple_descriptor_t *)&app_ep1Desc, zcl_rx_handler, NULL);

    zcl_reportingTabInit();

    /* Register ZCL specific cluster information */
    zcl_register(APP_ENDPOINT1, APP_EP1_CB_CLUSTER_NUM, (zcl_specClusterInfo_t *)g_appEp1ClusterList);

    /* Start the Green Power Device transmitter: sends the commissioning
     * burst now, then a periodic GP command every 10 seconds. */
    app_gpd_demo_init();
}

void app_task(void) {
    if(bdb_isIdle()) {
        report_handler();
    }
}

extern volatile uint16_t T_evtExcept[4];

static void appSysException(void) {

#if UART_PRINTF_MODE
    printf("app_sysException, line: %d, event: %d, reset\r\n", T_evtExcept[0], T_evtExcept[1]);
#endif

    SYSTEM_RESET();
}

/*********************************************************************
 * @fn      user_init
 *
 * @brief   User level initialization code.
 *
 * @param   isRetention - if it is waking up with ram retention.
 *
 * @return  None
 */
void user_init(bool isRetention) {

    if (!isRetention) {
        populate_sw_build();
        populate_date_code();
    }

    print_boot_message();

#ifdef CHECK_BOOTLOADER
    bootloader_check();
#endif

    /* Initialize Stack */
    stack_init();

    /* Initialize user application */
    user_app_init();

    /* Register except handler for test */
    sys_exceptHandlerRegister(appSysException);

    /* User's Task */
#if ZBHCI_EN
    zbhciInit();
    ev_on_poll(EV_POLL_HCI, zbhciTask);
#endif
    ev_on_poll(EV_POLL_IDLE, app_task);

    /* No bdb_init() call: this device never joins a Zigbee network. */
}
