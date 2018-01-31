# Bootwifi
It is common to want to start an ESP32 and have it connect to a WiFi environment but how does one bootstrap it?  To connect to a WiFi environment, we typically need to know the SSID and password of the network to which we wish to connect.  But without network connection to the ESP32, how do we set it?  This component provides a potential solution.

The module exposes a function called `bootwifi` which, when called, will own the connection of the device to the local WiFi environment.  To do this, it looks in its flash storage to see if it has previously been given an SSID/password pair.  If it has, it attempts to connect to that access point and join the network.

However, let us assume that it has never been given that information.  In this case it will become an access point in its own right.  What this means is that the ESP32 will become a WiFi network to which other WiFi devices can connect.  Now you can connect to the ESP32 using your phone or other WiFi device as it will appear as an access point against which we can connect.  Once connected, we can open a browser to it.  In the browser page, we will be prompted for the SSID and password we wish to subsequently use.  This will be saved and used from then on.  Now the device will connect to that network.

What if we take our ESP32 to a new environment where the previously saved access point is no longer accessible or we simply just fail to connect?  Again, we will fall back into being an access point and the user will be able to supply new information.

What if we want to change the access point to which the ESP32 connects even if that access point has been previously saved and is still connectable?  Simple, the ESP32 can check a GPIO pin at startup and, if that pin is high (default low) then that can be used as a manual indication that we should become an access point without even attempting to connect to the network.

This code is supplied in the form of an ESP-IDF module.  In addition, BootWiFi has a pre-requisite of the `cpp_utils` component also distributed as part of this repoistory.

The logic uses C++ exception handling and hence C++ exception handling must be enabled in `make menuconfig`.

## Example
Below is a sample of a minimal main.cpp.  
* If you run it on an esp32, it should create a new open wifi network with SSID: "ESP32".  
* Connect to that network. 
* Then open a browser and go to http://192.168.4.1.
```C++
#include "BootWiFi.h"
#include "sdkconfig.h"

extern "C" {
	void app_main(void);
}

BootWiFi *boot;

void app_main(void) {
	boot = new BootWiFi();
	boot->boot();
}
```

## GPIO boot override
To enable the ability to specify a GPIO pin to override known station information, compile the code with `-DBOOTWIFI_OVERRIDE_GPIO=<num>` when `<num>` is a GPIO pin number.  If the pin is high at startup, then it will override.  The pin is configured as pull-down low so it need not be artificially held low.  The default is no override pin.

## Future enhancements
There is always room for enhancements:

* Improve the web page shown to the user - Right now it is pretty basic and ideally could be dramatically improved.  Features to be added include
  - listing of available access points for selection
* Integrate SSL security.
* NeoPixel support for visualization of connection status:
  - Green - connected
  - Blue - being an access point
  - Red - Connecting
  - Flashing red - failed
* Ability to specify an IP address for static IP address connection to the access point.
* mDNS support when available.
* Component configuration options including:
  - Network SSID to use when being an access point.
  - Network password to use when being an access point (if any). 
  
## Design and implementation notes
The parameters for Bootwifi are stored in Non-Volatile Storage (NVS).  The name space in NVS
is `bootwifi`.  The keys are:

* `version` - The version of the protocol.
* `connectionInfo` - The details for connection.

The form shown to the end user sends back a response as an HTTP POST to `/ssidSelected`. which contains the following form fields:

* ssid
* password
* ip
* gw
* netmask
 