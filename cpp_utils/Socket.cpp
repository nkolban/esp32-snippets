/*
 * Socket.cpp
 *
 *  Created on: Mar 5, 2017
 *      Author: kolban
 */
#include <iostream>
#include <streambuf>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <string>

#include <sstream>

#include <errno.h>
#include <esp_log.h>
#include <lwip/sockets.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include "GeneralUtils.h"
#include "SSLUtils.h"
#include "sdkconfig.h"
#include "Socket.h"

static const char* LOG_TAG = "Socket";

#undef bind

static void my_debug(
   void* ctx,
   int level,
   const char* file,
   int line,
   const char* str) {

   ((void) level);
   ((void) ctx);
   printf("%s:%04d: %s", file, line, str);
}

Socket::Socket() {
	m_sock   = -1;
	m_useSSL = false;
}

Socket::~Socket() {
	//close_cpp(); // When the class instance has ended, delete the socket.
}


/**
 * @brief Accept a new socket.
 */
Socket Socket::accept() {
	struct sockaddr addr;
	getBind(&addr);
	ESP_LOGD(LOG_TAG, ">> accept: Accepting on %s; sockFd: %d, using SSL: %d", addressToString(&addr).c_str(), m_sock, getSSL());
	struct sockaddr_in client_addr;
	socklen_t sin_size;
	int clientSockFD = ::lwip_accept_r(m_sock,  (struct sockaddr*) &client_addr, &sin_size);
	//printf("------> new connection client %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	if (clientSockFD == -1) {
		SocketException se(errno);
		ESP_LOGE(LOG_TAG, "accept(): %s, m_sock=%d", strerror(errno), m_sock);
		throw se;
	}

	ESP_LOGD(LOG_TAG, " - accept: Received new client!: sockFd: %d", clientSockFD);
	Socket newSocket;
	newSocket.m_sock = clientSockFD;
	if (getSSL()) {
		newSocket.setSSL(true);
		newSocket.m_sslSock.fd = clientSockFD;
		newSocket.sslHandshake();
		ESP_LOGD(LOG_TAG, "DEBUG DEBUG ");
		uint8_t x;   // What is going on here???
		newSocket.receive(&x, 1, 0); // FIX FIX FIX
	}
	ESP_LOGD(LOG_TAG, "<< accept: sockFd: %d", clientSockFD);
	return newSocket;
} // accept


/**
 * @brief Convert a socket address to a string representation.
 * @param [in] addr The address to parse.
 * @return A string representation of the address.
 */
