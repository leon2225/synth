#pragma once

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/pio.h"
#include "Tone.h"

class DAC {
    public:
        static DAC& getInstance()
        {
            static DAC theOneToRuleThemAll;
            return theOneToRuleThemAll;
        }
        
        void interruptHandler();
        bool cyclicHandler(volatile uint32_t** buffer, uint32_t *bufferLength);
        void setup(uint32_t lrclkPin, uint32_t bclkPin, uint32_t doutPin, uint32_t sampleRate);
    private:
        DAC() {}

        uint32_t lrclkPin;
        uint32_t bclkPin;
        uint32_t doutPin;
        
        PIO pio;
        uint32_t sm; 
        uint32_t offset;

        uint32_t dmaChannels[2] = {0};
        uint8_t bufferToFill = 0; // 1 for first half, 2 for second half, 0 for none

        void DMASetup();
        int32_t getChannelThatFired();
        void computeBuffer();

    public:
        DAC(DAC const&)             = delete;
        void operator=(DAC const&)  = delete;
};