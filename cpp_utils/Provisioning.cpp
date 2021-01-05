/*
 * Provisioning.cpp
 *
 *  Created on: Jan 05, 2021
 *      Author: kchugalinskiy
 */


#include "Provisioning.h"

namespace Provisioning {

WiFi::WiFi(wifi_prov_scheme_t scheme, wifi_prov_event_handler_t scheme_event_handler, wifi_prov_event_handler_t app_event_handler) {
	wifi_prov_mgr_config_t config {
		.scheme = scheme,
		.scheme_event_handler = scheme_event_handler,
		.app_event_handler = app_event_handler
	};
	ESP_ERROR_CHECK(wifi_prov_mgr_init(config));
}

WiFi::~WiFi() {
	wifi_prov_mgr_deinit();
}

void WiFi::Start(const char *pop, const char *service_name, const char *service_key, wifi_prov_security_t security) {
	ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, pop, service_name, service_key));
}

void WiFi::Stop() {
	wifi_prov_mgr_stop_provisioning();
}

void WiFi::Wait() {
	wifi_prov_mgr_wait();
}

} // namespace Provisioning