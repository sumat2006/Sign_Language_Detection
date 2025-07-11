[![Release Version](https://img.shields.io/github/release/h2zero/NimBLE-Arduino.svg?style=plastic)
![Release Date](https://img.shields.io/github/release-date/h2zero/NimBLE-Arduino.svg?style=plastic)](https://github.com/h2zero/NimBLE-Arduino/releases/latest/)
<br/>

# NimBLE-Arduino
A fork of the NimBLE stack refactored for compilation in the Arduino IDE.

> [!IMPORTANT]
> Version 2 is now released!
> Check out the [1.x to 2.x Migration Guide](docs/1.x_to2.x_migration_guide.md) and [Release Notes](https://github.com/h2zero/NimBLE-Arduino/releases/latest)

## Supported MCU's
 - Espressif: ESP32, ESP32C3, ESP32S3, ESP32C6, ESP32H2, ESP32C2, ESP32C5
 - Nordic: nRF51, nRF52 series (**Requires** using [n-able arduino core](https://github.com/h2zero/n-able-Arduino))

**Note for ESP-IDF users: This repo will not compile correctly in ESP-IDF. An ESP-IDF component version of this library can be [found here.](https://github.com/h2zero/esp-nimble-cpp)**

This library **significantly** reduces resource usage and improves performance for ESP32 BLE applications as compared with the bluedroid based library. The goal is to maintain, as much as reasonable, compatibility with the original library but but using the NimBLE stack. In addition, this library will be more actively developed and maintained to provide improved capabilities and stability over the original.
<br/>

For Nordic devices, this library provides access to a completely open source and configurable BLE stack. No softdevice to work around, allowing for full debugging and resource management, continuous updates, with a cross platform API.

# Arduino installation
**Arduino Library manager:** Go to `sketch` -> `Include Library` -> `Manage Libraries`, search for NimBLE and install.

**Alternatively:** Download as .zip and extract to Arduino/libraries folder, or in Arduino IDE from Sketch menu -> Include library -> Add .Zip library.

`#include "NimBLEDevice.h"` at the beginning of your sketch.

# Platformio installation
* Open platformio.ini, a project configuration file located in the root of PlatformIO project.  
* Add the following line to the lib_deps option of [env:] section:
```
h2zero/NimBLE-Arduino@^2.1.0
```
* Build a project, PlatformIO will automatically install dependencies.

# Using
This library is intended to be compatible with the original ESP32 BLE functions and classes with minor changes.

If you have not used the original Bluedroid library please refer to the [New user guide](docs/New_user_guide.md).

If you are familiar with the original library, see: [The migration guide](docs/Migration_guide.md) for details about breaking changes and migration.

If you already use this library and need to migrate your code to version 2.x see the [1.x to 2.x Migration Guide.](docs/1.x_to2.x_migration_guide.md)

[Full API documentation and class list can be found here.](https://h2zero.github.io/NimBLE-Arduino/)

For added performance and optimizations see [Usage tips](docs/Usage_tips.md).

Check the Refactored_original_examples in the examples folder for highlights of the differences with the original library.

More advanced examples highlighting many available features are in examples/ NimBLE_Server, NimBLE_Client.

Beacon examples provided by @beegee-tokyo are in examples/ BLE_Beacon_Scanner, BLE_EddystoneTLM_Beacon.

Change the settings in the `src/nimconfig.h` file to customize NimBLE to your project,
such as increasing max connections, default is 3 for the esp32.
<br/>

# Development Status
This Library is tracking the esp-nimble repo, nimble-1.5.0-idf branch, currently [@e3cbdc0.](https://github.com/espressif/esp-nimble)  
<br/>

# Sponsors
Thank you to all the sponsors who support this project!

<!-- sponsors --><!-- sponsors -->

If you use this library for a commercial product please consider [sponsoring the development](https://github.com/sponsors/h2zero) to ensure the continued updates and maintenance.  
<br/>

# Acknowledgments
* [nkolban](https://github.com/nkolban) and [chegewara](https://github.com/chegewara) for the [original esp32 BLE library](https://github.com/nkolban/esp32-snippets/tree/master/cpp_utils) this project was derived from.
* [beegee-tokyo](https://github.com/beegee-tokyo) for contributing your time to test/debug and contributing the beacon examples.
* [Jeroen88](https://github.com/Jeroen88) for the amazing help debugging and improving the client code.
<br/>
