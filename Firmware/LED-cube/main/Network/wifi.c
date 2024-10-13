#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_netif_ip_addr.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "mdns.h"

#include "wifi_manager.h"

#include "Network/wifi.h"

/* Event group to signal the status of the connection */
static EventGroupHandle_t   wifiEventGroup; 
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

static const char *TAG = "[NETWORK-WIFI]";
static int connectionAttempts;

// static void connectEventHandler(void* arg, esp_event_base_t event_base,
//                                 int32_t event_id, void* event_data)
// {
//     if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
//     {
//         // Successfully started as a WiFi station, try to connect to the access point
//         esp_wifi_connect();
//     } 
//     else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
//     {
//         // We failed to connect or got disconnected from access point, retry
//         if (connectionAttempts < CONNECT_RETRY_NUM) 
//         {
//             esp_wifi_connect();
//             connectionAttempts++;
//             ESP_LOGI(TAG, "Retrying to connect to the access point");
//         } 
//         else 
//         {
//             // We failed to connect too many times, give up
//             xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);
//         }
//         ESP_LOGI(TAG,"Failed to connect to the access point");
//     } 
//     else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
//     {
//         // Got an IP through DHCP, send out over console and signal wifiConnect function
//         ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
//         ESP_LOGI(TAG, "Assigned IP:" IPSTR, IP2STR(&event->ip_info.ip));
//         connectionAttempts = 0;
//         xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
//     }
// }

// esp_err_t wifiConnect(void)
// {
//     wifiEventGroup = xEventGroupCreate();

//     // Set WiFi mode to station in WiFi registers and default event loop
//     esp_netif_create_default_wifi_sta();

//     // Initalize WiFi 
//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     ESP_ERROR_CHECK(esp_wifi_init(&cfg));

//     // Register our own callback handlers for WiFi activity, this will allow us to monitor the status of the WiFi connection
//     esp_event_handler_instance_t instanceAnyEvent;
//     esp_event_handler_instance_t instanceGotIP;
//     ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
//                                                         ESP_EVENT_ANY_ID,
//                                                         &connectEventHandler,
//                                                         NULL,
//                                                         &instanceAnyEvent));
//     ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
//                                                         IP_EVENT_STA_GOT_IP,
//                                                         &connectEventHandler,
//                                                         NULL,
//                                                         &instanceGotIP));

//     // Prepare to start the WiFi
//     wifi_config_t wifi_config = {
//         .sta = {
//             .ssid = WIFI_SSID,
//             .password = WIFI_PASSWORD,
//         },
//     };
//     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//     ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

//     // Start the WiFi
//     ESP_ERROR_CHECK(esp_wifi_start());

//     ESP_LOGI(TAG, "Attempting WiFi connection...");

//     // Wait until a connection event occurs (triggered by our callback) (either a failure to connect or a successful connection)
//     EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
//             WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
//             pdFALSE,
//             pdFALSE,
//             portMAX_DELAY);

//     // Figure out if we successfully connected or not
//     if (bits & WIFI_CONNECTED_BIT) {
//         ESP_LOGI(TAG, "Connected to access point SSID:%s password:%s",
//                  WIFI_SSID, WIFI_PASSWORD);
//         return ESP_OK;
//     } 
//     else if (bits & WIFI_FAIL_BIT) 
//     {
//         ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
//                  WIFI_SSID, WIFI_PASSWORD);
//     } 
//     else 
//     {
//         ESP_LOGE(TAG, "UNEXPECTED EVENT");
//     }
//     return ESP_FAIL;
// }

void connectionSuccess(void *pvParameter){
	ip_event_got_ip_t* param = (ip_event_got_ip_t*)pvParameter;

	/* transform IP to human readable string */
	char str_ip[16];
	esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);

	ESP_LOGI(TAG, "I have a connection and my IP is %s!", str_ip);

    // Setup to be called once the access point is disabled and we are offically connected to the "real" network
    // Initialize mDNS for IP advertising
    // mdns_ip_addr_t addressList = {
    //     .addr = {
    //         .u_addr.ip4 = param->ip_info.ip,
    //         .type = ESP_IPADDR_TYPE_V4
    //     },
    //     .next = NULL,
    // };
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set("LEDCUBE")); // Access through LEDCUBE.local
    ESP_ERROR_CHECK(mdns_instance_name_set("Slotech LED CUBE")); // General friendly name for the device
    // ESP_ERROR_CHECK(mdns_delegate_hostname_set_address("LEDCUBE", &addressList));
    
    // Add mDNS service to advertise out HTTP server
    // As we are only interested in IP resolution, the only property of the service that will be set is the name of the device
    mdns_txt_item_t serviceTxtData = {
        .key = "name", 
        .value = "Slotech LED CUBE"
    };
    ESP_ERROR_CHECK(mdns_service_add("LEDCUBE-WebServer", "_http", "_tcp", 80, &serviceTxtData, 1));

    xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
}

// void networkStationOnlyCallback(void *pvParameter)
// {
//     // Setup to be called once the access point is disabled and we are offically connected to the "real" network
//     // Initialize mDNS for IP advertising
//     ESP_ERROR_CHECK(mdns_init());
//     ESP_ERROR_CHECK(mdns_hostname_set("LEDCUBE")); // Access through LEDCUBE.local
//     ESP_ERROR_CHECK(mdns_instance_name_set("Slotech LED CUBE")); // General friendly name for the device

//     // Add mDNS service to advertise out HTTP server
//     // As we are only interested in IP resolution, the only property of the service that will be set is the name of the device
//     mdns_txt_item_t serviceTxtData = {
//         .key = "name", 
//         .value = "Slotech LED CUBE"
//     };
//     ESP_ERROR_CHECK(mdns_service_add("LEDCUBE-WebServer", "_http", "_tcp", 80, &serviceTxtData, 1));

//     ESP_LOGI(TAG, "mDNS successfully setup.");
// }

esp_err_t wifiConnect(void)
{
    // Create event group to wait on connection status
    wifiEventGroup = xEventGroupCreate();

    // Setup wifi manager to handle provisioning and connection
    wifi_manager_start();
    wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &connectionSuccess);
    // wifi_manager_set_callback(WM_ORDER_DISCONNECT_STA, &networkStationOnlyCallback);

    // Wait until the wifi manager is able to connect to the wifi
    xEventGroupWaitBits(wifiEventGroup,
            WIFI_CONNECTED_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    return ESP_OK;
}