# Eclipse Paho MQTT C/C++ client for Embedded platforms

This repository contains the source code for the [Eclipse Paho](http://eclipse.org/paho) MQTT C/C++ client library for Embedded platorms.

It is dual licensed under the EPL and EDL (see about.html and notice.html for more details).  You can choose which of these licenses you want to use the code under.  The EDL allows you to embed the code into your application, and distribute your application in binary or source form without contributing any of your code, or any changes you make back to Paho.  See the EDL for the exact conditions.

The MQTTPacket directory contains the lowest level C library with the smallest requirements.  This supplies simple serialization
and deserialization routines.  It is mainly up to you to write and read to and from the network.

The MQTTClient directory contains the next level C++ library.  This still avoids most networking code so that you can plugin the
network of your choice.

## Build requirements / compilation

There are helper scripts (build...) in various directories.  The client library is a set of building blocks which you pick and choose from, so that the smallest MQTT application can be built.

## Usage and API

See the samples directory for examples of intended use.


## Runtime tracing

As yet, there is no tracing.  For the smallest client, should we have tracing?


## Reporting bugs

This project uses GitHub Issues here: [github.com/eclipse/paho.mqtt.embedded-c/issues](https://github.com/eclipse/paho.mqtt.embedded-c/issues) to track ongoing development and issues.

## More information

Discussion of the Paho clients takes place on the [Eclipse paho-dev mailing list](https://dev.eclipse.org/mailman/listinfo/paho-dev).

General questions about the MQTT protocol are discussed in the [MQTT Google Group](https://groups.google.com/forum/?hl=en-US&fromgroups#!forum/mqtt).

There is much more information available via the [MQTT community site](http://mqtt.org).

# ESP32 Specific
The compilation requires that we enable mbedTLS debugging features.  Run "make menuconfig" and visit `Component Config -> mbedTLS` and check the `Enable mbedTLS debugging` section.  Save and rebuild the environment.
