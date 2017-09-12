/*
 * Socket.h
 *
 *  Created on: Mar 5, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_SOCKET_H_
#define COMPONENTS_CPP_UTILS_SOCKET_H_

#include <string>
#include <iostream>
#include <streambuf>
#include <cstdio>
#include <cstring>
#include <lwip/inet.h>
#include <lwip/sockets.h>

#undef accept
#undef bind
#undef close
#undef connect
#undef listen
#undef recv
#undef send

/**
 * @brief Encapsulate a socket.
 *
 * Using this class we can connect to a partner TCP server.  Once connected, we can perform
 * send and receive requests to send and receive data.  We should not attempt to send or receive
 * until after a successful connect nor should we send or receive after closing the socket.
 */
class Socket {
public:
	Socket();
	virtual ~Socket();

	Socket accept();
	static std::string addressToString(struct sockaddr* addr);
	void bind(uint16_t port, uint32_t address);
	void close();
	int  connect(struct in_addr address, uint16_t port);
	int  connect(char* address, uint16_t port);
	int  createSocket_cpp(bool isDatagram = false);
	void getBind(struct sockaddr* pAddr);
	int getFD() const;
	bool isValid();
	void listen(uint16_t port, bool isDatagram=false);
	bool operator<(const Socket& other) const;
	std::string readToDelim(std::string delim);
	size_t  receive(uint8_t* data, size_t length, bool exact=false);
	int  receiveFrom_cpp(uint8_t* data, size_t length, struct sockaddr* pAddr);
	int send(std::string value) const;
	int send(const uint8_t* data, size_t length) const;
	int send(uint16_t value);
	int send(uint32_t value);
	void sendTo(const uint8_t* data, size_t length, struct sockaddr* pAddr);
	std::string toString();


private:
	int m_sock;
};

class SocketInputRecordStreambuf : public std::streambuf {
public:
	SocketInputRecordStreambuf(Socket socket, size_t dataLength, size_t bufferSize=512);
	~SocketInputRecordStreambuf();
	int_type underflow();
private:
	char *m_buffer;
	Socket  m_socket;
	size_t  m_dataLength;
	size_t  m_bufferSize;
	size_t  m_sizeRead;
};


#endif /* COMPONENTS_CPP_UTILS_SOCKET_H_ */
