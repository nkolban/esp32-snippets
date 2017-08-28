# Building curl

Create folder called `components` to host curl as a component.  In the `components` folder run:

```
$ git clone https://github.com/curl/curl.git
```

This results in a download/clone of the `curl` library.  A new folder called `curl` will be found.  Go into that folder.

We now need to copy some ESP32 specific configuration files.  These are:

* `lib/curl_config.h`
* `component.mk`



Once the above steps have been completed, we should be able to build our ESP-IDF project as normal and that will include the construction of the curl library ready for use.  Now you can code directly to the curl APIs or you can code to the  C++ REST classes that leverage curl.