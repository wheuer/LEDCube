#ifndef __HTTP_SERVER_H_
#define __HTTP_SERVER_H_

#include "esp_check.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t appGETHandler(httpd_req_t* request);
esp_err_t appPOSTHandler(httpd_req_t* request);

#ifdef __cplusplus
}
#endif

#endif // __HTTP_SERVER_H_