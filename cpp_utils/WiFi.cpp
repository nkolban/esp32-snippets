/*
 * WiFi.cpp
 *
 *  Created on: Feb 25, 2017
 *      Author: kolban
 */

//#define _GLIBCXX_USE_C99
#include <string>
#include <sstream>
#include <iomanip>
#include "sdkconfig.h"


#include "WiFi.h"
#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <nvs_flash.h>
#include <lwip/dns.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>

#include <string.h>
#include <Task.h>


static char tag[]= "WiFi";


/*
static void setDNSServer(char *ip) {
    ip_addr_t dnsserver;
    ESP_LOGD(tag, "Setting DNS[%d] to %s", 0, ip);
    inet_pton(AF_INET, ip, &dnsserver);
    ESP_LOGD(tag, "ip of DNS is %.8x", *(uint32_t *)&dnsserver);
    dns_setserver(0, &dnsserver);
}
*/

/**
 * @brief Creates and uses a default event handler
 */
WiFi::WiFi()
    : ip(0)
    , gw(0)
    , netmask(0)
    , wifiEventHandler(nullptr)
{
    wifiEventHandler = new WiFiEventHandler();
}

/**
 * @brief Deletes the event handler that was used by the class
 */
WiFi::~WiFi() {
    delete wifiEventHandler;
}

/**
 * @brief Add a reference to a DNS server.
 *
 * Here we define a server that will act as a DNS server.  We can add two DNS
 * servers in total.  The first will be the primary, the second will be the backup.
 * The public Google DNS servers are "8.8.8.8" and "8.8.4.4".
 *
 * For example:
 *
 * @code{.cpp}
 * wifi.addDNSServer("8.8.8.8");
 * wifi.addDNSServer("8.8.4.4");
 * @endcode
 *
 * @param [in] ip The IP address of the DNS Server.
 * @return N/A.
 */
void WiFi::addDNSServer(const std::string& ip) {
    addDNSServer(ip.c_str());
} // addDNSServer

void WiFi::addDNSServer(const char* ip) {
    ip_addr_t dns_server;
    if(inet_pton(AF_INET, ip, &dns_server))
        addDNSServer(ip);
} // addDNSServer

void WiFi::addDNSServer(ip_addr_t ip) {
    ESP_LOGD(tag, "Setting DNS[%d] to %d.%d.%d.%d", m_dnsCount, ((uint8_t*)(&ip))[0], ((uint8_t*)(&ip))[1], ((uint8_t*)(&ip))[2], ((uint8_t*)(&ip))[3]);
    ::dns_setserver(m_dnsCount, &ip);
    m_dnsCount++;
    m_dnsCount %= 2;
} // addDNSServer

/**
 * @brief Set a reference to a DNS server.
 *
 * Here we define a server that will act as a DNS server.  We use numdns to specify which DNS server to set
 *
 * For example:
 *
 * @code{.cpp}
 * wifi.setDNSServer(0, "8.8.8.8");
 * wifi.setDNSServer(1, "8.8.4.4");
 * @endcode
 *
 * @param [in] numdns The DNS number we wish to set
 * @param [in] ip The IP address of the DNS Server.
 * @return N/A.
 */
void WiFi::setDNSServer(int numdns, const std::string& ip) {
    setDNSServer(numdns, ip.c_str());
} // setDNSServer

void WiFi::setDNSServer(int numdns, const char* ip) {
    ip_addr_t dns_server;
    if(inet_pton(AF_INET, ip, &dns_server))
        setDNSServer(numdns, dns_server);
} // setDNSServer

void WiFi::setDNSServer(int numdns, ip_addr_t ip) {
    ESP_LOGD(tag, "Setting DNS[%d] to %d.%d.%d.%d", m_dnsCount, ((uint8_t*)(&ip))[0], ((uint8_t*)(&ip))[1], ((uint8_t*)(&ip))[2], ((uint8_t*)(&ip))[3]);
    ::dns_setserver(numdns, &ip);
} // setDNSServer

/**
 * @brief Connect to an external access point.
 *
 * The event handler will be called back with the outcome of the connection.
 *
 * @param[in] ssid The network SSID of the access point to which we wish to connect.
 * @param[in] password The password of the access point to which we wish to connect.
 * @return N/A.
 */
