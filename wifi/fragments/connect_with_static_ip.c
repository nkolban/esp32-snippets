/**
 * A common task is to connect to an access point and hence be a
 * WiFi station.  By default, when we connect to the access point we
 * then perform DHCP client protocols to obtain an IP address.  This
 * takes a few seconds (in my experience).  If we are in the middle of
 * edit, compile, run, test - repeat cycles ... every few seconds of
 * "pause" can add up to our workflow.  This fragment illustrates the use
 * of a static IP address.  We continue to connect to the access point but
 * now instead of using DHCP to be allocated an address, we tell the
 * access point the address we want to use.  Obviously it can't already
 * be in use and it has to be in the range of valid addresses.  If these
 * are satisfied, we shave off a few seconds of pause time and work as we
 * immediately have our IP address.
 */
#include <lwip/sockets.h>

// The IP address that we want our device to have.
#define DEVICE_IP          "192.168.5.2"

// The Gateway address where we wish to send packets.
// This will commonly be our access point.
#define DEVICE_GW          "192.168.5.1"

// The netmask specification.
#define DEVICE_NETMASK     "255.255.255.0"

// The identity of the access point to which we wish to connect.
#define AP_TARGET_SSID     "RASPI3"

// The password we need to supply to the access point for authorization.
#define AP_TARGET_PASSWORD "password"

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}


// Code fragment here ...
  nvs_flash_init();
  tcpip_adapter_init();

  tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA); // Don't run a DHCP client
  tcpip_adapter_ip_info_t ipInfo;

  inet_pton(AF_INET, DEVICE_IP, &ipInfo.ip);
  inet_pton(AF_INET, DEVICE_GW, &ipInfo.gw);
  inet_pton(AF_INET, DEVICE_NETMASK, &ipInfo.netmask);
  tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);

  ESP_ERROR_CHECK(esp_event_loop_init(wifiEventHandler, NULL));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  wifi_config_t sta_config = {
    .sta = {
      .ssid      = AP_TARGET_SSID,
      .password  = AP_TARGET_PASSWORD,
      .bssid_set = 0
    }
  };
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_connect());
