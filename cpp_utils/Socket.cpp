/*
 * Socket.cpp
 *
 *  Created on: Mar 5, 2017
 *      Author: kolban
 */

#include <errno.h>
#include <esp_log.h>
#include <lwip/sockets.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sdkconfig.h"
#include "Socket.h"

static char tag[] = "Socket";

Socket::Socket() {
	m_sock = -1;
}

Socket::~Socket() {
	close_cpp(); // When the class instance has ended, delete the socket.
}


/**
 * @brief Convert a socket address to a string representation.
 * @param [in] addr The address to parse.
 * @return A string representation of the address.
 */
std::string Socket::addressToString(struct sockaddr* addr) {
	struct sockaddr_in *pInAddr = (struct sockaddr_in *)addr;
	char temp[30];
	char ip[20];
	inet_ntop(AF_INET, &pInAddr->sin_addr, ip, sizeof(ip));
	sprintf(temp, "%s [%d]", ip, ntohs(pInAddr->sin_port));
	return std::string(temp);
} // addressToString


/**
 * @brief Bind an address/port to a socket.
 * Specify a port of 0 to have a local port allocated.
 * Specify an address of INADDR_ANY to use the local server IP.
 * @param [in] port Port number to bind.
 * @param [in] address Address to bind.
 * @return N/A
 */
void Socket::bind_cpp(uint16_t port, uint32_t address) {
	if (m_sock == -1) {
		ESP_LOGE(tag, "bind_cpp: Socket is not initialized.");
	}
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(address);
	serverAddress.sin_port   = htons(port);
	int rc = ::bind(m_sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (rc == -1) {
		ESP_LOGE(tag, "bind_cpp: bind[socket=%d]: %d: %s", m_sock, errno, strerror(errno));
		return;
	}
} // bind_cpp


/**
 * @brief Close the socket.
 *
 * @return N/A.
 */
void Socket::close_cpp() {
	if (m_sock != -1) {
		::close(m_sock);
	}
	m_sock = -1;
} // close_cpp


/**
 * @brief Connect to a partner.
 *
 * @param [in] address The IP address of the partner.
 * @param [in] port The port number of the partner.
 * @return Success or failure of the connection.
 */
int Socket::connect_cpp(struct in_addr address, uint16_t port) {
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr   = address;
	serverAddress.sin_port   = htons(port);
	char msg[50];
	inet_ntop(AF_INET, &address, msg, sizeof(msg));
	ESP_LOGD(tag, "Connecting to %s:[%d]", msg, port);
	createSocket_cpp();
	int rc = ::connect(m_sock, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in));
	if (rc == -1) {
		ESP_LOGE(tag, "connect_cpp: Error: %s", strerror(errno));
		close_cpp();
		return -1;
	} else {
		ESP_LOGD(tag, "Connected to partner");
		return 0;
	}
} // connect_cpp


/**
 * @brief Connect to a partner.
 *
 * @param [in] strAddress The string representation of the IP address of the partner.
 * @param [in] port The port number of the partner.
 * @return Success or failure of the connection.
 */
int Socket::connect_cpp(char* strAddress, uint16_t port) {
	struct in_addr address;
	inet_pton(AF_INET, (char *)strAddress, &address);
	return connect_cpp(address, port);
}


/**
 * @brief Create the socket.
 * @param [in] isDatagram Set to true to create a datagram socket.  Default is false.
 * @return The socket descriptor.
 */
int Socket::createSocket_cpp(bool isDatagram) {
	if (isDatagram) {
		m_sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	else {
		m_sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	if (m_sock == -1) {
		ESP_LOGE(tag, "createSocket_cpp: socket: %d", errno);
		return m_sock;
	}
	return m_sock;
} // createSocket_cpp


/**
 * @brief Get the bound address.
 * @param [out] pAddr The storage to hold the address.
 * @return N/A.
 */
void Socket::getBind_cpp(struct sockaddr *pAddr) {
	if (m_sock == -1) {
		ESP_LOGE(tag, "getBind_cpp: Socket is not initialized.");
	}
	socklen_t nameLen = sizeof(struct sockaddr);
	::getsockname(m_sock, pAddr, &nameLen);
} // getBind_cpp


/**
 * @brief Create a listening socket.
 * @param [in] port The port number to listen upon.
 * @param [in] isDatagram True if we are listening on a datagram.
 */
void Socket::listen_cpp(uint16_t port, bool isDatagram) {
	createSocket_cpp(isDatagram);
	bind_cpp(port, INADDR_ANY);
} // listen_cpp


/**
 * @brief Receive data from the partner.
 *
 * @param [in] data The buffer into which the received data will be stored.
 * @param [in] length The size of the buffer.
 * @return The length of the data received or -1 on an error.
 */
int Socket::receive_cpp(uint8_t* data, size_t length) {
	int rc = ::recv(m_sock, data, length, 0);
	if (rc == -1) {
		ESP_LOGE(tag, "receive_cpp: %s", strerror(errno));
	}
	return rc;
} // receive_cpp


/**
 * @brief Receive data with the address.
 * @param [in] data The location where to store the data.
 * @param [in] length The size of the data buffer into which we can receive data.
 * @param [in] pAddr An area into which we can store the address of the partner.
 * @return The length of the data received.
 */
int Socket::receiveFrom_cpp(uint8_t* data, size_t length,	struct sockaddr *pAddr) {
	socklen_t addrLen = sizeof(struct sockaddr);
	int rc = ::recvfrom(m_sock, data, length, 0, pAddr, &addrLen);
	return rc;
} // receiveFrom_cpp


/**
 * @brief Send data to the partner.
 *
 * @param [in] data The buffer containing the data to send.
 * @param [in] length The length of data to be sent.
 * @return N/A.
 *
 */
void Socket::send_cpp(const uint8_t* data, size_t length) {
	int rc = ::send(m_sock, data, length, 0);
	if (rc == -1) {
		ESP_LOGE(tag, "send: socket=%d, %s", m_sock, strerror(errno));
	}
} // send_cpp


/**
 * @brief Send a string to the partner.
 *
 * @param [in] value The string to send to the partner.
 * @return N/A.
 */
void Socket::send_cpp(std::string value) {
	send_cpp((uint8_t *)value.data(), value.size());
} // send_cpp


/**
 * @brief Send data to a specific address.
 * @param [in] data The data to send.
 * @param [in] length The length of the data to send/
 * @param [in] pAddr The address to send the data.
 */
void Socket::sendTo_cpp(const uint8_t* data, size_t length, struct sockaddr* pAddr) {
	int rc = ::sendto(m_sock, data, length, 0, pAddr, sizeof(struct sockaddr));
	if (rc == -1) {
		ESP_LOGE(tag, "sendto_cpp: socket=%d %s", m_sock, strerror(errno));
	}
} // sendTo_cpp
