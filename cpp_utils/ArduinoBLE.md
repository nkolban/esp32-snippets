# Arduino BLE Support
As part of this Github project, we provide libraries for Bluetooth Low Energy (BLE) for the ESP32 Arduino environment.  Support for this capability is still in the process of being cooked (as of August 2017).  As such you should not build product based on these functions as changes to the API and implementation are extremely likely over the next couple of months.

That said, we now have the ability to produce a driver you can use for testing.  This will give you early access to the function while give those who choose to build and maintain the code early feedback on what works, what doesn't and what can be improved from a usage perspective.

When complete, the BLE support for ESP32 Arduino will be available as a single ZIP file.  The file will be called **ESP32_BLE.zip**.  It is this file that will be able to be imported into the Arduino IDE from the `Sketch -> Include Library -> Add .ZIP library`.  When initial development of the library has been completed, this ZIP will be placed under some form of release control so that an ordinary Arduino IDE user can simply download this as a unit and install.

However, today, we are choosing **not** to distribute the BLE support as a downloadable library.   The reason for this is that we believe that the code base for the BLE support is too fluid and it is being actively worked upon.  Each day there will be changes and that means that for any given instance of the ZIP you get, if it is more than a day old, it is likely out of date.

Instead of giving you a fish (a new ESP32_BLE.zip) we are going to teach you to fish so that you yourself can build the very latest ESP32_BLE.zip yourself.  Hopefully you will find this quite easy to accomplish.

Here is the recipe.

1. Create a new directory called `build`
2. Enter that directory and run `git clone https://github.com/nkolban/esp32-snippets.git`
3. Change into the directory called  `./esp32_snippets/cpp_utils`
4. Run `make -f Makefile.arduino build_ble`
5. Change into the directory called `./Arduino`

And here you will find the `ESP32_BLE.zip` that is build from the latest source.  You can then install this into your Arduino IDE environment are you are ready to go.


## Modifying the Arduino environment for BLE
The Arduino environment supplied for the ESP32 is **not** sufficient for full BLE support as supplied.  A modification must be made.

Here are the instructions

1. Find the directory where your Arduino IDE is installed
2. Navigate relative from there to `<Arduino>/hardware/espressif/esp32/tools/sdk/include/config`
3. Edit the file called `sdkconfig.h`
4. Find the line which reads `#define CONFIG_BTC_TASK_STACK_SIZE 2048` and change to `#define CONFIG_BTC_TASK_STACK_SIZE 8000`

You are now ready to build (re-build) your BLE based applications.

A request has been made to he owners of the Arduino on ESP32 project to see if we can't make this the default or otherwise supply an easier story for modification see [arduino-esp32 issue 567](https://github.com/espressif/arduino-esp32/issues/567).

