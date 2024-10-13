#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include <esp_log.h>

#include "Power/battery.h"

const static char* TAG = "[POWER-BATTERY]";

static adc_oneshot_unit_handle_t adc1_handle = NULL;
static adc_cali_handle_t cali_handle;

void initBattery(void)
{
    // Init ADC
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // Configure ADC
    // Maximum read battery voltage is 4.2/2 = 2.1 V, still need highest attenuation of  12 dB
    // 12 dB will have an effective measure range of 150 - 2450 mV
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, BATTERY_ADC_CHANNEL, &config));

    // Perform ADC calibration
    esp_err_t ret = ESP_FAIL;
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle);
    if (ret == ESP_OK) 
    {
        ESP_LOGI(TAG, "ADC Calibrated");
    }
    else
    {
        ESP_LOGW(TAG, "ADC Calibration FAILED");
    }
}

void deinitBattery(void)
{
    // Free ADC data structures
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));

    // Reset ADC calibration data
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(cali_handle));
}

float readBatteryVoltage(void)
{
    if (adc1_handle == NULL)
    {
        ESP_LOGW(TAG, "Called readBatteryVoltage() without initializing ADC");
        return 0.0;
    }
    int rawValue;
    int measuredVoltage;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, BATTERY_ADC_CHANNEL, &rawValue));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, rawValue, &measuredVoltage));
    return (measuredVoltage / 1000.0) * 2;
}

