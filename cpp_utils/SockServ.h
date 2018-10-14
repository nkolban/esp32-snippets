

#ifndef MAIN_SOCKSERV_H_
#define MAIN_SOCKSERV_H_
#include <stdint.h>
#include <string>
#include <set>
#include "Socket.h"
#include "FreeRTOS.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>


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
	uint16_t            m_port;
	Socket              m_serverSocket;
	FreeRTOS::Semaphore m_clientSemaphore = FreeRTOS::Semaphore("clientSemaphore");
	std::set<Socket>    m_clientSet;
	QueueHandle_t       m_acceptQueue;
	bool                m_useSSL;

public:
	SockServ(uint16_t port);
	SockServ();
	~SockServ();
	int    connectedCount();
	void   disconnect(Socket s);
	bool   getSSL();
	size_t receiveData(Socket s, void* pData, size_t maxData);
	void   sendData(uint8_t* data, size_t length);
	void   sendData(std::string str);
	void   setPort(uint16_t port);
	void   setSSL(bool use = true);
	void   start();
	void   stop();
	Socket waitForData(std::set<Socket>& socketSet);
	Socket waitForNewClient();

};

#endif /* MAIN_SOCKSERV_H_ */
