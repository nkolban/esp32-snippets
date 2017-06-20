# Building curl

Create folder called `components` to host curl as a component.  In the `components` folder run:

```
$ git clone https://github.com/curl/curl.git
```

This results in a download/clone of the `curl` library.  A new folder called `curl` will be found.  Go into that folder.

We now need to copy some ESP32 specific configuration files.  These are:

* `lib/curl_config.h`
* `component.mk`

Finally, we need to make a small edit.  Find the file called:

`curl/include/curl/system.h`

and open it in your favorite editor.  Find the section that reads:

```
#elif defined(__GNUC__)
#  if !defined(__LP64__) && (defined(__ILP32__) || \
      defined(__i386__) || defined(__ppc__) || defined(__arm__) || \
      defined(__sparc__) || defined(__mips__) || defined(__sh__))
```

and change to:

```
#elif defined(__GNUC__)
#  if !defined(__LP64__) && (defined(__ILP32__) || \
      defined(__i386__) || defined(__ppc__) || defined(__arm__) || \
      defined(__sparc__) || defined(__mips__) || defined(__sh__) || defined(__XTENSA__))
```

A request has been made to the owners of libcurl to make this change in their own source.  See [issue 1598](https://github.com/curl/curl/issues/1598).

Once the above steps have been completed, we should be able to build our ESP-IDF project as normal and that will include the construction
of the curl library ready for use.  Now you can code directly to the curl APIs or you can code to the 
C++ REST classes that leverage curl.