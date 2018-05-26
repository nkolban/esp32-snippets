# ESP32 FTP Server
The world is awash in excellent FTP servers both as stand-alone applications as well as libraries that can be linked with to build ones own integrated FTP server.  This project is yet another.

The design intent is that it will be available to ESP32 based applications to server files from a Posix file system.

## FTP Protocol
We will be concentrating exclsuively on the FTP Server protocol.  This will allow an ESP32 to serve up files to FTP clients and allow FTP clients to deposit new files.

The FTP Protocol is described in [RFC 959](https://tools.ietf.org/html/rfc959).

Our server engine will implement the Server-PI (Server Protocol Interpreter) and the Server-DTP (Server Data Transfer Process).

## Commands

### PASV
The client has requested that the server enter passive mode.  The response must be:

```
227 Entering Passive Mode (h1,h2,h3,h4,p1,p2).
```

### PORT
Client requests that the server form a data connection to the given host and port.  This is used in FTP *active* mode.
```
PORT {h1,h2,h3,h4,p1,p2}
```
port number is p1*256 + p2

### RETR
This command represents a request *from the client* to retrieve a file held by the server.

### STOR
This command represents a request *from the client* to stor a file on the server.

### SYST
Determine the System type of the FTP server.
```
SYST
```
The recommended response is:
```
215 UNIX Type: L8
```

### TYPE
Define the type of data to be transmitted.  Codes include:

* `A` - ASCII
* `E` - EBCDIC
* `I` - Image (binary)

Example:
```
TYPE I
```

### USER
Supply the user name that the client wishes to present to the FTP server.
```
USER {name}
```

A response of `331` will request a password and should then be followed by the `PASS` command.

## Active vs Passive
The FTP server can communicate data to the client in one of two modes called *active* and *passive*.  In active mode, the FTP client is responsible for setting up a listening socket.  When the FTP client is ready to receive data, it sends a `PORT` command informing the server of the IP address and port against which the FTP server should form the connection.
In passive mode, the FTP client sends a `PASV` command to the FTP server.  The FTP server then responds with a host and port pair which will be listened upon to receive a connection request *from* the client.  Once received, the data can then flow over this connection.

See also:

* [Active FTP vs. Passive FTP, a Definitive Explanation](http://slacksite.com/other/ftp.html)

# References

* [FTP: File Transfer Protocol](https://cr.yp.to/ftp.html)
* [Wikipedia: File Transfer Protocol](https://en.wikipedia.org/wiki/File_Transfer_Protocol)