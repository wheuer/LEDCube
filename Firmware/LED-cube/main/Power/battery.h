#ifndef __BATTERY_H_
#define __BATTERY_H_

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#ifdef __cplusplus
extern "C" {
#endif

// battery is connected to 0.5 voltage divider on IO35, adc 1 channel 7
#define BATTERY_ADC_CHANNEL ADC_CHANNEL_7

void initBattery(void);
void deinitBattery(void);
float readBatteryVoltage(void);


#ifdef __cplusplus
}
#endif

#endif // __BATTERY_H_