# Displays
There are a large number of displays that are available on the market for hobbyists to use
in our projects.  Most of these displays can be connected via some combinations of data buses
including I2C and SPI.  The data sheets for those displays describes the signals they expect
in order to show output.  Unfortunately, there appears to be as many data descriptions as there
are displays so no "one-size-fits-all".  We could devote many, many hours to reading and
experimenting with the data sheets for a device in order to get it running ... or ... we can
say "thank you" to the likes of good folks such as "Adafruit" who have done the work of writing
drivers and making them available in source form through Github.  Many of these drivers were
written for the Arduino class of devices using C and C++.  However, for us in ESP32 land, we
have a number of excellent choices.  Firstly, there is a port of the whole Arduino framework
to the ESP32 so we can use an ESP32 in exactly the same manner as would use an Arduino including
the ability to run sketches that include Arduino driven displays.

An alternative is to take the Arduino drivers and "port" them to leverage the ESP32 native
environment called "ESP-IDF".  Fortunately, this is an extremely easy task as there is almost
a one-to-one mapping between functions expected by an Arduino driver and capabilities offered 
by the ESP-IDF.

In this directory area of this Github project, we will find some simple ports of some
projects that were written for the Arduino.  Amongst these we will find:

* [Adafruit_SSD1305-Library](https://github.com/adafruit/Adafruit_SSD1305_Library)
* [Adafruit_SSD1306-Library](https://github.com/adafruit/Adafruit_SSD1306)
* [Adafruit-PCD8544-Nokia-5110-LCD-Library](https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library)
* [Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library)

In the `tests` folder we will also find some test applications. 