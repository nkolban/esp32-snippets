# CPP Utils
This directory contains a wealth of C++ classes that have been found useful when working in C++ in conjunction
with the ESP-IDF.  The classes have been documented using `doxygen` so one can run a doxygen processor over them
to create the user guides and programming references.

# Compiling the C++ classes
The C++ classes found here exist as an ESP-IDF component.  To build the classes and then use them in your project perform the following
steps:

1. Create an ESP-IDF project.
2. Create a directory called `components` in the root of your ESP-IDF project.
3. Copy this directory (`cpp_utils`) into your new `components` directory.  The result will be `<project>/components/cpp_utils/<files>`.
4. In your ESP-IDF project build as normal.

The C++ classes will be compiled and available to be used in your own code.


## BLE Functions
The Bluetooth BLE functions are only compiled if Bluetooth is enabled in `make menuconfig`.  This is primarily because
the ESP-IDF build system has chosen to only compile the underlying BLE functions if Bluetooth is enabled.

## Building the Documentation
The code is commented using the Doxygen tags.  As such we can run Doxygen to generate the data.  I use `doxywizard` using
the `Doxyfile` located in this directory.