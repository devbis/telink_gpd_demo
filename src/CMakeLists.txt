SET(TARGET gp_test_device)

ADD_DEFINITIONS(
    # END_DEVICE keeps normal sleep/retention behavior. CGP transport is
    # implemented locally in src/gpd/gpd_mac.c.
    -DEND_DEVICE=1
    -DMCU_CORE_${TELINK_PLATFORM}=1
    -DMCU_STARTUP_${TELINK_PLATFORM}=1
)

SET(ZIGBEE_SRC
    ${SDK_PREFIX}/platform/boot/link_cfg.S
    ${SDK_PREFIX}/platform/boot/${TELINK_PLATFORM}/cstartup_${TELINK_PLATFORM}.S
    ${SDK_PREFIX}/platform/services/b85m/irq_handler.c
    ${SDK_PREFIX}/platform/tc32/div_mod.S
    ${SDK_PREFIX}/platform/chip_${TELINK_PLATFORM}/flash.c
    ${SDK_PREFIX}/platform/chip_${TELINK_PLATFORM}/adc.c
    ${SDK_PREFIX}/platform/chip_${TELINK_PLATFORM}/flash/flash_common.c
    ${SDK_PREFIX}/platform/chip_${TELINK_PLATFORM}/flash/flash_mid011460c8.c
    ${SDK_PREFIX}/platform/chip_${TELINK_PLATFORM}/flash/flash_mid1060c8.c
    ${SDK_PREFIX}/platform/chip_${TELINK_PLATFORM}/flash/flash_mid13325e.c
    ${SDK_PREFIX}/platform/chip_${TELINK_PLATFORM}/flash/flash_mid134051.c
    ${SDK_PREFIX}/platform/chip_${TELINK_PLATFORM}/flash/flash_mid136085.c
    ${SDK_PREFIX}/platform/chip_${TELINK_PLATFORM}/flash/flash_mid1360c8.c
    ${SDK_PREFIX}/platform/chip_${TELINK_PLATFORM}/flash/flash_mid1360eb.c
    ${SDK_PREFIX}/platform/chip_${TELINK_PLATFORM}/flash/flash_mid14325e.c
    ${SDK_PREFIX}/platform/chip_${TELINK_PLATFORM}/flash/flash_mid1460c8.c

    ${SDK_PREFIX}/zigbee/aps/aps_group.c
    ${SDK_PREFIX}/zigbee/mac/mac_phy.c
    ${SDK_PREFIX}/zigbee/mac/mac_pib.c
    ${SDK_PREFIX}/zigbee/zdo/zdp.c
    ${SDK_PREFIX}/zigbee/common/zb_config.c
    ${SDK_PREFIX}/zigbee/af/zb_af.c
    ${SDK_PREFIX}/zigbee/ss/ss_nv.c

    ${SDK_PREFIX}/proj/common/list.c
    ${SDK_PREFIX}/proj/common/mempool.c
    ${SDK_PREFIX}/proj/common/string.c
    ${SDK_PREFIX}/proj/common/tlPrintf.c
    ${SDK_PREFIX}/proj/common/utility.c
    ${SDK_PREFIX}/proj/drivers/drv_adc.c
    ${SDK_PREFIX}/proj/drivers/drv_flash.c
    ${SDK_PREFIX}/proj/drivers/drv_calibration.c
    ${SDK_PREFIX}/proj/drivers/drv_gpio.c
    ${SDK_PREFIX}/proj/drivers/drv_hw.c
    ${SDK_PREFIX}/proj/drivers/drv_i2c.c
    ${SDK_PREFIX}/proj/drivers/drv_keyboard.c
    ${SDK_PREFIX}/proj/drivers/drv_nv.c
    ${SDK_PREFIX}/proj/drivers/drv_pm.c
    ${SDK_PREFIX}/proj/drivers/drv_putchar.c
    ${SDK_PREFIX}/proj/drivers/drv_pwm.c
    ${SDK_PREFIX}/proj/drivers/drv_security.c
    ${SDK_PREFIX}/proj/drivers/drv_spi.c
    ${SDK_PREFIX}/proj/drivers/drv_uart.c
    ${SDK_PREFIX}/proj/drivers/drv_timer.c
    ${SDK_PREFIX}/proj/os/ev.c
    ${SDK_PREFIX}/proj/os/ev_buffer.c
    ${SDK_PREFIX}/proj/os/ev_poll.c
    ${SDK_PREFIX}/proj/os/ev_queue.c
    ${SDK_PREFIX}/proj/os/ev_rtc.c
    ${SDK_PREFIX}/proj/os/ev_timer.c
)

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/src/include/boards
    ${SDK_PREFIX}/zigbee/af
    ${SDK_PREFIX}/zigbee/bdb/includes
    ${SDK_PREFIX}/zigbee/mac/includes
    ${SDK_PREFIX}/zigbee/ota
    ${SDK_PREFIX}/zigbee/zcl
    ${SDK_PREFIX}/zigbee/gp
    ${SDK_PREFIX}/zigbee/common/includes
    ${SDK_PREFIX}/zigbee/zbapi
    ${SDK_PREFIX}/proj
    ${SDK_PREFIX}/proj/common
    ${SDK_PREFIX}/platform
    ${SDK_PREFIX}/platform/chip_${TELINK_PLATFORM}
)

LINK_DIRECTORIES(
    ${SDK_PREFIX}/zigbee/lib/tc32
    ${SDK_PREFIX}/platform/lib
    ${SDK_PREFIX}/platform/tc32
)

SET(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -T${SDK_PREFIX}/platform/boot/${TELINK_PLATFORM}/boot_${TELINK_PLATFORM}.link"
)

file(GLOB SOURCES1 *.c *.cpp *.h *.S common/*.c common/*.h gpd/*.c gpd/*.h)
SET(SOURCES ${SOURCES1} ${ZIGBEE_SRC})

ADD_EXECUTABLE(${TARGET} ${SOURCES})
TARGET_LINK_LIBRARIES(${TARGET}
    -Wl,--start-group
    zb_ed
    drivers_${TELINK_PLATFORM}
    libsoft-fp.a
    -Wl,--end-group
)

ADD_BIN_TARGET(${TARGET} ${PROJECT_SOURCE_DIR}/tools)
