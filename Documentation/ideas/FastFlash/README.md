# FastFlash
As we build ESP32 applications, we typically perform a compile, flash, test loop cycle.  We compile an app, we flash the ESP32 with that app and then we test whether it works.  We perform these actions over and over again.  When we look at the time taken in each step, we see there is compile time on our PC and then the time taken to flash the PC.  This story talks about the time taken to flash the PC.

The ESP32 is typically configured to flash at 115200 kbps.  This is 115200 bits per second.  If we think that a typical ESP32 application is 800KBytes then this requires a transmission of:

800000 * 9 = 7.2 million bits = 62.5 seconds

we can increase our baud rate up to 921600 = 7.8 seconds