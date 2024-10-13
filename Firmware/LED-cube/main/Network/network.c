#include "esp_event.h"
#include <esp_log.h>
#include <nvs_flash.h>
#include "esp_netif.h"
#include "esp_check.h"

#include "Network/network.h"
#include "Network/wifi.h"
#include "Network/http_server.h"

static const char *TAG = "[NETWORK]";

void networkLaunch(void)
{
    // Boilerplate network stack initilization
    // ESP_ERROR_CHECK(nvs_flash_init());
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Attempt connection to WiFi
    if (wifiConnect() == ESP_OK)
    {

        // Connected to WiFi, we can start our http webserver
        // if (startServer() == ESP_OK)
        // {
        //     ESP_LOGI(TAG, "HTTP server started");
        // }
        // else
        // {
        //     ESP_LOGE(TAG, "Failed to start HTTP server");
        // }
    }
    else
    {
        ESP_LOGE(TAG, "Failed to connect to WiFi");
    }
}


