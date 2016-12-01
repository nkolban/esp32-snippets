char *authModeToString(wifi_auth_mode_t mode) {
	switch(mode) {
	case WIFI_AUTH_OPEN:
		return "open";
	case WIFI_AUTH_WEP:
		return "wep";
	case WIFI_AUTH_WPA_PSK:
		return "wpa";
	case WIFI_AUTH_WPA2_PSK:
		return "wpa2";
	case WIFI_AUTH_WPA_WPA2_PSK:
		return "wpa_wpa2";
	default:
		return "unknown";
	}
}
