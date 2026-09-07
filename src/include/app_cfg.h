#pragma once

#define ON  1
#define OFF 0

#ifndef MCU_CORE_8258
#define MCU_CORE_8258 1
#endif

#include "version_cfg.h"

/* A GPD has no application UART, HCI, BDB, or OTA service. */
#define UART_PRINTF_MODE OFF
#define USB_PRINTF_MODE  OFF
#define PM_ENABLE        ON
#define PA_ENABLE        OFF

#define DEBUG_PKT        ON
#define DEBUG_PKT_FILTER OFF
#define DEBUG_CMD        OFF
#define DEBUG_DP         OFF
#define DEBUG_TIME       OFF
#define DEBUG_SAVE       OFF
#define DEBUG_SCHEDULE   OFF
#define DEBUG_PM        OFF
#define DEBUG_REPORTING OFF
#define DEBUG_OTA       OFF
#define DEBUG_STA_STATUS OFF

/* TB03F uses the 512 KiB 8258 flash configuration. */
#define BOARD_8258_03F 11
#define BOARD          BOARD_8258_03F

#if defined(MCU_CORE_8258)
#define CLOCK_SYS_CLOCK_HZ 48000000
#else
#error "This project supports only TLSR8258 TB03F."
#endif

#include "board_tb03f.h"

#define VOLTAGE_DETECT_ENABLE OFF
#define VOLTAGE_DETECT_ADC_PIN GPIO_PC4
#define MODULE_WATCHDOG_ENABLE ON

/* The application sends raw GPDFs and does not expose a GP endpoint. */
#define ZCL_GP_SUPPORT OFF

#include "stack_cfg.h"

typedef enum {
    EV_POLL_ED_DETECT,
    EV_POLL_HCI,
    EV_POLL_IDLE,
    EV_POLL_MAX,
} ev_poll_e;
