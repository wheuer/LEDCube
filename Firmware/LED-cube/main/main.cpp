#include <stdio.h>
#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"

#include "AdafruitGFX/Adafruit_GFX.h"
#include "AdafruitGFX/LEDCubePort.h"
#include "cube.h"

// void LEDUpdater(void* pvParameters)
// {
//     cubeInit();
//     setBrightness(1);
//     uint32_t entropy = 0;
//     uint32_t toggle = 0;
//     while (1)
//     {
//         for (int i = 0; i < 16; i++)
//         {
//             setRow(i);
//             if (toggle > 25)
//             {
//                 entropy = sendAllRandom(1, 0);
//                 toggle = 0;
//             }
//             else
//             {
//                 sendAllRandom(0, entropy);
//             }
//             //vTaskDelay(1 / portTICK_PERIOD_MS);
//         }
//         toggle += 1;
//     }
// }

extern "C" void app_main(void)
{
    printf("Hello little chungus\n");

    // Create Panel object to test drawing things
    LedCubePanel testPanel(4, 16, 16);

    cubeInit();
    setBrightness(1);
    cubeStart();

    while (1)
    {
        testPanel.clear();
        testPanel.drawCircle(7, 7, 5, 0xF800); // Red
        vTaskDelay(1500 / portTICK_PERIOD_MS);
        testPanel.clear();
        testPanel.fillCircle(7, 7, 5, 0xF800); // Red
        vTaskDelay(1500 / portTICK_PERIOD_MS);

        testPanel.clear();
        testPanel.drawCircle(7, 7, 5, 0x07E0); // Green
        vTaskDelay(1500 / portTICK_PERIOD_MS);
        testPanel.clear();
        testPanel.fillCircle(7, 7, 5, 0x07E0); // Green
        vTaskDelay(1500 / portTICK_PERIOD_MS);

        testPanel.clear();
        testPanel.drawCircle(7, 7, 5, 0x001F); // Blue
        vTaskDelay(1500 / portTICK_PERIOD_MS);
        testPanel.clear();
        testPanel.fillCircle(7, 7, 5, 0x001F); // Blue
        vTaskDelay(1500 / portTICK_PERIOD_MS);
    }
    
}

