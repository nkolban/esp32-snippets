/*
 * SockServ.cpp
 *
 *  Created on: Feb 24, 2017
 *      Author: kolban
 */

#include <errno.h>
#include <esp_log.h>
#include <FreeRTOS.h>
#include <lwip/sockets.h>

#include <stdint.h>
#include <string.h>
#include <string>

#include "sdkconfig.h"
#include "SockServ.h"

static const char* LOG_TAG = "SockServ";


/**
 * @brief Create an instance of the class.
 *
 * We won't actually start listening for clients until after the start() method has been called.
 * @param [in] port The TCP/IP port number on which we will listen for incoming connection requests.
 */
SockServ::SockServ(uint16_t port) {
	this->m_port = port;
	m_clientSock = -1;
	m_sock       = -1;
	m_clientSemaphore.take("SockServ");
} // SockServ


/**
 * @brief Accept an incoming connection.
 *
 * Block waiting for an incoming connection and accept it when it arrives.
 */
void SockServ::acceptTask(void *data) {

	SockServ* pSockServ = (SockServ*)data;
	struct sockaddr_in clientAddress;

	while(1) {
		socklen_t clientAddressLength = sizeof(clientAddress);
		int tempSock = ::accept(pSockServ->m_sock, (struct sockaddr *)&clientAddress, &clientAddressLength);
		if (tempSock == -1) {
			ESP_LOGE(LOG_TAG, "close(): %s", strerror(errno));
		}
		ESP_LOGD(LOG_TAG, "accept() - New socket");
		if (pSockServ->m_clientSock != -1) {
			int rc = ::close(pSockServ->m_clientSock);
			if (rc == -1) {
				ESP_LOGE(LOG_TAG, "close(): %s", strerror(errno));
			}
		}
		pSockServ->m_clientSock = tempSock;
		pSockServ->m_clientSemaphore.give();
	}
} // acceptTask


/**
 * @brief Determine the number of connected partners.
 *
 * @return The number of connected partners.
 */
int SockServ::connectedCount() {
	if (m_clientSock == -1) {
		return 0;
	}
	return 1;
} // connectedCount


/**
 * @brief Disconnect any connected partners.
 */
void SockServ::disconnect() {
	if (m_clientSock != -1) {
		int rc = ::close(m_clientSock);
		if (rc == -1) {
			ESP_LOGE(LOG_TAG, "close(): %s", strerror(errno));
		}
		m_clientSock = -1;
		m_clientSemaphore.take("disconnect");
	}
} // disconnect


/**
 * @brief Wait for data
 * @param [in] pData Pointer to buffer to hold the data.
 * @param [in] maxData Maximum size of the data to receive.
 * @return The amount of data returned or 0 if there was an error.
 */
size_t SockServ::receiveData(void* pData, size_t maxData) {
	if (m_clientSock == -1) {
		return 0;
	}
	int rc = ::recv(m_clientSock, pData, maxData, 0);
	if (rc == -1) {
		ESP_LOGE(LOG_TAG, "recv(): %s", strerror(errno));
		return 0;
	}
	return rc;
} // receiveData


/**
 * @brief Send data from a string to any connected partners.
 *
 * @param[in] str A string from which sequence of bytes will be used to send to the partner.
 */
void SockServ::sendData(std::string str) {
	sendData((uint8_t *)str.data(), str.size());
} // sendData


/**
 * @brief Send data to any connected partners.
 *
 * @param[in] data A sequence of bytes to send to the partner.
 * @param[in] length The length of the sequence of bytes to send to the partner.
 */
void SockServ::sendData(uint8_t* data, size_t length) {
	if (connectedCount() == 0) {
		return;
	}
	int rc = ::send(m_clientSock, data, length, 0);
	if (rc == -1) {
		ESP_LOGE(LOG_TAG, "send(): %s", strerror(errno));
	}
} // sendData


/**
 * @brief Start listening for new partner connections.
 *
 * The port number on which we will listen is the one defined when the class was created.
 */
void SockServ::start() {
	m_sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sock == -1) {
		ESP_LOGE(LOG_TAG, "socket(): %s", strerror(errno));
	}
	struct sockaddr_in serverAddress;
	serverAddress.sin_family      = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port        = htons(m_port);
	int rc = ::bind(m_sock, (const struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (rc == -1) {
		ESP_LOGE(LOG_TAG, "bind(): %s", strerror(errno));
	}
	rc = ::listen(m_sock, 5);
	if (rc == -1) {
		ESP_LOGE(LOG_TAG, "listen(): %s", strerror(errno));
	}
	ESP_LOGD(LOG_TAG, "Now listening on port %d", m_port);
	FreeRTOS::startTask(acceptTask, "acceptTask", this);
} // start


/**
 * @brief Stop listening for new partner connections.
 */
void SockServ::stop() {
	int rc = ::close(m_sock);
	if (rc == -1) {
		ESP_LOGE(LOG_TAG, "close(): %s", strerror(errno));
	}
} // stop


/**
 * @brief Wait for a client connection to be present.
 * Returns when a client connection is present.  This can block until a client connects
 * or can return immediately is there is already a client connection in existence.
 */
void SockServ::waitForClient() {
	m_clientSemaphore.wait("waitForClient");
} // waitForClient
