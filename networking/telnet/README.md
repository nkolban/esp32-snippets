#Telnet
This sample illustrates the use of the libtelnet library that can be
read about here:

[https://github.com/seanmiddleditch/libtelnet](https://github.com/seanmiddleditch/libtelnet)

The sample connects to a WiFi access point and then starts to listen on
TCP port 23 (Telnet) for an incoming client request.

##Addition of libtelnet
To install the libtelnet component:

1. Create a sub-directory called `components`.
2. Change into `components`.
3. Clone the libtelnet repository:
```
$ git clone https://github.com/seanmiddleditch/libtelnet.git
```
4. Change into `libtelnet`
5. Create a directory called `include`.
6. Copy all the `*.h` files into `include`:
```
$ cp *.h include/
```
7. Create a `component.mk` file that contains:
```
include $(IDF_PATH)/make/component_common.mk
```