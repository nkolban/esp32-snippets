#WebSockets
The WebSocket implementation attempts to implement [RFC 6455 - The WebSocket Protocol](https://tools.ietf.org/html/rfc6455).



When an HTTP request arrives, we examine the request to see if it is a request to open a new WebSocket connection.  If it is, we respond with the acknowledgement that we are prepared to be a WebSocket partner.  Once done, the socket that was once used to handle the REST request now becomes the one used to handle bi-directional communications between the ESP32 (original WebSocket server) and the client (browser/Node.js).

Since we don't know when a client may transmit, it could actually transmit at any time.  As such we want to register an asynchronous callback handler that will be invoked when the client sends in some WebSocket data.  To handle this, we then need to `select()` on the WebSocket socket and, when it wakes, invoke the callback handler.  We must **not** issue a new `select()` until the WebSocket message has been consumed.

It is anticipated that a WebSocket message could be large.  The first use case I have come across for using WebSockets is for file transfer and since we have 4MBytes of flash and only 512K of RAM (of which likely only 100K or so may be available) we are likely going to run out of RAM if we wish to write a large file to flash file systems.  What this means is that we likely can't buffer a large WebSocket data message in RAM.

For example, imagine a WebSocket client wants to send a file of 1MByte to the ESP32.  That is more data than we have RAM so we can't hold that message in its entirety in RAM.  What we need to do is "read some" and then "write some" and repeat until all has been consumed.  This sounds like the concept of a stream.  We would have an input stream (data coming into the ESP32) associated with the WebSocket and an output stream (data going out from the ESP32) being written to the file.  We could thus read a small section of the input, write it to the output and continue while we have new data in the input.

This sounds workable ... so let us now think about how we might go about creating an input stream for a WebSocket message.  Each WebSocket message starts with a WebSocket frame which contains, amongst other things, the length of the payload data.  This means that we know up front how much of the remaining data is payload.  This becomes essential as we can't rely on an "end of file" marker in the input stream to indicate the end of the WebSocket payload.  The reason for this is that the WebSocket is a TCP stream that will be used to carry multiple sequential messages.

Let us now invent a new class.  Let us call it a SocketInputRecordStreambuf.
It will have a constructor of the form:

```
SocketInputRecordStreambuf(Socket &socket, size_t dataLength, size_t bufferSize=512)
```

The `socket` is the TCP/IP socket that we are going to read data from.  The `dataLength` is the size of the data we wish to read.  The class will extend `std::streambuf`.  It will internally maintain a data buffer of size `bufferSize`.  Initially, the buffer will be empty.  When a read is performed on the stream, a call to `underflow()` will be made (this is a `std::streambuf` virtual function).

Our rules for this class include:

* We must **not** read more the `dataLength` bytes from the socket.
* We must **indicate** and `EOF` once we have had `dataLength` bytes consumed by a stream reader.
* The class must implement a `discard()` method that will discard further bytes from the socket such that the total number of bytes read from the socket will equal `dataLength`.
* Deleting an instance of the class must invoke `discard()`.

## File transfer
WebSockets make a great file transfer technology.  While this is more an application utilization of the technology than the design of the framework, we'll capture it here.  Let us first set the scene.  We have a client application that wishes to transmit a file.  We will assume that the file is composed of three logical components:

* The file name
* The file length
* The content of the file

It would be wrong to expect the client to send the file as one continuous unit in one WebSocket message.  The reason for this is that the client would have to have loaded the complete file into its memory buffers in order to send it.  As such, we should assume that the client will send the files as one or more "parts" where each part represents a piece of the file.

We thus invent the following protocol:

For the first message we have:

```
+-----------------------+-----------+----+--------------------+-----------+
| transferId (32bit/LE) | file name | \0 | length (32bits/LE) | Data .... |
+-----------------------+-----------+----+--------------------+-----------+
```

For subsequent messages we have:

```
+-----------------------+-----------+
| transferId (32bit/LE) | Data .... |
+-----------------------+-----------+
```
Let us look at these.

* `transferId` - An Id that is randomly generated by the client.  This is used to associate multiple messages for the same file together.
* `file name` - The name of the file that the client wishes to send.  Can include paths.  This will be used to determine where on the file system the file will be written.
* `length` - The size of the file in bytes.  Knowing the size will allow us to know when the whole file has been received.

We will create an encapsulation class called `WebSocketFileTransfer`.