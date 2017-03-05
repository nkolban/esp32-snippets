

#ifndef MAIN_SOCKSERV_H_
#define MAIN_SOCKSERV_H_
#include <stdint.h>
#include <string>

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
	uint16_t port;
	int sock;
	int clientSock;
	static void acceptTask(void *data);
public:
	SockServ(uint16_t port);
	int connectedCount();
	void disconnect();
	void sendData(uint8_t *data, size_t length);
	void sendData(std::string str);
	void start();
	void stop();
};

#endif /* MAIN_SOCKSERV_H_ */
