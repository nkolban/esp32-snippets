/*
 * Socket.h
 *
 *  Created on: Mar 5, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_SOCKET_H_
#define COMPONENTS_CPP_UTILS_SOCKET_H_
#include <lwip/inet.h>
#include <lwip/sockets.h>
#include <string>

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

	void send_cpp(std::string value);
	static std::string addressToString(struct sockaddr *addr);
	void bind_cpp(uint16_t port, uint32_t address);
	void close_cpp();
	int connect_cpp(struct in_addr address, uint16_t port);
	int connect_cpp(char *address, uint16_t port);
	int createSocket_cpp(bool isDatagram = false);
	void getBind_cpp(struct sockaddr *pAddr);
	void listen_cpp(uint16_t port, bool isDatagram);
	int receive_cpp(uint8_t *data, size_t length);
	int receiveFrom_cpp(uint8_t *data, size_t length, struct sockaddr *pAddr);
	void send_cpp(const uint8_t *data, size_t length);
	void sendTo_cpp(const uint8_t *data, size_t length, struct sockaddr *pAddr);
private:
	int m_sock;
};

#endif /* COMPONENTS_CPP_UTILS_SOCKET_H_ */
