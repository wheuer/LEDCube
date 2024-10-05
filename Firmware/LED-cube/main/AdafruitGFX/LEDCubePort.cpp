#include "cube.h"
#include "AdafruitGFX/LEDCubePort.h"

// This function is how the AdafruitGFX routines interact with the cube panels
// This is rather slow, but the panels are limited to 16x16
void LedCubePanel::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    // Color that we are given is a 16 bit, 5-6-5, pixel color. Need to isolate and scale
    uint8_t blue = color & 0x1F;
    uint8_t green = (color & 0x7E0) >> 5;
    uint8_t red = (color & 0xF800) >> 11;

    // TODO: Since the panels are only on/off for the RGB channels,
    //       we might want to threshold the values as any value >0 will be 100%

    // Here we are simply writing to the cube buffer that is constantly updated asynchronously
    // The RGB values for the panel is only 1-bit so any value != 0 will be fully lit
    setPixel(this->panelIndex, x, y, (rgb_t){red, green, blue});
}

void LedCubePanel::clear(void)
{
    clearPanel(this->panelIndex);
}
