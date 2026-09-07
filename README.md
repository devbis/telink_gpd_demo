# TLSR8258 Green Power Device demo

This project is a small transmit-only Green Power Device (GPD) firmware for
the Telink TLSR8258 TB03F module.

The firmware sends a Green Power commissioning frame once after boot and an
On/Off Toggle command every 10 seconds. Frames are built by the local GPD
implementation and submitted directly to the 802.15.4 MAC layer.

This is not a router and it does not join a Zigbee network. The build uses the
Telink end-device stack because its power-management path provides the desired
deep-sleep and SRAM-retention behavior. `END_DEVICE` is therefore a firmware
power-management choice here, not a request to expose a Zigbee end device.

## Hardware and dependencies

- Telink TLSR8258 TB03F module with 512 KiB flash.
- CMake 3.15 or newer.
- Telink TC32 toolchain.
- Telink Zigbee SDK, supplied separately and available as `tl_zigbee_sdk`.
- Python 3 for the firmware post-processing helper.

The SDK and toolchain are intentionally not part of this repository. The SDK
has its own license and must be used according to its distribution terms.

## Build

Configure the project with paths to the local SDK and TC32 toolchain:

```sh
cmake -S . -B build \
  -DSDK_PREFIX="$PWD/tl_zigbee_sdk" \
  -DTOOLCHAIN_PREFIX=/path/to/tc32
cmake --build build --target gp_test_device.bin
```

The resulting files are:

```text
build/src/gp_test_device       # TC32 ELF image
build/src/gp_test_device.bin   # flashable binary image
```

## GPD configuration

The demo currently uses:

- IEEE 802.15.4 channel 11;
- SrcID `0x87654321`;
- On/Off Switch device ID;
- the standard test key `ZigBeeAlliance09`;
- Green Power security level with a frame counter and CCM* MIC.

To make the frames visible in Zigbee2MQTT, the coordinator must operate on
the same channel and have Green Power support enabled. The GPD itself is not
paired, joined, or represented by a normal Zigbee endpoint.

## Power management

Power management is enabled with `PM_ENABLE`. When the next timer is pending,
the TLSR8258 enters deep sleep with SRAM retention. On wake-up, the firmware
keeps the application and event state and only reconfigures the radio PHY.

## License

Unless stated otherwise, the project code is licensed under the Apache License
2.0. See [LICENSE](LICENSE). Third-party SDK files remain subject to their own
license terms.
