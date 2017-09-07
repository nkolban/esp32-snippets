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

	Socket accept_cpp();
	static std::string addressToString(struct sockaddr* addr);
	void bind_cpp(uint16_t port, uint32_t address);
	void close_cpp();
	int  connect_cpp(struct in_addr address, uint16_t port);
	int  connect_cpp(char* address, uint16_t port);
	int  createSocket_cpp(bool isDatagram = false);
	void getBind_cpp(struct sockaddr* pAddr);
	int getFD() const;
	bool isValid();
	void listen_cpp(uint16_t port, bool isDatagram=false);
	bool operator<(const Socket& other) const;
	std::string readToDelim(std::string delim);
	int  receive_cpp(uint8_t* data, size_t length, bool exact=false);
	int  receiveFrom_cpp(uint8_t* data, size_t length, struct sockaddr* pAddr);
	int send_cpp(std::string value) const;
	int send_cpp(const uint8_t* data, size_t length) const;
	int send_cpp(uint16_t value);
	int send_cpp(uint32_t value);
	void sendTo_cpp(const uint8_t* data, size_t length, struct sockaddr* pAddr);
	std::string toString();


private:
	int m_sock;
};

class SocketInputRecordStream : public std::streambuf {
public:
	SocketInputRecordStream(Socket* socket, size_t dataLength, size_t bufferSize=512);
	~SocketInputRecordStream();
	int_type underflow();
private:
	char *m_buffer;
	Socket* m_pSocket;
	size_t  m_dataLength;
	size_t  m_bufferSize;
	size_t  m_sizeRead;
};

#endif /* COMPONENTS_CPP_UTILS_SOCKET_H_ */
