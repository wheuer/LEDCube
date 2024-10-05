#ifndef __CUBE_H
#define __CUBE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LATCH_PIN 18        // LED Driver latch common for every panel
#define BRIGHTNESS_PIN 23   // LED Driver EN pin common for every panel 
#define MASTER_CLK_PIN 21   // LED Driver CLK pin common for every panel
#define MUX_B0_PIN 22       // Bit 0 of row multiplexers, common for every panel
#define MUX_B1_PIN 19       // Bit 1 of row multiplexers, common for every panel
#define MUX_B2_PIN 4        // Bit 2 of row multiplexers, common for every panel
#define MUX_B3_PIN 17       // Bit 3 of row multiplexers, common for every panel

#define SERIAL_0_PIN 16
#define SERIAL_1_PIN 25
#define SERIAL_2_PIN 26
#define SERIAL_3_PIN 27
#define SERIAL_4_PIN 14
#define SERIAL_5_PIN 13

#define SERIAL_0_PIN_MASK (1 << SERIAL_0_PIN)
#define SERIAL_1_PIN_MASK (1 << SERIAL_1_PIN)
#define SERIAL_2_PIN_MASK (1 << SERIAL_2_PIN)
#define SERIAL_3_PIN_MASK (1 << SERIAL_3_PIN)
#define SERIAL_4_PIN_MASK (1 << SERIAL_4_PIN)
#define SERIAL_5_PIN_MASK (1 << SERIAL_5_PIN)

#define NUM_ROWS 16
#define NUM_COLUMNS 16
#define NUM_PANELS 6

// typedef struct
// {
//     union {
//         uint8_t r;
//         uint8_t red;
//     };
//     union {
//         uint8_t g;
//         uint8_t green;
//     };
//     union {
//         uint8_t b;
//         uint8_t blue;
//     };
// } rgb_t;

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rgb_t;

extern bool frameBuffer[6][NUM_ROWS * NUM_COLUMNS * 3];

void cubeInit(void);
void cubeStart(void);
void cubePause(void);
void setBrightness(float brightness); // 0.0 - 1.0
void setPixel(uint8_t panel, uint8_t x, uint8_t y, rgb_t color);
void clearPanel(uint8_t panel);

#ifdef __cplusplus
}
#endif

#endif // __CUBE_H