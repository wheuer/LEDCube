#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_timer.h"
#include <esp_log.h>

#include "Power/charger.h"
#include "Power/battery.h"
#include "Power/power.h"

const static char* TAG = "[POWER-MONITOR]";

EventGroupHandle_t powerEventGroup; 
static esp_timer_handle_t powerUpdateTimer;
static uint8_t batteryVoltageIndex = 0;
static bool voltageStable = false; // We have not taken at least BATTERY_MEASUREMENTS_PER_REPORT yet, it would be skewed by zeros in buffer
static float averageBatteryVoltage; // Over the last BATTERY_MEASUREMENTS_PER_REPORT raw readings 
static float batteryVoltages[BATTERY_MEASUREMENTS_PER_REPORT];
static bool previousChargingStatus = false;
static bool chargingStatus = false;

float getBatteryVoltage(void)
{
    return averageBatteryVoltage;
}

bool getChargeStatus(void)
{
    return chargingStatus;
}

static void updateStatus(void* args)
{
    // Update
    previousChargingStatus = chargingStatus;
    chargingStatus = isCharging();
    batteryVoltages[batteryVoltageIndex] = readBatteryVoltage();

    // Mark the first time the buffer fills aka first "real" measurement
    if (batteryVoltageIndex == 9) voltageStable = true;

    // Make sure the first measuremetns are not skewed by a partially zeroed buffer
    if (voltageStable)
    {
        float sum = 0.0;
        for (int i = 0; i < BATTERY_MEASUREMENTS_PER_REPORT; i++) sum += batteryVoltages[i];
        averageBatteryVoltage = sum / BATTERY_MEASUREMENTS_PER_REPORT;
    } 
    else
    {
        averageBatteryVoltage = batteryVoltages[batteryVoltageIndex];
    }

    // Increment with wrapping 
    batteryVoltageIndex = (batteryVoltageIndex + 1) % BATTERY_MEASUREMENTS_PER_REPORT;

    // Send events if needed
    uint32_t eventBits = 0;
    if (chargingStatus && !previousChargingStatus)
    {
        eventBits |= CHARGER_CONNECTED;
        ESP_LOGI(TAG, "Charger connected");
    } 
    else if (!chargingStatus && previousChargingStatus)
    {
        eventBits |= CHARGER_DISCONNECTED;
        ESP_LOGI(TAG, "Charger disconnected");
    }

    if (averageBatteryVoltage <= BATTERY_LOW_THRESHOLD)
    {
        eventBits |= BATTERY_LOW;
    }
    else if (averageBatteryVoltage >= BATTERY_OK_THRESHOLD)
    {
        eventBits |= BATTERY_OK;
    }

    if (eventBits) xEventGroupSetBits(powerEventGroup, eventBits);
}

void powerInit(void)
{
    // Initalize the battery and charger utilities
    initBattery();
    initCharger(); 

    // Create power event group for signaling events
    powerEventGroup = xEventGroupCreate();

    // Create periodic timer for updating battery voltage and checking for events
    esp_timer_create_args_t updateStatusTimerConfig = {
        .callback = updateStatus,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "PowerUpdateTimer",
        true // Skip unhandeled events in light sleep
    };
    esp_timer_create(&updateStatusTimerConfig, &powerUpdateTimer);

    // Set initial state
    chargingStatus = isCharging();
    averageBatteryVoltage = readBatteryVoltage();

    // Start updating the status every POWER_UPDATE_INTERVAL ms
    esp_timer_start_periodic(powerUpdateTimer, POWER_UPDATE_INTERVAL * 1000);
}