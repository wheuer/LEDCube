#ifndef __POWER_H_
#define __POWER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define POWER_UPDATE_INTERVAL 250 // ms
#define BATTERY_MEASUREMENTS_PER_REPORT 10

#define BATTERY_LOW_THRESHOLD 3.20 
#define BATTERY_OK_THRESHOLD 3.50

typedef enum PowerEvent {
    CHARGER_DISCONNECTED = 1,
    CHARGER_CONNECTED = 2,
    BATTERY_LOW = 4,
    BATTERY_OK = 8
} PowerEvent;

extern EventGroupHandle_t powerEventGroup; 

float getBatteryVoltage(void);
bool getChargeStatus(void);
void powerInit(void);

#ifdef __cplusplus
}
#endif

#endif // __POWER_H_