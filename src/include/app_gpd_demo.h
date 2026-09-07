#ifndef SRC_INCLUDE_APP_GPD_DEMO_H_
#define SRC_INCLUDE_APP_GPD_DEMO_H_

/* Starts the GP test device: sends a Commissioning burst immediately, then
 * an On/Off Toggle GP command every 10 seconds via the reusable gpd/
 * library (see src/gpd/gpd.h). */
void app_gpd_demo_init(void);

#endif /* SRC_INCLUDE_APP_GPD_DEMO_H_ */
