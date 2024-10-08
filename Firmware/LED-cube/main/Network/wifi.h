#ifndef __WIFI_H_
#define __WIFI_H_

#include "esp_check.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI_SSID           "Heuer536"
#define WIFI_PASSWORD       "6514573915"
#define CONNECT_RETRY_NUM   5

esp_err_t wifiConnect(void);

#ifdef __cplusplus
}
#endif

#endif // __WIFI_H_