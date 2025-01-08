#include <stdio.h>
#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "driver/gpio.h"

#include "AdafruitGFX/Adafruit_GFX.h"
#include "AdafruitGFX/LEDCubePort.h"
#include "cube.h"
#include "Network/network.h"
#include "Network/http_server.h"
#include "Power/power.h"
#include "effects.h"

#define PURE_RED    0xF800
#define PURE_GREEN  0x07E0
#define PURE_BLUE   0x001F

const static char* TAG = "[MAIN]";

LedCubePanel panels[6];

static Effect previousLightMode = OFF;
static Effect currentLightMode = OFF;

// How often the update function is called in units of TASK_BASE_UPDATE_INTERVAL
// The check interval is about the minimum interval for update effects to be reflected
static uint32_t effectUpdateInterval = 0;
static uint32_t effectUpdateIntervalCounter = 0;

static CubeEffectMessage newCubeEffect;

extern "C" Effect getCurrentEffect(void)
{
    return currentLightMode;
}

static void initPanels(void)
{
    for (int i = 0; i < 6; i++)
    {
        // Don't need to clear the panel, the screen buffers are initalized to zero
        panels[i].setPanelNumber(i);
    }
}

// Handler for updating the current effect
// This is called at the effectUpdateInterval
static void updateCurrentEffect(void)
{
    switch (currentLightMode)
    {
        case CHARGING:
            break;
        case RED:
            setAll((rgb_t) {1, 0, 0});
            break;
        case GREEN:
            setAll((rgb_t) {0, 1, 0});
            break;
        case BLUE:
            setAll((rgb_t) {0, 0, 1});
            break;
        case WHITE:
            setAll((rgb_t) {1, 1, 1});
            break;
        case NOISE:
            fillRandom();
            break;
        default:
            break;
    }
}

// Update all state variables to default for current effect
// This function is called once at the start of the effect 
//      and should handle any required initialization
static void changeEffect(void)
{
    ESP_LOGI(TAG, "Changing effect to %d", currentLightMode);

    // For every effect we directly clear all the panels
    for (int i = 0; i < 6; i++) clearPanel(i);

    switch (currentLightMode)
    {
        case OFF:
            effectUpdateInterval = OFF_UPDATE_INTERVAL;
            setBrightness(0.0);
            break;
        case CHARGING:
            effectUpdateInterval = CHARGING_UPDATE_INTERVAL;
            break;
        case WAIT_FOR_CHARGE:
            effectUpdateInterval = WAIT_FOR_CHARGE_UPDATE_INTERVAL;
            break;
        case NOISE:
            effectUpdateInterval = NOISE_UPDATE_INTERVAL;
            break;
        default:
            // This currently covers red, green, blue, and white
            effectUpdateInterval = DEFAULT_UPDATE_INTERVAL;
            break;
    }
    updateCurrentEffect();
}

static void appInit(void)
{
    initPanels();
    cubeInit();
    powerInit();
    cubeStart();
}

extern "C" void app_main(void)
{
    appInit();

    // The call to launch the network will block until we are connected so do it after all init is complete
    networkLaunch();

    EventBits_t eventBits;
    while (1)
    {
        /*
        Main will wait on power event group with a timeout equal to the TASK_BASE_UPDATE_INTERVAL
            -> If resumed on power event, handle accordingly
            -> If resumed on timeout
                -> Check for incoming effect update requests
                -> Increment update interval counter and update panels if needed
        */
       eventBits = xEventGroupWaitBits(powerEventGroup,
                                       CHARGER_DISCONNECTED | CHARGER_CONNECTED | BATTERY_LOW | BATTERY_OK,
                                       pdTRUE, // Clear on exit
                                       pdFALSE,
                                       TASK_BASE_UPDATE_INTERVAL / portTICK_PERIOD_MS);

        if (eventBits && !(eventBits & BATTERY_OK)) ESP_LOGI(TAG, "Event bits %d", (int) eventBits);

        if (eventBits & BATTERY_LOW && currentLightMode != WAIT_FOR_CHARGE)
        {
            // The battery is now uncomfortably low, save effect and transition to waiting for charge state
            previousLightMode = currentLightMode;
            currentLightMode = WAIT_FOR_CHARGE;
            changeEffect();
        }

        if (eventBits & BATTERY_OK && currentLightMode == WAIT_FOR_CHARGE)
        {
            // We were waiting but now the battery voltage is good, could have been a glitch or large current spike
            // Act like it never happened...
            currentLightMode = previousLightMode;
            previousLightMode = OFF;
            changeEffect();
        }
        
        if (eventBits & CHARGER_CONNECTED && currentLightMode != CHARGING)
        {
            // We are now charging, save effect and transition to charging state
            previousLightMode = currentLightMode;
            currentLightMode = CHARGING;
            changeEffect();
        }
        else if (eventBits & CHARGER_DISCONNECTED && currentLightMode == CHARGING)
        {
            // We just stopped charging, revert to previous effect
            currentLightMode = previousLightMode;
            previousLightMode = CHARGING;
            changeEffect();

            // Clear queue so we don't immediately update to any cached requests
            while(xQueueReceive(newEffectQueueHandle, &newCubeEffect, 0));
        }

        if (currentLightMode != CHARGING && currentLightMode != WAIT_FOR_CHARGE)
        {
            // Not charging or waiting for charge so we have a valid active effect, check for new effects
            if (xQueueReceive(newEffectQueueHandle, &newCubeEffect, 0))
            {
                if (newCubeEffect.activeEffect != INVALID)
                {
                    previousLightMode = currentLightMode;
                    currentLightMode = newCubeEffect.activeEffect;
                    setBrightness(newCubeEffect.brightness);
                    changeEffect();
                }
            }
        }

        // Update the panel update interval counter and update if needed
        if (++effectUpdateIntervalCounter >= effectUpdateInterval) 
        {
            updateCurrentEffect();
            effectUpdateIntervalCounter = 0;
        }
    }
}