void WiFi::connectAP(const std::string& ssid, const std::string& password){
    ::nvs_flash_init();
    ::tcpip_adapter_init();
    if (ip != 0 && gw != 0 && netmask != 0) {
        ::tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA); // Don't run a DHCP client

        tcpip_adapter_ip_info_t ipInfo;
        ipInfo.ip.addr = ip;
        ipInfo.gw.addr = gw;
        ipInfo.netmask.addr = netmask;

        ::tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
    }


    ESP_ERROR_CHECK( esp_event_loop_init(wifiEventHandler->getEventHandler(), wifiEventHandler));
    ESP_ERROR_CHECK(::esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(::esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_config_t sta_config;
    ::memset(&sta_config, 0, sizeof(sta_config));
    ::memcpy(sta_config.sta.ssid, ssid.data(), ssid.size());
    ::memcpy(sta_config.sta.password, password.data(), password.size());
    sta_config.sta.bssid_set = 0;
    ESP_ERROR_CHECK(::esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(::esp_wifi_start());

    ESP_ERROR_CHECK(::esp_wifi_connect());
} // connectAP

/**
 * @brief Dump diagnostics to the log.
 */
void WiFi::dump() {
    ESP_LOGD(tag, "WiFi Dump");
    ESP_LOGD(tag, "---------");
    char ipAddrStr[30];
    ip_addr_t ip = ::dns_getserver(0);
    inet_ntop(AF_INET, &ip, ipAddrStr, sizeof(ipAddrStr));
    ESP_LOGD(tag, "DNS Server[0]: %s", ipAddrStr);
} // dump

/**
 * @brief Get the AP IP Info.
 * @return The AP IP Info.
 */
tcpip_adapter_ip_info_t WiFi::getApIpInfo() {
    tcpip_adapter_ip_info_t ipInfo;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ipInfo);
    return ipInfo;
} // getApIpInfo



/**
 * @brief Get the MAC address of the AP interface.
 * @return The MAC address of the AP interface.
 */
std::string WiFi::getApMac() {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    auto mac_str = (char*) malloc(18);
    sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(std::move(mac_str));
} // getApMac


/**
 * @brief Get the AP SSID.
 * @return The AP SSID.
 */
std::string WiFi::getApSSID() {
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_AP, &conf);
    return std::string((char *)conf.sta.ssid);
} // getApSSID


/**
 * @brief Lookup an IP address by host name.
 *
 * @param [in] hostName The hostname to resolve.
 *
 * @return The IP address of the host or 0.0.0.0 if not found.
 */
struct in_addr WiFi::getHostByName(const std::string& hostName) {
    return getHostByName(hostName.c_str());
} // getHostByName

struct in_addr WiFi::getHostByName(const char* hostName) {
    struct in_addr retAddr;
    struct hostent *he = gethostbyname(hostName);
    if (he == nullptr) {
        retAddr.s_addr = 0;
        ESP_LOGD(tag, "Unable to resolve %s - %d", hostName, h_errno);
    } else {
        retAddr = *(struct in_addr *)(he->h_addr_list[0]);
        ESP_LOGD(tag, "resolved %s to %.8x", hostName, *(uint32_t *)&retAddr);
    }
    return retAddr;
} // getHostByName


/**
 * @brief Get the WiFi Mode.
 * @return The WiFi Mode.
 */
std::string WiFi::getMode() {
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    switch(mode) {
        case WIFI_MODE_NULL:
            return "WIFI_MODE_NULL";
        case WIFI_MODE_STA:
            return "WIFI_MODE_STA";
        case WIFI_MODE_AP:
            return "WIFI_MODE_AP";
        case WIFI_MODE_APSTA:
            return "WIFI_MODE_APSTA";
        default:
            return "unknown";
    }
} // getMode


/**
 * @brief Get the STA IP Info.
 * @return The STA IP Info.
 */
tcpip_adapter_ip_info_t WiFi::getStaIpInfo() {
    tcpip_adapter_ip_info_t ipInfo;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
    return ipInfo;
} // getStaIpInfo


/**
 * @brief Get the MAC address of the STA interface.
 * @return The MAC address of the STA interface.
 */
std::string WiFi::getStaMac() {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    auto mac_str = (char*) malloc(18);
    sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(std::move(mac_str));
} // getStaMac


/**
 * @brief Get the STA SSID.
 * @return The STA SSID.
 */
std::string WiFi::getStaSSID() {
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    return std::string((char *)conf.ap.ssid);
} // getStaSSID


/**
 * @brief Perform a WiFi scan looking for access points.
 *
 * An access point scan is performed and a vector of WiFi access point records
 * is built and returned with one record per found scan instance.  The scan is
 * performed in a blocking fashion and will not return until the set of scanned
 * access points has been built.
 *
 * @return A vector of WiFiAPRecord instances.
 */
