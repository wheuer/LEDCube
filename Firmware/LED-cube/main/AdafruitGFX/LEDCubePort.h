#ifndef __LED_CUBE_PORT_H__
#define __LED_CUBE_PORT_H__

#include "AdafruitGFX/Adafruit_GFX.h"

class LedCubePanel : public Adafruit_GFX {
    public:
        // Default constructor, will need to set a panel number before using
        LedCubePanel() : Adafruit_GFX(16, 16)
        {
            this->panelIndex = 10; // Invalid panel number (won't work until configured)
        }

        // Simple constructor, all LED panels are 16x16 squares
        LedCubePanel(uint8_t panelNumber) : Adafruit_GFX(16, 16)
        {
            this->panelIndex = panelNumber;
        }

        // Set the panel number manually for when using the default constructor
        void setPanelNumber(uint8_t panelNumber);

        // One and only function that will be used to interface with the panels
        void drawPixel(int16_t x, int16_t y, uint16_t color);

        void clear(void);

    private:
        uint8_t panelIndex;
};


#endif // __LED_CUBE_PORT_H__