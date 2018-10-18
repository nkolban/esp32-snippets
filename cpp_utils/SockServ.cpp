/*
 * SockServ.cpp
 *
 *  Created on: Feb 24, 2017
 *      Author: kolban
 */

#include <errno.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <stdint.h>

#include <string.h>
#include <string>

#include "sdkconfig.h"
#include "FreeRTOS.h"
#include "SockServ.h"
#include "Socket.h"

static const char* LOG_TAG = "SockServ";


/**
 * @brief Create an instance of the class.
 *
 * We won't actually start listening for clients until after the start() method has been called.
 * @param [in] port The TCP/IP port number on which we will listen for incoming connection requests.
 */
SockServ::SockServ(uint16_t port) : SockServ() {
	this->m_port = port;
} // SockServ


/**
 * Constructor
 */
SockServ::SockServ() {
	m_port        = 0;  // Unknown port.
	m_acceptQueue = xQueueCreate(1, sizeof(Socket));
	m_useSSL      = false;
	m_clientSemaphore.take("SockServ");   // Create the queue; deleted in the destructor.
} // SockServ


/**
 * Destructor
 */
SockServ::~SockServ() {
	vQueueDelete(m_acceptQueue);   // Delete the queue created in the constructor.
} // ~SockServ


/**
 * @brief Accept an incoming connection.
 * @private
 *
 * Block waiting for an incoming connection and accept it when it arrives.  The new
 * socket is placed on a queue and a semaphore signaled that a new client is available.
 */
/* static */ void SockServ::acceptTask(void* data) {
	SockServ* pSockServ = (SockServ*) data;
	while (true) {
		try {
			ESP_LOGD(LOG_TAG, "Waiting on accept");
			Socket tempSock = pSockServ->m_serverSocket.accept();
			if (!tempSock.isValid()) continue;

			pSockServ->m_clientSet.insert(tempSock);
			xQueueSendToBack(pSockServ->m_acceptQueue, &tempSock, portMAX_DELAY);
			pSockServ->m_clientSemaphore.give();
		} catch (std::exception e) {
			ESP_LOGD(LOG_TAG, "acceptTask ending");
			pSockServ->m_clientSemaphore.give();   // Wake up any waiting clients.
			FreeRTOS::deleteTask();
			break;
		}
	}
} // acceptTask


/**
 * @brief Determine the number of connected partners.
 *
 * @return The number of connected partners.
 */
int SockServ::connectedCount() {
	return m_clientSet.size();
} // connectedCount


/**
 * @brief Disconnect any connected partners.
 */
void SockServ::disconnect(Socket s) {
	auto search = m_clientSet.find(s);
	m_clientSet.erase(search);
} // disconnect


/**
 * Get the SSL status.
 */
bool SockServ::getSSL() {
	return m_useSSL;
} // getSSL


/**
 * @brief Wait for data
 * @param [in] pData Pointer to buffer to hold the data.
 * @param [in] maxData Maximum size of the data to receive.
 * @return The amount of data returned or 0 if there was an error.
 */
size_t SockServ::receiveData(Socket s, void* pData, size_t maxData) {
	size_t rc = s.receive((uint8_t*) pData, maxData);
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
	sendData((uint8_t*) str.data(), str.size());
} // sendData


/**
 * @brief Send data to any connected partners.
 *
 * @param[in] data A sequence of bytes to send to the partner.
 * @param[in] length The length of the sequence of bytes to send to the partner.
 */
void SockServ::sendData(uint8_t* data, size_t length) {
  for (auto it = m_clientSet.begin(); it != m_clientSet.end(); ++it) {
	  (*it).send(data, length);
  }
} // sendData


/**
 * @brief Set the port number to use.
 * @param port The port number to use.
 */
void SockServ::setPort(uint16_t port) {
	m_port = port;
} // setPort


void SockServ::setSSL(bool use) {
	m_useSSL = use;
} // setSSL


/**
 * @brief Start listening for new partner connections.
 *
 * The port number on which we will listen is the one defined when the class was created.
 */
void SockServ::start() {
	assert(m_port != 0);
	//m_serverSocket.setSSL(m_useSSL);
	m_serverSocket.listen(m_port);   // Create a socket and start listening on it.
	ESP_LOGD(LOG_TAG, "Now listening on port %d", m_port);
	FreeRTOS::startTask(acceptTask, "acceptTask", this, 8 * 1024);
} // start


/**
 * @brief Stop listening for new partner connections.
 */
void SockServ::stop() {
	ESP_LOGD(LOG_TAG, ">> stop");
	// By closing the server socket, the task watching on accept() on that socket
	// will throw an exception which will propagate a clean ending.
	m_serverSocket.close();   // Close the server socket.
	ESP_LOGD(LOG_TAG, "<< stop");
} // stop


Socket SockServ::waitForData(std::set<Socket>& socketSet) {
	fd_set readSet;
	int maxFd = -1;

	for (auto it = socketSet.begin(); it != socketSet.end(); ++it) {
		FD_SET(it->getFD(), &readSet);
		if (it->getFD() > maxFd) {
			maxFd = it->getFD();
		}
	} // End for

	int rc = ::select(
			maxFd+1,  // Number of sockets to scan
			&readSet, // Set of read sockets
			nullptr,  // Set of write sockets
			nullptr,  // Set of exception sockets
			nullptr   // Timeout
	);
	if (rc == -1) {
		ESP_LOGE(LOG_TAG, "Error with select");
		Socket s;
		return s;
	}

	for (auto it = socketSet.begin(); it != socketSet.end(); ++it) {
		if (FD_ISSET(it->getFD(), &readSet)) {
			return *it;
		}
	} // End for
	Socket s;
	return s;
}


/**
 * @brief Wait for a client connection to be present.
 * Returns when a client connection is present.  This can block until a client connects
 * or can return immediately is there is already a client connection in existence.
 */
Socket SockServ::waitForNewClient() {
	ESP_LOGD(LOG_TAG, ">> waitForNewClient");
	m_clientSemaphore.wait("waitForNewClient");                 // Unlocked in acceptTask.
	m_clientSemaphore.take("waitForNewClient");
	Socket tempSocket;
	BaseType_t rc = xQueueReceive(m_acceptQueue, &tempSocket, 0);   // Read the socket from the queue.
	if (rc != pdPASS) {
		ESP_LOGE(LOG_TAG, "No new client from SockServ!");
		throw new SocketException(0);
	}
	ESP_LOGD(LOG_TAG, "<< waitForNewClient");
	return tempSocket;
} // waitForNewClient
