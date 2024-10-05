#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "soc/soc.h"
#include "soc/gpio_reg.h"

#include "cube.h"

static esp_timer_handle_t cubeTickTimer;

// Private fast version of frame buffer
// Two values for all panels, first value are the bits to set and the second are the bits to clear
static uint32_t fastFrameBuffer[NUM_ROWS * NUM_COLUMNS * 3 * 2]; // 3 LEDs (RGB) per LED and 2 values per LED 
bool frameBuffer[6][NUM_ROWS * NUM_COLUMNS * 3];

void setPixel(uint8_t panel, uint8_t x, uint8_t y, rgb_t color)
{
    frameBuffer[panel][((y * NUM_COLUMNS + x) * 3) + 0] = color.red;
    frameBuffer[panel][((y * NUM_COLUMNS + x) * 3) + 1] = color.green;
    frameBuffer[panel][((y * NUM_COLUMNS + x) * 3) + 2] = color.blue;
}

void clearPanel(uint8_t panel)
{
    memset(frameBuffer[panel], 0, sizeof(frameBuffer[panel]));
}

void setBrightness(float brightness)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (uint16_t) 8192 * (1 - brightness));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

static void convertFastFrameBuffer(void)
{
    // Convert the normal frame buffer into the fast format
    uint32_t fastBufferIndex = 0;
    uint32_t currentSetValue = 0;
    uint32_t currentClearValue = 0;
    for (int i = 0; i < NUM_ROWS; i++)
    {
        for (int j = 47; j >= 0; j--)
        {
            // Check the value for every panel and add it to the running display value
            frameBuffer[0][(i * NUM_COLUMNS * 3) + j] ? (currentSetValue |= SERIAL_0_PIN_MASK) : (currentClearValue |= SERIAL_0_PIN_MASK);
            frameBuffer[1][(i * NUM_COLUMNS * 3) + j] ? (currentSetValue |= SERIAL_1_PIN_MASK) : (currentClearValue |= SERIAL_1_PIN_MASK);
            frameBuffer[2][(i * NUM_COLUMNS * 3) + j] ? (currentSetValue |= SERIAL_2_PIN_MASK) : (currentClearValue |= SERIAL_2_PIN_MASK);
            frameBuffer[3][(i * NUM_COLUMNS * 3) + j] ? (currentSetValue |= SERIAL_3_PIN_MASK) : (currentClearValue |= SERIAL_3_PIN_MASK);
            frameBuffer[4][(i * NUM_COLUMNS * 3) + j] ? (currentSetValue |= SERIAL_4_PIN_MASK) : (currentClearValue |= SERIAL_4_PIN_MASK);
            frameBuffer[5][(i * NUM_COLUMNS * 3) + j] ? (currentSetValue |= SERIAL_5_PIN_MASK) : (currentClearValue |= SERIAL_5_PIN_MASK);

            // Add the display values to the fast buffer
            fastFrameBuffer[fastBufferIndex++] = currentSetValue;
            fastFrameBuffer[fastBufferIndex++] = currentClearValue;

            // Reset display values for next subpixel
            currentSetValue = 0;
            currentClearValue = 0;
        }
    }
}

