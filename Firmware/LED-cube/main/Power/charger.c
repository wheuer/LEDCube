#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_timer.h"
#include <esp_log.h>

#include "Power/charger.h"

const static char* TAG = "[POWER-CHARGER]";

static esp_timer_handle_t chargerUpdateTimer;
volatile static bool chargeStatus = false;

static void updateChargingStatus(void* args)
{
    // Unfortunately the VBUS pin voltage should be ~2.5V with the VOH voltage of the ESP32 being 2.475V
    // If this basic reading is unreliable we can use the ADC 
    if (gpio_get_level(VBUS_SENSE_PIN))
    {
        chargeStatus = true;
    }
    else 
    {
        chargeStatus = false;
    }
    ESP_LOGI(TAG, "Charge status: %d", chargeStatus);
}

bool isCharging(void)
{
    return chargeStatus;
}

void initCharger(void)
{  
    // Configure VBUS sense pin
    // It is not recommended to use the VBUS pin (36) interrupts while using ADC1, see ESP idf and ESP32 GPIO errata
    gpio_config_t myGPIO;
    myGPIO.pin_bit_mask = (1ULL << VBUS_SENSE_PIN);
    myGPIO.mode = GPIO_MODE_INPUT;
    myGPIO.intr_type = GPIO_INTR_DISABLE;
    myGPIO.pull_down_en = 0;
    myGPIO.pull_up_en = 0;
    gpio_config(&myGPIO);

    // Since we cannot use interrupts :( we will have to have a periodic polling timer
    esp_timer_create_args_t updateChargingStatusTimerConfig = {
        .callback = updateChargingStatus,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "ChargerUpdateTimer",
        true // Skip unhandeled events in light sleep
    };
    esp_timer_create(&updateChargingStatusTimerConfig, &chargerUpdateTimer);

    // Start updating the charger status every 500 ms
    esp_timer_start_periodic(chargerUpdateTimer, 500 * 1000);
}

void deinitCharger(void)
{
    esp_timer_stop(chargerUpdateTimer);
}