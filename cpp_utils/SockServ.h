

#ifndef MAIN_SOCKSERV_H_
#define MAIN_SOCKSERV_H_
#include <stdint.h>
#include <string>
#include "FreeRTOS.h"


/**
 * @brief Provide a socket listener and the ability to send data to connected partners.
 *
 * We use this class to listen on a given socket and accept connections from partners.
 * When we call one of the sendData() methods, the data passed as parameters is then sent
 * to the connected partners.
 *
 * Here is an example code fragment that uses the class:
 *
 * @code{.cpp}
 * SockServ mySockServer = SockServ(9876);
 * mySockServer.start();
 *
 * // Later ...
 * mySockServer.sendData(data, dataLen);
 * @endcode
 *
 */

class SockServ {
private:
	static void acceptTask(void*);
	uint16_t m_port;
	int      m_sock;
	int      m_clientSock;
	FreeRTOS::Semaphore m_clientSemaphore;
public:
	SockServ(uint16_t port);
	int    connectedCount();
	void   disconnect();
	size_t receiveData(void* pData, size_t maxData);
	void   sendData(uint8_t *data, size_t length);
	void   sendData(std::string str);
	void   start();
	void   stop();
	void   waitForClient();
};

#endif /* MAIN_SOCKSERV_H_ */
