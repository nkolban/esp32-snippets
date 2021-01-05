/*
 * Provisioning.h
 *
 *  Created on: Jan 05, 2021
 *      Author: kchugalinskiy
 */
#ifndef CPP_UTILS_PROVISIONING_H_
#define CPP_UTILS_PROVISIONING_H_

#include "wifi_provisioning/manager.h"

namespace Provisioning {

class WiFi {
public:
	WiFi(wifi_prov_scheme_t scheme, wifi_prov_event_handler_t scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE,
				wifi_prov_event_handler_t app_event_handler = WIFI_PROV_EVENT_HANDLER_NONE);
	~WiFi();

	void Start(const char *pop, const char *service_name, const char *service_key = nullptr, wifi_prov_security_t security = WIFI_PROV_SECURITY_1);
	void Stop();
	void Wait();

private:
	// copy and assignment are discouraged.
	WiFi(const WiFi &) = delete;

	void operator=(const WiFi &) = delete;
};

} // namespace Provisioning

#endif //CPP_UTILS_PROVISIONING_H_
