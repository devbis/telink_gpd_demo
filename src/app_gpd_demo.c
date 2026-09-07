/*
 * app_gpd_demo.c - this project's one-off test behaviour: wires up the
 * reusable gpd/ library (see src/gpd/gpd.h) with this test's configuration
 * and schedules the periodic GP command. Kept separate from gpd/ so that
 * library stays reusable for a future GPD device with different commands,
 * timing, or security settings.
 */
#include "app_main.h"

/* Well-known default GPD security key "ZigBeeAlliance09", used by many
 * commercial GPDs when no individual/derived key is provisioned. */
static const u8 APP_GPD_DEMO_SEC_KEY[16] = {
    'Z', 'i', 'g', 'B', 'e', 'e', 'A', 'l', 'l', 'i', 'a', 'n', 'c', 'e', '0', '9'
};

/* Arbitrary fixed SrcID for this test device. */
#define APP_GPD_DEMO_SRC_ID     0x87654321

/* GP spec device id: On/Off Switch. */
#define APP_GPD_DEMO_DEVICE_ID  0x02

#define APP_GPD_DEMO_CHANNEL    25

static int app_gpd_demo_periodicCb(void *arg)
{
    gpd_sendCommand(GPDF_CMD_ID_ONOFF_TOGGLE, NULL, 0);
    return TIMEOUT_10SEC;
}

void app_gpd_demo_init(void)
{
    gpd_config_t cfg;

    TL_SETSTRUCTCONTENT(cfg, 0);
    cfg.srcId = APP_GPD_DEMO_SRC_ID;
    memcpy(cfg.securityKey, APP_GPD_DEMO_SEC_KEY, sizeof(cfg.securityKey));
    cfg.securityLevel = GPD_SEC_LEVEL_FC_MIC;
    cfg.channel = APP_GPD_DEMO_CHANNEL;
    cfg.autoCommissioning = 0;

    gpd_init(&cfg);

    /* The commissioning frame is sent once.  Periodic application commands
     * are scheduled below; cGp_dataReq() itself does not maintain a retry
     * queue for these broadcast frames. */
    gpd_sendCommissioning(APP_GPD_DEMO_DEVICE_ID);

    TL_ZB_TIMER_SCHEDULE(app_gpd_demo_periodicCb, NULL, TIMEOUT_10SEC);
}
