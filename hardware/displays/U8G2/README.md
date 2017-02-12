#ESP32 U8G2 library support
There is an excellent open source library called `u8g2` that can be found on Github here:

[https://github.com/olikraus/u8g2](https://github.com/olikraus/u8g2)

The purpose of the library is to provide a display independent driver layer for monochrome displays including LCD and OLED.
The library "knows" how to driver the underlying displays as well as providing drawing primitives including text, fonts, lines and
other geometrical shapes.

A version of the library is available in cleanly compiling C and compiles without incident on the ESP32-IDF framework.

However, since the library is agnostic of MCU environments and will work on a variety of boards, there has to be a mapping from
the functions expected by the library and the underlying MCU board hardware.  This includes driving GPIOs, I2C, SPI and more.

The code in this folder provides a mapping from U8g2 to the ESP32 ESP-IDF.  This should be included in your build of U8g2 applications.

To use with the ESP32, we must invoke the `u8g2_esp32_hal_init()` function before invoking any of the normal U8g2 functons.  What
this call does is tell the ESP32 what pins we wish to map.  Here is an example of use:

```
u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
u8g2_esp32_hal.clk   = PIN_CLK;
u8g2_esp32_hal.mosi  = PIN_MOSI;
u8g2_esp32_hal.cs    = PIN_CS;
u8g2_esp32_hal.dc    = PIN_DC;
u8g2_esp32_hal.reset = PIN_RESET;
u8g2_esp32_hal_init(u8g2_esp32_hal);
```

The function takes as input a `u8g2_esp32_hal` structure instance which has the logical pins that U8g2 needs mapped to the
physical pin on the ESP32 that we wish to use.  If we don't use a specific pin for our specific display, set the value to
be `U8G2_ESP32_HAL_UNDEFINED` which is the default initialization value.

Remember, ESP32 pins are not hard-coded to functions and as such, all the GPIO pins on the ESP32 are open for use.  Following
this initialization, we can use U8g2 as normal and described in the U8g2 documentation.

##Compiling U8G2
To use the actual U8g2 library in your ESP32 project, perform the following steps:

1. Create a directory called `components` in your main project directory.
2. Change into the `components` directory.
3. Run `git clone https://github.com/olikraus/u8g2.git` to bring in a the latest copy of u8g2 library.
4. Change into the `u8g2` directory.
5. Create a file called `component.mk`
6. Enter the following in the `component.mk` file:
```
COMPONENT_SRCDIRS:=csrc
COMPONENT_ADD_INCLUDEDIRS:=csrc
```


##Development
While in principal, there should be nothing specific needed beyond this addition to make U8g2 work on the ESP32, only a small
number of boards have been tested.  In addition, currently only SPI displays have been tested.  This will be remidied over time.