static void cubeTickCallback(void* args)
{
    static uint8_t currentRow = 0;

    // Data is organized to flip flop writing the set and clear GPIO registers
    uint32_t writeIndex = currentRow * NUM_COLUMNS * 3 * 2;
    for (int i = 0; i < NUM_COLUMNS * 3; i++)
    {
        // Set the GPIO values to the correct output state
        REG_WRITE(GPIO_OUT_W1TS_REG, fastFrameBuffer[writeIndex++]);
        REG_WRITE(GPIO_OUT_W1TC_REG, fastFrameBuffer[writeIndex++]);

        // CLK the LED shift registers
        REG_WRITE(GPIO_OUT_W1TC_REG, (1 << MASTER_CLK_PIN)); 
        REG_WRITE(GPIO_OUT_W1TS_REG, (1 << MASTER_CLK_PIN)); 
        REG_WRITE(GPIO_OUT_W1TC_REG, (1 << MASTER_CLK_PIN));        
    }

    // Set the row mux, just use API here to avoid ugly code, could likely be faster if needed
    gpio_set_level(MUX_B0_PIN, currentRow & 1);
    gpio_set_level(MUX_B1_PIN, (currentRow & 2) >> 1);
    gpio_set_level(MUX_B2_PIN, (currentRow & 4) >> 2);
    gpio_set_level(MUX_B3_PIN, (currentRow & 8) >> 3);

    // Latch the LED shift registers and row mux
    REG_WRITE(GPIO_OUT_W1TC_REG, (1 << LATCH_PIN)); 
    REG_WRITE(GPIO_OUT_W1TS_REG, (1 << LATCH_PIN)); 
    REG_WRITE(GPIO_OUT_W1TC_REG, (1 << LATCH_PIN));  

    // Increment current row, updating the display if starting new frame
    if (currentRow == 15)
    {
        // Starting new frame on next row draw, update fastFrameBuffer
        convertFastFrameBuffer();
        currentRow = 0;
    }
    else
    {
        currentRow++;
    }
}

void cubeInit(void)
{
    /*
        Configure all the used GPIO
    */
    gpio_config_t myGPIO;
    myGPIO.pin_bit_mask = (1 << LATCH_PIN) | (1 << MASTER_CLK_PIN)
        | (1 << MUX_B0_PIN) | (1 << MUX_B1_PIN) | (1 << MUX_B2_PIN)
        | (1 << MUX_B3_PIN) | (1 << SERIAL_0_PIN) | (1 << SERIAL_1_PIN)
        | (1 << SERIAL_2_PIN) | (1 << SERIAL_3_PIN) | (1 << SERIAL_4_PIN)
        | (1 << SERIAL_5_PIN); // | (1 << BRIGHTNESS_PIN)
    myGPIO.mode = GPIO_MODE_OUTPUT;
    myGPIO.intr_type = GPIO_INTR_DISABLE;
    myGPIO.pull_down_en = 0;
    myGPIO.pull_up_en = 0;
    gpio_config(&myGPIO);

    /*
        Configure a LEDc timer for the brightness PWM generation
    */
    ledc_timer_config_t ledcTimer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 4096,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledcTimer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = BRIGHTNESS_PIN,
        .duty           = 0, // Set duty to 0% to start
        .hpoint         = 0
    };

    ledc_channel_config(&ledc_channel);

    /*
        Configure the 1ms cube tick timer that will trigger update events
    */
    esp_timer_create_args_t cubeTickTimerConfig = {
        .callback = cubeTickCallback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK, // In task for now, if timing gets wonky switch to interrupt
        .name = "CubeTickTimer",
        true // Skip unhandeled events in light sleep
    };
    esp_timer_create(&cubeTickTimerConfig, &cubeTickTimer);

    /*
        Fill default value into fast frame buffer
    */
    uint32_t defaultPattern[18] = {
        0x0E016000, 0x00000000, // B
        0x00000000, 0x0E016000, // -
        0x00000000, 0x0E016000, // -
        0x00000000, 0x0E016000, // -
        0x0E016000, 0x00000000, // G
        0x00000000, 0x0E016000, // -
        0x00000000, 0x0E016000, // -
        0x00000000, 0x0E016000, // -
        0x0E016000, 0x00000000, // R
    };

    uint32_t bufferIndex = 0;
    for (int i = 0; i < NUM_ROWS; i++)
    {   
        // Do the last 15 LEDs as the pattern is in groups of 3
        for (int j = 0; j < 5; j++)
        {
            memcpy(&fastFrameBuffer[bufferIndex], defaultPattern, 18 * 4);
            bufferIndex += 18;
        }
        // Do the final (first) LED
        memcpy(&fastFrameBuffer[bufferIndex], defaultPattern, 6 * 4);
        bufferIndex += 6;
    } 
}

void cubeStart(void)
{
    esp_timer_start_periodic(cubeTickTimer, 1000); // 1 ms period
}

void cubePause(void)
{
    esp_timer_stop(cubeTickTimer);
}





