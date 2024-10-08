#include <stdio.h>
#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"

#include "AdafruitGFX/Adafruit_GFX.h"
#include "AdafruitGFX/LEDCubePort.h"
#include "cube.h"
#include "Network/network.h"

#define PURE_RED    0xF800
#define PURE_GREEN  0x07E0
#define PURE_BLUE   0x001F

typedef enum LIGHT_MODE {
    MODE_OFF,
    MODE_RED,
    MODE_GREEN,
    MODE_BLUE,
    MODE_RANDOM,
} LIGHT_MODE;

LedCubePanel panels[6];
LIGHT_MODE currentLightMode;

void initPanels(void)
{
    for (int i = 0; i < 6; i++)
    {
        // Don't need to clear the panel, the screen buffers are initalized to zero
        panels[i].setPanelNumber(i);
    }
}

extern "C" void app_main(void)
{
    // printf("Hello little chungus\n");

    // initPanels();

    // cubeInit();
    // setBrightness(0.25);
    // cubeStart();
    
    // while (1)
    // {
    //     fillRandom();
    //     vTaskDelay(200 / portTICK_PERIOD_MS);
    // }

    printf("Hopefully this works...");
    networkLaunch();

    
}

