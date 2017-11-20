# mongoose/webserver
This snippet provides a simple Webserver using the Mongoose library on the ESP32.
It provides two URL endpoints:

* `/time` - Returns a JSON encoded time since startup.
* `/test1.html` - Returns a simple HTML web page.

The web page has been hard-coded into the application.  We used the `xxd` tool on Linux
which takes as input a binary file (eg. an HTML source file) and generates a header file
that exposes the data.  For example:

```
$ xxd -i test1.html test1_html.h
```