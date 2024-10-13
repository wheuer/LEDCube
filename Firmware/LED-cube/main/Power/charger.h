#ifndef __CHARGER_H_
#define __CHARGER_H_

#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

// battery is connected to 0.5 voltage divider on IO35, adc 1 channel 7
#define VBUS_SENSE_PIN GPIO_NUM_36

void initCharger(void);
bool isCharging(void);
void deinitCharger(void);

#ifdef __cplusplus
}
#endif

#endif // __CHARGER_H_