std::vector<WiFiAPRecord> WiFi::scan() {
    ::nvs_flash_init();
    ::tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(wifiEventHandler->getEventHandler(), wifiEventHandler));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK(::esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(::esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK( esp_wifi_start() );
    wifi_scan_config_t conf;
    memset(&conf, 0, sizeof(conf));
    conf.show_hidden = true;
    esp_err_t rc = ::esp_wifi_scan_start(&conf, true);
    if (rc != ESP_OK) {
        ESP_LOGE(tag, "esp_wifi_scan_start: %d", rc);
    }
    uint16_t apCount;
    rc = ::esp_wifi_scan_get_ap_num(&apCount);
    ESP_LOGD(tag, "Count of found access points: %d", apCount);
    wifi_ap_record_t *list =
      (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, list));
    std::vector<WiFiAPRecord> apRecords;
    for (auto i=0; i<apCount; i++) {
        WiFiAPRecord wifiAPRecord;
        memcpy(wifiAPRecord.m_bssid, list[i].bssid, 6);
        wifiAPRecord.m_ssid = std::string((char *)list[i].ssid);
        wifiAPRecord.m_authMode = list[i].authmode;
        apRecords.push_back(wifiAPRecord);
    }
    free(list);
    return apRecords;
} // scan


/**
 * @brief Start being an access point.
 *
 * @param[in] ssid The SSID to use to advertize for stations.
 * @param[in] password The password to use for station connections.
 * @return N/A.
 */
void WiFi::startAP(const std::string& ssid, const std::string& password) {
    ::nvs_flash_init();
    ::tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(wifiEventHandler->getEventHandler(), wifiEventHandler));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
    wifi_config_t apConfig;
    ::memset(&apConfig, 0, sizeof(apConfig));
    ::memcpy(apConfig.ap.ssid, ssid.data(), ssid.size());
    apConfig.ap.ssid_len = ssid.size();
    ::memcpy(apConfig.ap.password, password.data(), password.size());
    apConfig.ap.channel = 0;
    apConfig.ap.authmode = WIFI_AUTH_OPEN;
    apConfig.ap.ssid_hidden = 0;
    apConfig.ap.max_connection = 4;
    apConfig.ap.beacon_interval = 100;
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_AP, &apConfig) );
    ESP_ERROR_CHECK( esp_wifi_start() );
} // startAP


/**
 * @brief Set the IP info and enable DHCP if ip != 0. If called with ip == 0 then DHCP is enabled.
 * If called with bad values it will do nothing.
 *
 * Do not call this method if we are being an access point ourselves.
 *
 * For example, prior to calling `connectAP()` we could invoke:
 *
 * @code{.cpp}
 * myWifi.setIPInfo("192.168.1.99", "192.168.1.1", "255.255.255.0");
 * @endcode
 *
 * @param [in] ip IP address value.
 * @param [in] gw Gateway value.
 * @param [in] netmask Netmask value.
 * @return N/A.
 */
void WiFi::setIPInfo(const std::string& ip, const std::string& gw, const std::string& netmask) {
    setIPInfo(ip.c_str(), gw.c_str(), netmask.c_str());
} // setIPInfo

void WiFi::setIPInfo(const char* ip, const char* gw, const char* netmask) {
    uint32_t new_ip;
    uint32_t new_gw;
    uint32_t new_netmask;

    auto success = (bool)inet_pton(AF_INET, ip, &new_ip);
    success = success && inet_pton(AF_INET, gw, &new_gw);
    success = success && inet_pton(AF_INET, netmask, &new_netmask);

    if(!success) {
        return;
    }

    setIPInfo(new_ip, new_gw, new_netmask);
} // setIPInfo

void WiFi::setIPInfo(uint32_t ip, uint32_t gw, uint32_t netmask) {
    this->ip = ip;
    this->gw = gw;
    this->netmask = netmask;

    if(ip != 0 && gw != 0 && netmask != 0) {
        tcpip_adapter_ip_info_t ipInfo;
        ipInfo.ip.addr = ip;
        ipInfo.gw.addr = gw;
        ipInfo.netmask.addr = netmask;

        ::tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
        ::tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
    } else {
        ip = 0;
        ::tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_STA);
    }
}

/**
 * @brief Return a string representation of the WiFi access point record.
 *
 * @return A string representation of the WiFi access point record.
 */
