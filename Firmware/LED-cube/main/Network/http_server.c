#include <esp_http_server.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "http_app.h"

#include "Network/http_server.h"
#include "Power/power.h"
#include "effects.h"
#include "cube.h"

#define MIN(a,b) ((a) < (b) ? a : b)

static const char* TAG = "[NETWORK-HTTP_SERVER]";

QueueHandle_t newEffectQueueHandle;
static uint8_t newEffectQueueBuffer[sizeof(CubeEffectMessage) * EFFECT_QUEUE_MAX_ITEMS];
static StaticQueue_t newEffectQueue;
static CubeEffectMessage newCubeEffect;

extern const uint8_t app_index_html_start[] asm("_binary_app_index_html_start");
extern const uint8_t app_index_html_end[] asm("_binary_app_index_html_end");

esp_err_t appGETHandler(httpd_req_t* request)
{
    char buffer[32];
    if(strcmp(request->uri, "/") == 0)
    {
        httpd_resp_set_status(request, http_200_hdr);
        httpd_resp_set_type(request, http_content_type_html);
        httpd_resp_send(request, (char*)app_index_html_start, app_index_html_end - app_index_html_start);
    }
    else if (strcmp(request->uri, "/check-in") == 0) 
    {
        // Send back four things: current effect, battery voltage, brightness level, and charge status
        float voltage = getBatteryVoltage();
        Effect currentEffect = getCurrentEffect();
        int brightness = getCurrentBrightness() * 100;
        bool chargeStatus = getChargeStatus();

        snprintf(buffer, 32, "effect=%i&bv=%.2f&bl=%i&cs=%i", currentEffect, voltage, brightness, chargeStatus);

        httpd_resp_set_status(request, http_200_hdr);
        httpd_resp_set_type(request, http_content_type_text);
        httpd_resp_send(request, buffer, HTTPD_RESP_USE_STRLEN);
    }
    else
    {
        httpd_resp_set_status(request, http_404_hdr);
        httpd_resp_send(request, NULL, 0);
    }
    ESP_LOGI(TAG, "GET REQUEST ON %s", request->uri);
    return ESP_OK;
}

esp_err_t appPOSTHandler(httpd_req_t* request)
{
    char buf[128];
    int ret;
    int remaining = request->content_len;

    while (remaining > 0) 
    {
        // Read the data for the request, ret should be the length of data that was read
        ret = httpd_req_recv(request, buf, MIN(remaining, sizeof(buf)));
        if (ret <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) continue;
            else return ESP_FAIL;
        }

        // Update how much is left to read
        remaining -= ret;

        // Log the data that was received
        // ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        // ESP_LOGI(TAG, "%.*s", ret, buf);
        // ESP_LOGI(TAG, "====================================");

        // Try and decode the request
        if(strcmp(request->uri, "/light-effect") == 0)
        {
            // Looking for a "theme" and "brightness" value to set as the active LED theme
            char response[16];
            float newBrightness = -1;
            Effect newEffect = INVALID;
        
            if (httpd_query_key_value(buf, "theme", response, 15) == ESP_OK)
            {
                // atoi returns 0 (INVALID effect) if unable to resolve a number
                newEffect = (Effect) atoi(response);
                if (newEffect >= EFFECT_COUNT_OVERFLOW) newEffect = 0;
            }

            if (httpd_query_key_value(buf, "brightness", response, 15) == ESP_OK)
            {
                newBrightness = atoi(response) / 100.0f;
                if (newBrightness > 1.0) newBrightness = 1.0;
            }
        
            // Set effect values and send to effect queue
            // We will ignore if the queue is full
            newCubeEffect.brightness = newBrightness;
            newCubeEffect.activeEffect = newEffect;
            xQueueSend(newEffectQueueHandle, &newCubeEffect, (TickType_t) 0);
            ESP_LOGI(TAG, "New effect received (effect=%d) (brightness=%.2f)", newEffect, newBrightness);

            // Message should be <128 bytes (smaller than any intermediate buffers) 
            // Break to avoid sending multiple queue items
            break;
        }
    }

    // Need to respond. Currently always just send user back to index.
    httpd_resp_send(request, (char*)app_index_html_start, app_index_html_end - app_index_html_start);

    ESP_LOGI(TAG, "POST REQUEST ON %s", request->uri);
    return ESP_OK;
}

void appServerInit(void)
{
    // Create queue for storing new cube effect messages (requests to change effect)
    newEffectQueueHandle = xQueueCreateStatic(EFFECT_QUEUE_MAX_ITEMS, 
                                              sizeof(CubeEffectMessage), 
                                              newEffectQueueBuffer,
                                              &newEffectQueue);
}
