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
	sock = -1;
}

Socket::~Socket() {
	close_cpp(); // When the class instance has ended, delete the socket.
}


void Socket::close_cpp() {
	if (sock != -1) {
		::close(sock);
	}
	sock = -1;
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
	sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int rc = ::connect(sock, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in));
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
 * @brief Receive data from the partner.
 *
 * @param [in] data The buffer into which the received data will be stored.
 * @param [in] length The size of the buffer.
 * @return The length of the data received or -1 on an error.
 */
int Socket::receive_cpp(uint8_t* data, size_t length) {
	int rc = ::recv(sock, data, length, 0);
	if (rc == -1) {
		ESP_LOGE(tag, "receive_cpp: %s", strerror(errno));
	}
	return rc;
} // receive_cpp


/**
 * @brief Send data to the partner.
 *
 * @param [in] data The buffer containing the data to send.
 * @param [in] length The length of data to be sent.
 *
 */
void Socket::send_cpp(const uint8_t* data, size_t length) {
	int rc = ::send(sock, data, length, 0);
	if (rc == -1) {
		ESP_LOGE(tag, "receive_cpp: %s", strerror(errno));
	}
} // send_cpp


/**
 * @brief Send a string to the partner.
 *
 * @param [in] value The string to send to the partner.
 */

void Socket::send_cpp(std::string value) {
	send_cpp((uint8_t *)value.data(), value.size());
} // send_cpp
