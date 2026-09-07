/*
 * version_cfg.h - image identity used by the TLSR8258 boot layout.
 */
#pragma once

#include "../common/comm_cfg.h"

#define CHIP_TYPE       TLSR_8258_512K
#define APP_RELEASE     0x10
#define APP_BUILD       0x01
#define STACK_RELEASE   0x30
#define STACK_BUILD     0x01
#define HW_VERSION      0x01

/* These fields are consumed by the image header, not by OTA code. */
#define IMAGE_TYPE_APP  (0x07 | (IMAGE_TYPE_BOOT_FLAG << 7))
#define IMAGE_TYPE      ((CHIP_TYPE << 8) | IMAGE_TYPE_APP)
#define FILE_VERSION    ((APP_RELEASE << 24) | \
                         (APP_BUILD << 16) | \
                         (STACK_RELEASE << 8) | STACK_BUILD)

#define MANUFACTURER_CODE_TELINK 0xdb15
#define IS_BOOT_LOADER_IMAGE     0
#define RESV_FOR_APP_RAM_CODE_SIZE 0
#define IMAGE_OFFSET             APP_IMAGE_ADDR
