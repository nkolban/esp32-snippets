#include "lwip/socket.h"

static void dumpSockaddr(struct sockaddr_in address) {
	ESP_LOGD(tag, "Address details:");
	char *family = "Unknown";
	unsigned short port;
	char ipString[20];

	if (address.sin_family == AF_INET) {
		family = "AF_INET";
	} else if (address.sin_family == AF_INET6) {
		family = "AF_INET6";
	}
	port = ntohs(address.sin_port);
  inet_ntop(address.sin_family, &address.sin_addr, ipString, sizeof(ipString)) ;
	ESP_LOGD(tag, "Address details: family=%s, ip=%s, port=%d", family, ipString, port);
}
