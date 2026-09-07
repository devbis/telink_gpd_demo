#include "app_main.h"
#include "mac_phy.h"
#include "zb_common.h"

/* Initialize the MAC/PHY services used by the GPD transmitter. */
void stack_init(void)
{
    /* zb_init also prepares the SDK buffer and timer infrastructure. */
    zb_init();
}

/* Start the application-level GPD schedule. */
void user_app_init(void)
{
    /* Start the standalone Green Power Device transmitter. */
    app_gpd_demo_init();
}

extern volatile uint16_t T_evtExcept[4];

static void appSysException(void)
{

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
/* Initialize once on cold boot and restore only the radio after retention. */
void user_init(bool isRetention)
{
    if (!isRetention) {
        stack_init();
        user_app_init();
        sys_exceptHandlerRegister(appSysException);
    } else {
        /* Deep sleep with retention keeps stack/application state in SRAM. */
        mac_phyReconfig();
    }
}
