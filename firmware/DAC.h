#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/pio.h"
#include "Tone.h"

#define CHANNEL_NUMBER 10

class DAC {
    public:
        static DAC& getInstance()
        {
            static DAC theOneToRuleThemAll;
            return theOneToRuleThemAll;
        }
        
        int setTone(float frequency, uint32_t duration, uint32_t attack, uint32_t decay, uint32_t sustain, uint32_t release);
        void release(uint8_t channel);
        bool isDone(uint8_t channel);
        void interruptHandler();
        void cyclicHandler();
        void setup(uint lrclkPin, uint bclkPin, uint doutPin);
    private:
        DAC() {}

        uint lrclkPin;
        uint bclkPin;
        uint doutPin;
        
        PIO pio;
        uint sm; 
        uint offset;

        Tone currentTones[CHANNEL_NUMBER];

        uint dmaChannels[2] = {0};
        uint8_t bufferToFill = 0; // 1 for first half, 2 for second half, 0 for none

        int8_t highestActiveChannel = 0;

        void DMASetup();
        int32_t getChannelThatFired();
        void computeBuffer();

    public:
        DAC(DAC const&)             = delete;
        void operator=(DAC const&)  = delete;
};