#pragma once

#include "pico/stdlib.h"
#include <stdio.h>
#include "DAC.h"

#define CHANNEL_NUMBER 10
#define QUEUE_LENGTH 100
#define SAMPLE_RATE 48000

struct AdsrProfile
{
    uint32_t attackRate;
    uint32_t decayRate;
    uint32_t sustainFactor;
    uint32_t releaseRate;

    AdsrProfile(float attack, float decay, float sustain, float release)
    {
        attackRate = __INT32_MAX__/(SAMPLE_RATE*attack);
        sustainFactor = __INT32_MAX__ * decay;
        decayRate = (__INT32_MAX__ - sustainFactor)/(SAMPLE_RATE*decay);
        releaseRate = sustainFactor/(__INT32_MAX__ * release);
    }
};

struct ToneJob
{
    Tone tone;
    uint32_t startTime;
};

class ToneSheduler {
    public:
        ToneSheduler();
        ~ToneSheduler();

        int addTone(float frequency, uint32_t startTime, float duration, AdsrProfile adsrProfile);
        void cyclicHandler();

    private:
        void fillBufferCallback(volatile uint32_t* buffer, uint32_t bufferLength);

        Tone currentTones[CHANNEL_NUMBER];
        int8_t highestActiveChannel = 0;

        ToneJob toneQueue[QUEUE_LENGTH];

        DAC *dac;
};