std::string Socket::addressToString(struct sockaddr* addr) {
	struct sockaddr_in* pInAddr = (struct sockaddr_in*) addr;
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
 * @return Returns 0 on success.
 */
int Socket::bind(uint16_t port, uint32_t address) {
	ESP_LOGD(LOG_TAG, ">> bind: port=%d, address=0x%x", port, address);

	if (m_sock == -1) {
		ESP_LOGE(LOG_TAG, "bind: Socket is not initialized.");
	}
	struct sockaddr_in serverAddress;
	serverAddress.sin_family      = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(address);
	serverAddress.sin_port        = htons(port);
	int rc = ::lwip_bind_r(m_sock, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
	if (rc != 0) {
		ESP_LOGE(LOG_TAG, "<< bind: bind[socket=%d]: %d: %s", m_sock, errno, strerror(errno));
		return rc;
	}
	ESP_LOGD(LOG_TAG, "<< bind");
	return rc;
} // bind


/**
 * @brief Close the socket.
 *
 * @return Returns 0 on success.
 */
int Socket::close() {
	ESP_LOGD(LOG_TAG, "close: m_sock=%d, ssl: %d", m_sock, getSSL());
	int rc;
	if (getSSL()) {
		rc = mbedtls_ssl_close_notify(&m_sslContext);
		if (rc < 0) {
			ESP_LOGD(LOG_TAG, "mbedtls_ssl_close_notify: %d", rc);
		}
	}
	rc = 0;
	if (m_sock != -1) {
		ESP_LOGD(LOG_TAG, "Calling lwip_close on %d", m_sock);
		rc = ::lwip_close_r(m_sock);
		if (rc != 0) {
			ESP_LOGE(LOG_TAG, "Error with lwip_close: %d", rc);
		}
	}
	m_sock = -1;
	return rc;
} // close


/**
 * @brief Connect to a partner.
 *
 * @param [in] address The IP address of the partner.
 * @param [in] port The port number of the partner.
 * @return Success or failure of the connection.
 */
int Socket::connect(struct in_addr address, uint16_t port) {
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr   = address;
	serverAddress.sin_port   = htons(port);
	char msg[50];
	inet_ntop(AF_INET, &address, msg, sizeof(msg));
	ESP_LOGD(LOG_TAG, "Connecting to %s:[%d]", msg, port);
	createSocket();
	int rc = ::lwip_connect_r(m_sock, (struct sockaddr*) &serverAddress, sizeof(struct sockaddr_in));
	if (rc == -1) {
		ESP_LOGE(LOG_TAG, "connect_cpp: Error: %s", strerror(errno));
		close();
		return -1;
	} else {
		ESP_LOGD(LOG_TAG, "Connected to partner");
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
int Socket::connect(char* strAddress, uint16_t port) {
	struct in_addr address;
	inet_pton(AF_INET, strAddress, &address);
	return connect(address, port);
}


/**
 * @brief Create the socket.
 * @param [in] isDatagram Set to true to create a datagram socket.  Default is false.
 * @return The socket descriptor.
 */
int Socket::createSocket(bool isDatagram) {
	ESP_LOGD(LOG_TAG, ">> createSocket: isDatagram: %d", isDatagram);
	if (isDatagram) {
		m_sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	} else {
		m_sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	if (m_sock == -1) {
		ESP_LOGE(LOG_TAG, "<< createSocket: socket: %d", errno);
		return m_sock;
	}
	ESP_LOGD(LOG_TAG, "<< createSocket: sockFd: %d", m_sock);
	return m_sock;
} // createSocket


/**
 * @brief Get the bound address.
 * @param [out] pAddr The storage to hold the address.
 * @return N/A.
 */
void Socket::getBind(struct sockaddr* pAddr) {
	if (m_sock == -1) {
		ESP_LOGE(LOG_TAG, "getBind: Socket is not initialized.");
	}
	socklen_t nameLen = sizeof(struct sockaddr);
	int rc = ::getsockname(m_sock, pAddr, &nameLen);
	if (rc != 0) {
		ESP_LOGE(LOG_TAG, "Error with getsockname in getBind: %s", strerror(errno));
	}
} // getBind


/**
 * @brief Get the underlying socket file descriptor.
 * @return The underlying socket file descriptor.
 */
int Socket::getFD() const {
	return m_sock;
} // getFD

bool Socket::getSSL() const {
	return m_useSSL;
}

bool Socket::isValid() {
	return m_sock != -1;
} // isValid

/**
 * @brief Create a listening socket.
 * @param [in] port The port number to listen upon.
 * @param [in] isDatagram True if we are listening on a datagram.  The default is false.
 * @return Returns 0 on success.
 */
int Socket::listen(uint16_t port, bool isDatagram, bool reuseAddress) {
	ESP_LOGD(LOG_TAG, ">> listen: port: %d, isDatagram: %d", port, isDatagram);
	createSocket(isDatagram);
	setReuseAddress(reuseAddress);
	int rc = bind(port, 0);
	if (rc != 0) {
		ESP_LOGE(LOG_TAG, "<< listen: Error in bind: %s", strerror(errno));
		return rc;
	}
	// For a datagram socket, we don't execute a listen call.  That is is only for connection oriented
	// sockets.
	if (!isDatagram) {
		rc = ::lwip_listen_r(m_sock, 5);
		if (rc == -1) {
			ESP_LOGE(LOG_TAG, "<< listen: %s", strerror(errno));
			return rc;
		}
	}
	ESP_LOGD(LOG_TAG, "<< listen");
	return 0;
} // listen


bool Socket::operator <(const Socket& other) const {
	return m_sock < other.m_sock;
}


/**
 * @brief Set the socket option.
 */
int Socket::setSocketOption(int option, void* value, size_t len) {
	int res = ::setsockopt(m_sock, SOL_SOCKET, option, value, len);
	if (res < 0) {
		ESP_LOGE(LOG_TAG, "%X : %d", option, errno);
	}
	return res;
} // setSocketOption


/**
 * @brief Socket timeout.
 * @param [in] seconds to wait.
 */
int Socket::setTimeout(uint32_t seconds) {
	struct timeval tv;
	tv.tv_sec = seconds;
	tv.tv_usec = 0;
	if (setSocketOption(SO_RCVTIMEO, (char*) &tv, sizeof(struct timeval)) < 0) {
		return -1;
	}
	return setSocketOption(SO_SNDTIMEO, (char*) &tv, sizeof(struct timeval));
}


std::string Socket::readToDelim(std::string delim) {
	std::string ret;
	std::string part;
	auto it = delim.begin();
	while (true) {
		uint8_t val;
		int rc = receive(&val, 1);
		if (rc == -1) return "";
		if (rc == 0) return ret + part;
		if (*it == val) {
			part+= val;
			++it;
			if (it == delim.end()) return ret;
		} else {
			if (part.empty()) {
				ret += part;
				part.clear();
				it = delim.begin();
			}
			ret += val;
		}
	} // While
} // readToDelim


/**
 * @brief Receive data from the partner.
 * Receive data from the socket partner.  If exact = false, we read as much data as
 * is available without blocking up to length.  If exact = true, we will block until
 * we have received exactly length bytes or there are no more bytes to read.
 * @param [in] data The buffer into which the received data will be stored.
 * @param [in] length The size of the buffer.
 * @param [in] exact Read exactly this amount.
 * @return The length of the data received or -1 on an error.
 */
size_t Socket::receive(uint8_t* data, size_t length, bool exact) {
	//ESP_LOGD(LOG_TAG, ">> receive: sockFd: %d, length: %d, exact: %d", m_sock, length, exact);
	if (!exact) {
		int rc;
		if (getSSL()) {
			do {
				rc = mbedtls_ssl_read(&m_sslContext, data, length);
				ESP_LOGD(LOG_TAG, "rc=%d, MBEDTLS_ERR_SSL_WANT_READ=%d", rc, MBEDTLS_ERR_SSL_WANT_READ);
			} while (rc == MBEDTLS_ERR_SSL_WANT_WRITE || rc == MBEDTLS_ERR_SSL_WANT_READ);
		} else {
			rc = ::lwip_recv_r(m_sock, data, length, 0);
			if (rc == -1) {
				ESP_LOGE(LOG_TAG, "receive: %s", strerror(errno));
			}
		}
		//GeneralUtils::hexDump(data, rc);
		//ESP_LOGD(LOG_TAG, "<< receive: rc: %d", rc);
		return (size_t) rc;
	} // Read what we can, doesn't need to be an exact amount.

	size_t amountToRead = length;
	int rc;
	while (amountToRead > 0) {
		if (getSSL()) {
			do {
				rc = mbedtls_ssl_read(&m_sslContext, data, amountToRead);
			} while (rc == MBEDTLS_ERR_SSL_WANT_WRITE || rc == MBEDTLS_ERR_SSL_WANT_READ);
		} else {
			rc = ::lwip_recv_r(m_sock, data, amountToRead, 0);
		}
		if (rc == -1) {
			ESP_LOGE(LOG_TAG, "receive: %s", strerror(errno));
			return 0;
		}
		if (rc == 0) break;
		amountToRead -= rc;
		data += rc;
	}
	//GeneralUtils::hexDump(data, length);
	//ESP_LOGD(LOG_TAG, "<< receive: %d", length);
	return length;
} // receive_cpp


/**
 * @brief Receive data with the address.
 * @param [in] data The location where to store the data.
 * @param [in] length The size of the data buffer into which we can receive data.
 * @param [in] pAddr An area into which we can store the address of the partner.
 * @return The length of the data received.
 */
int Socket::receiveFrom(uint8_t* data, size_t length, struct sockaddr *pAddr) {
	socklen_t addrLen = sizeof(struct sockaddr);
	int rc = ::recvfrom(m_sock, data, length, 0, pAddr, &addrLen);
	return rc;
} // receiveFrom


/**
 * @brief Send data to the partner.
 *
 * @param [in] data The buffer containing the data to send.
 * @param [in] length The length of data to be sent.
 * @return N/A.
 *
 */
int Socket::send(const uint8_t* data, size_t length) const {
	ESP_LOGD(LOG_TAG, "send: Raw binary of length: %d", length);
	//GeneralUtils::hexDump(data, length);
	int rc = ERR_OK;
	while (length > 0) {
		if (getSSL()) {
			rc = mbedtls_ssl_write((mbedtls_ssl_context*)&m_sslContext, data, length);
			// retry with same parameters if MBEDTLS_ERR_SSL_WANT_WRITE or MBEDTLS_ERR_SSL_WANT_READ
			if ((rc != MBEDTLS_ERR_SSL_WANT_WRITE) && (rc != MBEDTLS_ERR_SSL_WANT_READ)) {
				if (rc < 0) {
					// no cure for other errors - log and exit
					ESP_LOGE(LOG_TAG, "send: SSL write error %d", rc);
					return rc;
				} else {
					// not all data was written, try again for the remainder
					length -= rc;
					data += rc;
				}
			}
		} else {
			rc = ::lwip_send_r(m_sock, data, length, 0);
			if ((rc < 0) && (errno != EAGAIN)) {
				// no cure for errors other than EAGAIN - log and exit
				ESP_LOGE(LOG_TAG, "send: socket=%d, %s", m_sock, strerror(errno));
				return rc;
			} else if (rc > 0) {
				// not all data was written, try again for the remainder
				length -= rc;
				data += rc;
			}
		}
	}
	return rc;
} // send


/**
 * @brief Send a string to the partner.
 *
 * @param [in] value The string to send to the partner.
 * @return N/A.
 */
int Socket::send(std::string value) const {
	ESP_LOGD(LOG_TAG, "send: Binary of length: %d", value.length());
	return send((uint8_t*) value.data(), value.size());
} // send


int Socket::send(uint16_t value) {
	ESP_LOGD(LOG_TAG, "send: 16bit value: %.2x", value);
	return send((uint8_t*) &value, sizeof(value));
} // send_cpp


int  Socket::send(uint32_t value) {
	ESP_LOGD(LOG_TAG, "send: 32bit value: %.2x", value);
	return send((uint8_t*) &value, sizeof(value));
} // send


/**
 * @brief Send data to a specific address.
 * @param [in] data The data to send.
 * @param [in] length The length of the data to send/
 * @param [in] pAddr The address to send the data.
 */
void Socket::sendTo(const uint8_t* data, size_t length, struct sockaddr* pAddr) {
	int rc;
	if (getSSL()) {
		rc = mbedtls_ssl_write(&m_sslContext, data, length);
	} else {
		rc = ::sendto(m_sock, data, length, 0, pAddr, sizeof(struct sockaddr));
	}
	if (rc < 0) {
		ESP_LOGE(LOG_TAG, "sendto: socket=%d %s", m_sock, strerror(errno));
	}
} // sendTo


/**
 * @brief Flag the socket address as re-usable.
 * @param [in] value True to mark the address as re-usable, false otherwise.
 */
void Socket::setReuseAddress(bool value) {
	ESP_LOGD(LOG_TAG, ">> setReuseAddress: %d", value);
	int val = value ? 1 : 0;
	setSocketOption(SO_REUSEADDR, &val, sizeof(val));
	ESP_LOGD(LOG_TAG, "<< setReuseAddress");
} // setReuseAddress


/**
 * @brief Flag the socket as using SSL
 * @param [in] sslValue True if we wish to use SSL.
 */
void Socket::setSSL(bool sslValue) {
  const char* pers = "ssl_server";
	ESP_LOGD(LOG_TAG, ">> setSSL: %s", sslValue?"Yes":"No");
	m_useSSL = sslValue;

	if (sslValue) {
		char *pvtKey = SSLUtils::getKey();
		char *certificate = SSLUtils::getCertificate();
		if (pvtKey == nullptr) {
			ESP_LOGE(LOG_TAG, "No private key file");
			return;
		}
		if (certificate == nullptr) {
			ESP_LOGE(LOG_TAG, "No certificate file");
			return;
		}

		mbedtls_net_init(&m_sslSock);
		mbedtls_ssl_init(&m_sslContext);
		mbedtls_ssl_config_init(&m_conf);
		mbedtls_x509_crt_init(&m_srvcert);
		mbedtls_pk_init(&m_pkey);
		mbedtls_entropy_init(&m_entropy);
		mbedtls_ctr_drbg_init(&m_ctr_drbg);

		int ret = mbedtls_x509_crt_parse(&m_srvcert, (unsigned char *) certificate, strlen(certificate) + 1);
		if (ret != 0) {
			ESP_LOGD(LOG_TAG, "mbedtls_x509_crt_parse returned 0x%x", -ret);
			goto exit;
		}

		ret = mbedtls_pk_parse_key(&m_pkey, (unsigned char *) pvtKey, strlen(pvtKey) + 1, NULL, 0);
		if (ret != 0) {
			ESP_LOGD(LOG_TAG, "mbedtls_pk_parse_key returned 0x%x", -ret);
			goto exit;
		}

		ret = mbedtls_ctr_drbg_seed(&m_ctr_drbg, mbedtls_entropy_func, &m_entropy, (const unsigned char*) pers, strlen(pers));
		if (ret != 0) {
			ESP_LOGD(LOG_TAG, "! mbedtls_ctr_drbg_seed returned %d\n", ret);
			goto exit;
		}

		ret = mbedtls_ssl_config_defaults(&m_conf,
				MBEDTLS_SSL_IS_SERVER,
				MBEDTLS_SSL_TRANSPORT_STREAM,
				MBEDTLS_SSL_PRESET_DEFAULT);
		if (ret != 0) {
			ESP_LOGD(LOG_TAG, "mbedtls_ssl_config_defaults returned %d\n\n", ret);
			goto exit;
		}

		mbedtls_ssl_conf_authmode(&m_conf, MBEDTLS_SSL_VERIFY_NONE);
		mbedtls_ssl_conf_rng(&m_conf, mbedtls_ctr_drbg_random, &m_ctr_drbg);

//	mbedtls_ssl_conf_ca_chain( &m_conf, m_srvcert.next, NULL);
		ret = mbedtls_ssl_conf_own_cert(&m_conf, &m_srvcert, &m_pkey);
		if (ret != 0) {
			ESP_LOGD(LOG_TAG, "mbedtls_ssl_conf_own_cert returned %d\n\n", ret);
			goto exit;
		}

		mbedtls_ssl_conf_dbg(&m_conf, my_debug, nullptr);
#ifdef CONFIG_MBEDTLS_DEBUG
		mbedtls_debug_set_threshold(4);
#endif

		ret = mbedtls_ssl_setup(&m_sslContext, &m_conf);
		if (ret != 0) {
			ESP_LOGD(LOG_TAG, "mbedtls_ssl_setup returned %d\n\n", ret);
			goto exit;
		}
	}
exit:
	return;
} // setSSL


/**
 * @brief perform the SSL handshake
 */
void Socket::sslHandshake() {
	ESP_LOGD(LOG_TAG, ">> sslHandshake: sock: %d", m_sslSock.fd);
	mbedtls_ssl_session_reset(&m_sslContext);
	ESP_LOGD(LOG_TAG, " - Reset complete");
	mbedtls_ssl_set_bio(&m_sslContext, &m_sslSock, mbedtls_net_send, mbedtls_net_recv, NULL);

	while (true) {
		int ret = mbedtls_ssl_handshake(&m_sslContext);
		if (ret == 0) break;

		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
			ESP_LOGD(LOG_TAG, "mbedtls_ssl_handshake returned %d\n\n", ret);
			return;
		}
	} // End while
	ESP_LOGD(LOG_TAG, "<< sslHandshake");
} // sslHandshake


/**
 * @brief Get the string representation of this socket
 * @return the string representation of the socket.
 */
std::string Socket::toString() {
	std::ostringstream oss;
	oss << "fd: " << m_sock;
	return oss.str();
} // toString


/**
 * @brief Create a socket input record streambuf
 * @param [in] socket The socket we will be reading from.
 * @param [in] dataLength The size of a record.
 * @param [in] bufferSize The size of the buffer we wish to allocate to hold data.
 */
SocketInputRecordStreambuf::SocketInputRecordStreambuf(
	Socket  socket,
	size_t  dataLength,
	size_t  bufferSize) {
	m_socket     = socket;    // The socket we will be reading from
	m_dataLength = dataLength; // The size of the record we wish to read.
	m_bufferSize = bufferSize; // The size of the buffer used to hold data
	m_sizeRead   = 0;          // The size of data read from the socket
	m_buffer = new char[bufferSize]; // Create the buffer used to hold the data read from the socket.

	setg(m_buffer, m_buffer, m_buffer); // Set the initial get buffer pointers to no data.
} // SocketInputRecordStreambuf


SocketInputRecordStreambuf::~SocketInputRecordStreambuf() {
	delete[] m_buffer;
} // ~SocketInputRecordStreambuf


/**
 * @brief Handle the request to read data from the stream but we need more data from the source.
 *
 */
SocketInputRecordStreambuf::int_type SocketInputRecordStreambuf::underflow() {
	if (m_sizeRead >= m_dataLength) return EOF;
	int bytesRead = m_socket.receive((uint8_t*) m_buffer, m_bufferSize, true);
	if (bytesRead == 0) return EOF;

	m_sizeRead += bytesRead;
	setg(m_buffer, m_buffer, m_buffer + bytesRead);
	return traits_type::to_int_type(*gptr());
} // underflow

SocketException::SocketException(int myErrno) {
	m_errno = myErrno;
}
