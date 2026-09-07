cmake_minimum_required(VERSION 3.15)

set(SDK_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/tl_zigbee_sdk" CACHE PATH "Path to the Telink Zigbee SDK root")
set(TOOLCHAIN_PREFIX "/Users/ivan.belokobylskiy/projects/telink/tc32" CACHE PATH "Path to the tc32 toolchain root")

# Must run before project() so CMAKE_SYSTEM_NAME/CMAKE_C_COMPILER take effect
# for compiler detection.
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/TelinkSDK.cmake)

if(NOT CMAKE_ASM_COMPILER)
    set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
endif()

project(gp_demo C ASM)

add_subdirectory(src)