std::string WiFiAPRecord::toString() {
    std::string auth;
    switch(getAuthMode()) {
    case WIFI_AUTH_OPEN:
        auth = "WIFI_AUTH_OPEN";
        break;
    case WIFI_AUTH_WEP:
        auth = "WIFI_AUTH_WEP";
        break;
    case WIFI_AUTH_WPA_PSK:
        auth = "WIFI_AUTH_WPA_PSK";
        break;
    case WIFI_AUTH_WPA2_PSK:
        auth = "WIFI_AUTH_WPA2_PSK";
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        auth = "WIFI_AUTH_WPA_WPA2_PSK";
        break;
    default:
        auth = "<unknown>";
        break;
    }
//    std::stringstream s;
//    s<< "ssid: " << m_ssid << ", auth: " << auth << ", rssi: " << m_rssi;
    auto info_str = (char*) malloc(6 + 32 + 8 + 22 + 8 + 3 + 1);
    sprintf(info_str, "ssid: %s, auth: %s, rssi: %d", m_ssid.c_str(), auth.c_str(), (int) m_rssi);
    return std::string(std::move(info_str));
} // toString

MDNS::MDNS() {
    ESP_ERROR_CHECK(mdns_init(TCPIP_ADAPTER_IF_STA, &m_mdns_server));
}

MDNS::~MDNS() {
    if (m_mdns_server != nullptr) {
        mdns_free(m_mdns_server);
    }
    m_mdns_server = nullptr;
}

/**
 * @brief Define the service for mDNS.
 *
 * @param [in] service
 * @param [in] proto
 * @param [in] port
 * @return N/A.
 */
void MDNS::serviceAdd(const std::string& service, const std::string& proto, uint16_t port) {
    serviceAdd(service.c_str(), proto.c_str(), port);
} // serviceAdd


void MDNS::serviceInstanceSet(const std::string& service, const std::string& proto, const std::string& instance) {
    serviceInstanceSet(service.c_str(), proto.c_str(), instance.c_str());
} // serviceInstanceSet


void MDNS::servicePortSet(const std::string& service, const std::string& proto, uint16_t port) {
    servicePortSet(service.c_str(), proto.c_str(), port);
} // servicePortSet


void MDNS::serviceRemove(const std::string& service, const std::string& proto) {
    serviceRemove(service.c_str(), proto.c_str());
} // serviceRemove


/**
 * @brief Set the mDNS hostname.
 *
 * @param [in] hostname The host name to set against the mDNS.
 * @return N/A.
 */
void MDNS::setHostname(const std::string& hostname) {
    setHostname(hostname.c_str());
} // setHostname


/**
 * @brief Set the mDNS instance.
 *
 * @param [in] instance The instance name to set against the mDNS.
 * @return N/A.
 */
void MDNS::setInstance(const std::string& instance) {
    setInstance(instance.c_str());
} // setInstance

/**
 * @brief Define the service for mDNS.
 *
 * @param [in] service
 * @param [in] proto
 * @param [in] port
 * @return N/A.
 */
void MDNS::serviceAdd(const char* service, const char* proto, uint16_t port) {
    ESP_ERROR_CHECK(mdns_service_add(m_mdns_server, service, proto, port));
} // serviceAdd


void MDNS::serviceInstanceSet(const char* service, const char* proto, const char* instance) {
    ESP_ERROR_CHECK(mdns_service_instance_set(m_mdns_server, service, proto, instance));
} // serviceInstanceSet


void MDNS::servicePortSet(const char* service, const char* proto, uint16_t port) {
    ESP_ERROR_CHECK(mdns_service_port_set(m_mdns_server, service, proto, port));
} // servicePortSet


void MDNS::serviceRemove(const char* service, const char* proto) {
    ESP_ERROR_CHECK(mdns_service_remove(m_mdns_server, service, proto));
} // serviceRemove


/**
 * @brief Set the mDNS hostname.
 *
 * @param [in] hostname The host name to set against the mDNS.
 * @return N/A.
 */
void MDNS::setHostname(const char* hostname) {
    ESP_ERROR_CHECK(mdns_set_hostname(m_mdns_server,hostname));
} // setHostname


/**
 * @brief Set the mDNS instance.
 *
 * @param [in] instance The instance name to set against the mDNS.
 * @return N/A.
 */
void MDNS::setInstance(const char* instance) {
    ESP_ERROR_CHECK(mdns_set_instance(m_mdns_server, instance));
} // setInstance
