/*
 * app_gpd_demo.c - this project's one-off test behaviour: wires up the
 * reusable gpd/ library (see src/gpd/gpd.h) with this test's configuration
 * and schedules the periodic GP command. Kept separate from gpd/ so that
 * library stays reusable for a future GPD device with different commands,
 * timing, or security settings.
 */
#include "zcl_include.h"
#include "gp.h"
#include "gpd/gpd.h"
#include "app_main.h"

#define APP_GPD_PERIOD_MS 10000

/* Well-known default GPD security key "ZigBeeAlliance09", used by many
 * commercial GPDs when no individual/derived key is provisioned. */
static const u8 APP_GPD_DEMO_SEC_KEY[16] = {
    'Z', 'i', 'g', 'B', 'e', 'e', 'A', 'l', 'l', 'i', 'a', 'n', 'c', 'e', '0', '9'
};

/* Arbitrary fixed SrcID for this test device. */
#define APP_GPD_DEMO_SRC_ID     0x87654321

/* GP spec device id: On/Off Switch. */
#define APP_GPD_DEMO_DEVICE_ID  0x02

/* The coordinator and the local sniffer are configured for channel 11. */
#define APP_GPD_DEMO_CHANNEL    11

static int app_gpd_demo_periodicCb(void *arg)
{
    gpd_sendCommand(GPDF_CMD_ID_ONOFF_TOGGLE, NULL, 0);
    return APP_GPD_PERIOD_MS;
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

    /* Send commissioning once, then schedule periodic application commands.
     * Each broadcast frame is submitted independently to the MAC. */
    gpd_sendCommissioning(APP_GPD_DEMO_DEVICE_ID);

    TL_ZB_TIMER_SCHEDULE(app_gpd_demo_periodicCb, NULL, APP_GPD_PERIOD_MS);
}
