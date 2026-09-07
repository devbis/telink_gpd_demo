#pragma once

/* Standalone application image, no bootloader. */
#define BOOT_LOADER_MODE       0
#define BOOT_LOADER_IMAGE_ADDR 0x0
#define APP_IMAGE_ADDR         0x0
#define IMAGE_TYPE_BOOT_FLAG   0

#define TLSR_8258_512K         0x02
#define TLSR_8258_1M           0x03

#define IMAGE_TYPE_BOOTLOADER  0xff
