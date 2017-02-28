/**
 * Provide a socket listener and the ability to send data to connected partners.
 *
 * We use this class to listen on a given socket and accept connections from partners.
 * When we call one of the sendData() methods, the data passed as parameters is then sent
 * to the connected partners.
 *
 */

#ifndef MAIN_SOCKSERV_H_
#define MAIN_SOCKSERV_H_
#include <stdint.h>
#include <string>

class SockServ {
private:
	uint16_t port;
	int sock;
	int clientSock;
	static void acceptTask(void *data);
public:
	SockServ(uint16_t port);
	void start();
	void stop();
	void sendData(uint8_t *data, size_t length);
	void sendData(std::string str);
};

#endif /* MAIN_SOCKSERV_H_ */
