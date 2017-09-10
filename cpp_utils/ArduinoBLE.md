# Arduino BLE Support
As part of this Github project, we provide libraries for Bluetooth Low Energy (BLE) for the ESP32 Arduino environment.  Support for this capability is still in the process of being cooked (as of September 2017).  As such you should not build product based on these functions as changes to the API and implementation are extremely likely over the next couple of months.

That said, we now have the ability to produce a driver you can use for testing.  This will give you early access to the function while give those who choose to build and maintain the code early feedback on what works, what doesn't and what can be improved from a usage perspective.

When complete, the BLE support for ESP32 Arduino will be available as a single ZIP file.  The file will be called **ESP32_BLE.zip**.  It is this file that will be able to be imported into the Arduino IDE from the `Sketch -> Include Library -> Add .ZIP library`.  When initial development of the library has been completed, this ZIP will be placed under some form of release control so that an ordinary Arduino IDE user can simply download this as a unit and install.

We provide sample builds of the `ESP32_BLE.zip` file in the `Arduino` folder found relative to this directory.

The build of the Arduino support will be current as of the date of the ZIP file however should you wish to build your own instance of the ZIP from the source, here is the recipe.

1. Create a new directory called `build`
2. Enter that directory and run `git clone https://github.com/nkolban/esp32-snippets.git`
3. Change into the directory called  `./esp32_snippets/cpp_utils`
4. Run `make -f Makefile.arduino build_ble`
5. Change into the directory called `./Arduino`

And here you will find the `ESP32_BLE.zip` that is build from the latest source.  You can then install this into your Arduino IDE environment are you are ready to go.


## Installing a new version
If you have previously installed a version of the Arduino BLE Support and need to install a new one, follow the steps above to build yourself a new instance of the `ESP32_BLE.zip` that is ready for installation.  I recommend removing the old one before installing the new one.  To remove the old one, find the directory where the Arduino IDE stores your libraries (on Linux this is commonly `$HOME/Arduino`).  In there you will find a directory called `libraries` and, if you have previously installed the BLE support, you will find a directory called `ESP32_BLE`.  Remove that directory.

## Switching on debugging
The BLE support contains extensive internal diagnostics which can be switched on by editing the file called `sdkconfig.h` and finding the lines which read:

```
#define CONFIG_LOG_DEFAULT 1
```

Change this to:

```
#define CONFIG_LOG_DEFAULT 5
```

and rebuild/deploy your project.

This file can be found in your Arduino IDE installation directories at:

```
<ArduinoIDE>/hardware/espressif/esp32/tools/sdk/include/config
```