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

#define USE_DEBUG_PINS 0

extern const uint DEBUG1_PIN;
extern const uint DEBUG2_PIN;
extern const uint DEBUG3_PIN;

static char strBuffer[100];

inline int32_t sadd(int32_t a, int32_t b);


ToneSheduler::ToneSheduler()
{
    Tone::setupSine();
    dac = &DAC::getInstance();
}

int ToneSheduler::addToneRel(float frequency, float relStartTime_sec, float duration, AdsrProfile adsrProfile)
{
    uint32_t startTime_sam = relStartTime_sec * SAMPLE_RATE;
    return addToneRaw(frequency, startTime_sam + currentTime, duration, adsrProfile);
}

int ToneSheduler::addToneAbs(float frequency, float startTime_sec, float duration, AdsrProfile adsrProfile)
{
    uint32_t startTime_sam = startTime_sec * SAMPLE_RATE;
    return addToneRaw(frequency, startTime_sam, duration, adsrProfile);
}

//***************************************************************************************
//* Adds a new tone to the dispatcher
//*
//* 
//***************************************************************************************
int ToneSheduler::addToneRaw(float frequency, uint32_t startTime_sam, float duration, AdsrProfile adsrProfile)
{

    //parameter check
    if (frequency < 0 || frequency > SAMPLE_RATE/2)
    {
        return -1;
    }
    if (!jobQueue.empty() && (startTime_sam < jobQueue.front().startTime))
    {
        return -2;
    }
    if (duration < 0)
    {
        return -3;
    }

    //check if the queue is full
    if (placeLeftInQueue == 0)
    {
        return -4;
    }

    //add the tone to the queue
    Tone tone = Tone(frequency, duration * SAMPLE_RATE, 
        adsrProfile.attackRate, adsrProfile.decayRate, adsrProfile.sustainFactor, adsrProfile.releaseRate);

    placeLeftInQueue--;
    jobQueue.push(ToneJob(tone, startTime_sam));
    return 0;
}

bool ToneSheduler::startTone(Tone tone)
{
    //find a free channel
    for (int channelIndex = 0; channelIndex < CHANNEL_NUMBER; channelIndex++)
    {
        if (currentTones[channelIndex].isDone())
        {
            //Channel is free, overwrite it
            currentTones[channelIndex] = tone;

            // if this is the highest active channel, update the highest active channel
            if (channelIndex > highestActiveChannel)
            {
                highestActiveChannel = channelIndex;
                //sprintf(strBuffer, ">N: %d\n", highestActiveChannel);
                //uart_puts(uart0, strBuffer);
            }



            return true;
        }
    }

    return false;
}

void ToneSheduler::handleDoneTone(uint8_t channel)
{
    // if this is the highest active channel, update the highest active channel
    if (channel == highestActiveChannel)
    {
        // find the new highest active channel
        for (int i = highestActiveChannel - 1; i >= 0; i--)
        {
            if (!currentTones[i].isDone())
            {
                highestActiveChannel = i;
                //sprintf(strBuffer, ">N: %d\n", highestActiveChannel);
                //uart_puts(uart0, strBuffer);
                return;
            }
        }
        highestActiveChannel = -1;
        //sprintf(strBuffer, ">N: %d\n", highestActiveChannel);
        //uart_puts(uart0, strBuffer);
    }
}

void ToneSheduler::cyclicHandler()
{
    volatile uint32_t *buffer;
    uint32_t bufferLength;
    
    bool fillBuffer = dac->cyclicHandler(&buffer, &bufferLength);

    if (fillBuffer)
    {
        //gpio_xor_mask(USE_DEBUG_PINS<<DEBUG3_PIN);
        fillBufferCallback(buffer, bufferLength);
        //gpio_xor_mask(USE_DEBUG_PINS<<DEBUG3_PIN);
    }
}

bool ToneSheduler::busy()
{
    return !jobQueue.empty() || highestActiveChannel != -1;
}

void ToneSheduler::fillBufferCallback(volatile uint32_t* buffer, uint32_t bufferLength)
{
    uint32_t sample = 0;
    bool startSuccess = false;

    // Check if any jobs from the queue has to be started for this buffer
    // Therefor check if the first (earliest) job in the queue has to be started before the end of the buffer 
    // (check for bufferlength/2 because of stereo buffering)
    bool sheduleNewJob =(!jobQueue.empty() && jobQueue.front().startTime <= (currentTime + (bufferLength/2)));
    uint32_t startTime = UINT32_MAX;
    if (sheduleNewJob)
    {
        startTime = jobQueue.front().startTime;
    }
    
    // Fill the buffer
    for (size_t i = 0; i < bufferLength; i+=2)
    {
        // Check if a new job has to be started
        if (startTime <= currentTime)
        {
            startSuccess = startTone(jobQueue.front().tone);
            if(startSuccess)
            {
                // if the job was started successfully, remove it from the queue
                // and update the startTime for the next job
                jobQueue.pop();
                placeLeftInQueue++;
                startTime = jobQueue.empty() ? UINT32_MAX : jobQueue.front().startTime;
            }
        }

        // calculate the sample
        sample = 0;
        for (int channel = 0; channel <= highestActiveChannel; channel++)
        {
            sample = sadd(sample, currentTones[channel].nextSample());
        }

        // update the current time and fill the stereo buffer
        currentTime ++;
        buffer[i] = sample;
        buffer[i+1] = sample;
    }

    // clean up done tones
    for (int channel = 0; channel <= highestActiveChannel; channel++)
    {
        if (currentTones[channel].isDone())
        {
            handleDoneTone(channel);
        }
    }
}

uint32_t ToneSheduler::getPlaceLeftInQueue()
{
    return placeLeftInQueue;
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