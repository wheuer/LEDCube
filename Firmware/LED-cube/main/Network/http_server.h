#ifndef __HTTP_SERVER_H_
#define __HTTP_SERVER_H_

#include "esp_check.h"
#include <esp_http_server.h>
#include "effects.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EFFECT_QUEUE_MAX_ITEMS 5

typedef struct CubeEffectMessage {
    float brightness;
    Effect activeEffect;
} CubeEffectMessage;

// Queue to send new effect notifications from HTTP server
extern QueueHandle_t newEffectQueueHandle;

esp_err_t appGETHandler(httpd_req_t* request);
esp_err_t appPOSTHandler(httpd_req_t* request);
void appServerInit(void);

#ifdef __cplusplus
}
#endif

#endif // __HTTP_SERVER_H_