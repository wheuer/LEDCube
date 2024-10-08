#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "Network/wifi.h"

/* Event group to signal the status of the connection */
static EventGroupHandle_t   wifiEventGroup; 
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

static const char *TAG = "[NETWORK-WIFI]";
static int connectionAttempts;

static void connectEventHandler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        // Successfully started as a WiFi station, try to connect to the access point
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        // We failed to connect or got disconnected from access point, retry
        if (connectionAttempts < CONNECT_RETRY_NUM) 
        {
            esp_wifi_connect();
            connectionAttempts++;
            ESP_LOGI(TAG, "Retrying to connect to the access point");
        } 
        else 
        {
            // We failed to connect too many times, give up
            xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"Failed to connect to the access point");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        // Got an IP through DHCP, send out over console and signal wifiConnect function
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Assigned IP:" IPSTR, IP2STR(&event->ip_info.ip));
        connectionAttempts = 0;
        xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifiConnect(void)
{
    wifiEventGroup = xEventGroupCreate();

    // Set WiFi mode to station in WiFi registers and default event loop
    esp_netif_create_default_wifi_sta();

    // Initalize WiFi 
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register our own callback handlers for WiFi activity, this will allow us to monitor the status of the WiFi connection
    esp_event_handler_instance_t instanceAnyEvent;
    esp_event_handler_instance_t instanceGotIP;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &connectEventHandler,
                                                        NULL,
                                                        &instanceAnyEvent));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &connectEventHandler,
                                                        NULL,
                                                        &instanceGotIP));

    // Prepare to start the WiFi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

    // Start the WiFi
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Attempting WiFi connection...");

    // Wait until a connection event occurs (triggered by our callback) (either a failure to connect or a successful connection)
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    // Figure out if we successfully connected or not
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to access point SSID:%s password:%s",
                 WIFI_SSID, WIFI_PASSWORD);
        return ESP_OK;
    } 
    else if (bits & WIFI_FAIL_BIT) 
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 WIFI_SSID, WIFI_PASSWORD);
    } 
    else 
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    return ESP_FAIL;
}