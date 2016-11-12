void telnet_esp32_listenForClients(void (*callbackParam)(uint8_t *buffer, size_t size));
void telnet_esp32_sendData(uint8_t *buffer, size_t size);
int telnet_esp32_vprintf(const char *fmt, va_list va);
