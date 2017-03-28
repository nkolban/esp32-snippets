#CPP Utils
This directory contains a wealth of C++ classes that have been found useful when working in C++ in conjunction
with the ESP-IDF.  The classes have been documented using `doxygen` so one can run a doxygen processor over them
to create the user guides and programming references.

##BLE Functions
The Bluetooth BLE functions are only compiled if Bluetooth is enabled in `make menuconfig`.  This is primarily because
the ESP-IDF build system has chosen to only compile the underlying BLE functions if Bluetooth is enabled.