#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_timer.h"
#include <esp_log.h>

#include "Power/charger.h"

const static char* TAG = "[POWER-CHARGER]";

bool isCharging(void)
{
    // Unfortunately the VBUS pin voltage should be ~2.5V with the VOH voltage of the ESP32 being 2.475V
    // If this basic reading is unreliable we can use the ADC 
    return gpio_get_level(VBUS_SENSE_PIN);
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
}
