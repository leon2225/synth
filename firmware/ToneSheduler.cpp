#include "ToneSheduler.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/clocks.h"

#include "i2s.pio.h"
#include "Tone.h"

extern const uint DEBUG1_PIN;
extern const uint DEBUG2_PIN;
extern const uint DEBUG3_PIN;

inline int32_t sadd(int32_t a, int32_t b);


ToneSheduler::ToneSheduler()
{
    Tone::setupSine();
    dac = &DAC::getInstance();
}

//***************************************************************************************
//* Adds a new tone to the dispatcher
//*
//* returns the channel number of the new tone, or -1 if no channel is available
//***************************************************************************************
int ToneSheduler::addTone(float frequency, uint32_t startTime, float duration, AdsrProfile adsrProfile)
{
    //find a free channel
    for (int i = 0; i < CHANNEL_NUMBER; i++)
    {
        if (currentTones[i].isDone())
        {
            currentTones[i] = Tone(frequency, duration * SAMPLE_RATE, 
                adsrProfile.attackRate, adsrProfile.decayRate, adsrProfile.sustainFactor, adsrProfile.releaseRate);

            // if this is the highest active channel, update the highest active channel
            if (i > highestActiveChannel)
            {
                highestActiveChannel = i;
            }
            return i;
        }
    }

    return -1;
}

void ToneSheduler::cyclicHandler()
{
    volatile uint32_t *buffer;
    uint32_t bufferLength;
    
    bool fillBuffer = dac->cyclicHandler(&buffer, &bufferLength);

    if (fillBuffer)
    {
        fillBufferCallback(buffer, bufferLength);
    }
}

#if 0
bool ToneDispatcher::isDone(uint8_t channel)
{
    bool done = currentTones[channel].isDone();
    
    if (done && channel == highestActiveChannel)
    {
        // find the new highest active channel
        for (int i = highestActiveChannel - 1; i >= 0; i--)
        {
            if (!currentTones[i].isDone())
            {
                highestActiveChannel = i;
                break;
            }
        }
    }

    return currentTones[channel].isDone();
}
#endif

void ToneSheduler::fillBufferCallback(volatile uint32_t* buffer, uint32_t bufferLength)
{
    uint32_t sample = 0;

    // Fill the buffer
    for (size_t i = 0; i < bufferLength; i+=2)
    {
        sample = 0;
        for (size_t channel = 0; channel <= highestActiveChannel; channel++)
        {
            sample = sadd(sample, currentTones[channel].nextSample());
        }

        buffer[i] = sample;
        buffer[i+1] = sample;
    }
}

inline int32_t sadd(int32_t a, int32_t b)
{
    int32_t result;
    if(__builtin_add_overflow(a, b, &result))
    {
        result = a > 0 ? INT32_MAX : INT32_MIN;
    }
    return result;
}