#include <esp_http_server.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "Network/http_server.h"

#define MIN(a,b) ((a) < (b) ? a : b)

static httpd_handle_t server = NULL;
static const char* TAG = "[NETWORK-HTTP_SERVER]";

static const char* indexHTML = "<!DOCTYPE html>\
                                <html>\
                                <body>\
                                <h2>LED CUBE</h2>\
                                <p>Select an active light theme:</p>\
                                <form action=\"/\" method=\"POST\">\
                                    <input type=\"radio\" id=\"red\" name=\"theme\" value=\"0\">\
                                    <label for=\"red\">All Red</label><br>\
                                    <input type=\"radio\" id=\"green\" name=\"theme\" value=\"1\">\
                                    <label for=\"green\">All Green</label><br>\
                                    <input type=\"radio\" id=\"blue\" name=\"theme\" value=\"2\">\
                                    <label for=\"blue\">All Blue</label><br>\
                                    <input type=\"radio\" id=\"noise\" name=\"theme\" value=\"3\">\
                                    <label for=\"noise\">Noise</label><br>\
                                    <br>\
                                    <button type=\"submit\">Update Theme</button>\
                                </form> \
                                </body>\
                                </html>";

static esp_err_t homeGETHandler(httpd_req_t *req)
{
    httpd_resp_send(req, indexHTML, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static esp_err_t lightEffectPOSTHandler(httpd_req_t *req)
{
    char buf[100];
    int ret;
    int remaining = req->content_len;

    while (remaining > 0) 
    {
        // Read the data for the request, ret should be the length of data that was read
        ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));
        if (ret <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) continue;
            else return ESP_FAIL;
        }

        // Update how much is left to read
        remaining -= ret;

        // Log the data that was received
        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        ESP_LOGI(TAG, "====================================");

        // Try and decode the request
        char response[16];
        if (httpd_query_key_value(buf, "theme", response, 15) == ESP_OK)
        {
            response[15] = '\0';
            ESP_LOGI(TAG, "Found light effect match: %s", response);
        }
        else
        {
            ESP_LOGI(TAG, "Could not find light effect match");
        }
    }

    // Send response to close request
    httpd_resp_send(req, indexHTML, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static const httpd_uri_t homeURI = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = homeGETHandler,
    .user_ctx  = NULL
};

static const httpd_uri_t lightEffectURI = {
    .uri       = "/",
    .method    = HTTP_POST,
    .handler   = lightEffectPOSTHandler,
    .user_ctx  = NULL
};

static httpd_handle_t startHTTPServer(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Setup URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &homeURI);
        httpd_register_uri_handler(server, &lightEffectURI);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void disconnectHandler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    ESP_LOGI(TAG, "Station Disconnected, stopping HTTP server");
    if (httpd_stop(server) == ESP_OK) 
    {
        server = NULL;
    } 
    else 
    {
        ESP_LOGE(TAG, "Failed to stop HTTP server");
    }
    
}

static void connectHandler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    // Got assigned an IP address, either it was changed or we reconnected, either way start the server
    ESP_LOGI(TAG, "Starting webserver");
    startHTTPServer();
}

esp_err_t startServer(void)
{
    // Register callbacks to know if the WiFi connection gets interrupted (not really robust yet)
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connectHandler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnectHandler, NULL));

    if (startHTTPServer() != NULL)
    {
        return ESP_OK;
    }
    else 
    {
        return ESP_FAIL;
    }    
}