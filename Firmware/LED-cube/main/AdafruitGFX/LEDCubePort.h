#ifndef __LED_CUBE_PORT_H__
#define __LED_CUBE_PORT_H__

#include "AdafruitGFX/Adafruit_GFX.h"

class LedCubePanel : public Adafruit_GFX {
    public:
        // Constructor
        LedCubePanel(uint8_t panelNumber, int16_t w, int16_t h) : Adafruit_GFX(w, h)
        {
            this->panelIndex = panelNumber;
        }

        // One and only function that will be used to interface with the panels
        void drawPixel(int16_t x, int16_t y, uint16_t color);

        void clear(void);

    private:
        uint8_t panelIndex;
};


#endif // __LED_CUBE_PORT_H__