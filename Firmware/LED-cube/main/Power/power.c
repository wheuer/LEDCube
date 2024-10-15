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
static float currentBatteryVoltage;
static bool previousChargingStatus = false;
static bool chargingStatus = false;

float getBatteryVoltage(void)
{
    return currentBatteryVoltage;
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
    currentBatteryVoltage = readBatteryVoltage();

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

    if (currentBatteryVoltage <= BATTERY_LOW_THRESHOLD)
    {
        eventBits |= BATTERY_LOW;
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
    currentBatteryVoltage = readBatteryVoltage();

    // Start updating the status every 250 ms
    esp_timer_start_periodic(powerUpdateTimer, 250 * 1000);
}