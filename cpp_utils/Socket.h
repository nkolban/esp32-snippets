/*
 * Socket.h
 *
 *  Created on: Mar 5, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_SOCKET_H_
#define COMPONENTS_CPP_UTILS_SOCKET_H_
#include "sdkconfig.h"
#include <mbedtls/platform.h>

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/debug.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/net.h>
#include <mbedtls/ssl.h>

#include <lwip/inet.h>
#include <lwip/sockets.h>

#undef accept
#undef bind
#undef close
#undef connect
#undef listen
#undef read
#undef recv
#undef send
#undef write

#include <string>
#include <iostream>
#include <streambuf>
#include <cstdio>
#include <cstring>
#include <exception>


#if CONFIG_CXX_EXCEPTIONS != 1
#error "C++ exception handling must be enabled within make menuconfig. See Compiler Options > Enable C++ Exceptions."
#endif

class SocketException: public std::exception {
public:
	SocketException(int myErrno);

private:
	int m_errno;

};


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
	int  bind(uint16_t port, uint32_t address);
	int  close();
	int  connect(struct in_addr address, uint16_t port);
	int  connect(char* address, uint16_t port);
	int  createSocket(bool isDatagram = false);
	void setReuseAddress(bool value);
	int  setSocketOption(int option, void* value, size_t len);
	int  setTimeout(uint32_t seconds);
	void getBind(struct sockaddr* pAddr);
	int  getFD() const;
	bool getSSL() const;
	bool isValid();
	int  listen(uint16_t port, bool isDatagram = false, bool reuseAddress = false);
	bool operator<(const Socket& other) const;
	std::string readToDelim(std::string delim);
	size_t  receive(uint8_t* data, size_t length, bool exact = false);
	int  receiveFrom(uint8_t* data, size_t length, struct sockaddr* pAddr);
	int  send(std::string value) const;
	int  send(const uint8_t* data, size_t length) const;
	int  send(uint16_t value);
	int  send(uint32_t value);
	void sendTo(const uint8_t* data, size_t length, struct sockaddr* pAddr);
	void setSSL(bool sslValue = true);
	std::string toString();

private:
	int  m_sock;     // The underlying TCP/IP socket
	bool m_useSSL;   // Should we use SSL
	mbedtls_net_context      m_sslSock;
	mbedtls_entropy_context  m_entropy;
	mbedtls_ctr_drbg_context m_ctr_drbg;
	mbedtls_ssl_context      m_sslContext;
	mbedtls_ssl_config       m_conf;
	mbedtls_x509_crt         m_srvcert;
	mbedtls_pk_context       m_pkey;
	void sslHandshake();

};

class SocketInputRecordStreambuf : public std::streambuf {
public:
	SocketInputRecordStreambuf(Socket socket, size_t dataLength, size_t bufferSize = 512);
	~SocketInputRecordStreambuf();
	int_type underflow();

private:
	char*   m_buffer;
	Socket  m_socket;
	size_t  m_dataLength;
	size_t  m_bufferSize;
	size_t  m_sizeRead;

};


#endif /* COMPONENTS_CPP_UTILS_SOCKET_H_ */
