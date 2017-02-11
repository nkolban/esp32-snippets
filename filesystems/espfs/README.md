#The ESP FS
A fantastic project by Mr SpriteTM can be found here:

[https://github.com/Spritetm/libesphttpd/tree/master/espfs](https://github.com/Spritetm/libesphttpd/tree/master/espfs)

This provides an ESP File System that is read only and stored on flash.

Here, you will find a copy of those core files that have been massaged to utilize ESP32 technologies.
Primarily, we use flash memory mapping to access the data as opposed to individual flash reads.  In addition,
and the primary intent, a new method was added with the signature:

```
int espFsAccess(EspFsFile *fh, void **buf, size_t *len)
```

This function returns a pointer to the whole content of the file which is stored in buf.  The
length of the file is stored in len and also returned from the function as a whole.
The data is accessed directly from flash without any RAM copies.
In addition, the function called:

```
EspFsInitResult espFsInit(void *flashAddress, size_t size)
```

was augmented to include the size of the flash storage to map.

For full details and background, see the following thread on the ESP32 forum:

[http://esp32.com/viewtopic.php?f=13&t=698](http://esp32.com/viewtopic.php?f=13&t=698)