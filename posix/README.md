# POSIX
Posix is the specification for Unix like functions.  The ESP-IDF provides many implementations for Posix functions but some are omitted and thus should not be used in ESP32 based applications.  However there are times when we received 3rd party code that uses a Posix function that we don't have in our environment.  Our choices then become:

* Contact the 3rd party provider and ask them to alter their code to remove it, replace it or make it optional.
* Contact Espressif and ask them to add support for the missing Posix function.
* Provide a shim that satisfies the Posix function using the capabailities that are available to us.

In the source file called `posix_shims.c` we provide some of those shims.