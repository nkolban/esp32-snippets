GPS Demo
====================

ESP32 read GPS Signal through UART (Serial) from GPS Module (tested on a ``ublox LEA-6S-0-001``)

Setup
-------
1. Download Project.
2. Go to subfolder ``components``
3. Get gps parser library *minmea*. run command ``git clone https://github.com/cloudyourcar/minmea``
4. From step 3 you get a new folder here - minmea. Now go into this folder and add a new file: ``component.mk``
5. Open ``component.mk``, add ``COMPONENT_ADD_INCLUDEDIRS=.``
6. **Extra**: Now you are good to go. But when you do compiling get ``error: implicit declaration of function `timegm```, then in file ``minmea.c`` find where is the ``timegm``, relplace it with ``timegm``

Photo
-------
Link: https://www.flickr.com/photos/gfast2/34401947423/in/dateposted/
