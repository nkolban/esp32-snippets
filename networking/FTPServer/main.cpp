#include "FTPServer.h"
#include <stdio.h>


int main(int argc, char* argv[]) {
	printf("FTPServer starting!\n");
	FTPServer *ftpServer = new FTPServer();
	ftpServer->setCallbacks(new FTPFileCallbacks());
	//ftpServer->setCredentials("user", "pass");
	ftpServer->setPort(9876);
	ftpServer->start();
}
