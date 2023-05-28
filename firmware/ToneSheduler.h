#pragma once

#include "pico/stdlib.h"
#include <stdio.h>
#include <queue>
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
        releaseRate = sustainFactor/(SAMPLE_RATE * release);
    }
};

struct ToneJob
{
    Tone tone;
    uint32_t startTime;

    ToneJob(Tone tone, uint32_t startTime)
    {
        this->tone = tone;
        this->startTime = startTime;
    }
};

class ToneSheduler {
    public:
        ToneSheduler();
        ~ToneSheduler();

        int addToneAbs(float frequency, float startTime_sec, float duration, AdsrProfile adsrProfile);
        int addToneRel(float frequency, float startOffset_sec, float duration, AdsrProfile adsrProfile);
        void cyclicHandler();
        bool busy();

    private:
        void fillBufferCallback(volatile uint32_t* buffer, uint32_t bufferLength);
        bool startTone(Tone tone);
        void handleDoneTone(uint8_t channel);
        int addToneRaw(float frequency, uint32_t startTime_sam, float duration, AdsrProfile adsrProfile);

        Tone currentTones[CHANNEL_NUMBER];
        int8_t highestActiveChannel = -1;

        std::queue<ToneJob> jobQueue;

        uint32_t currentTime = 0;

        DAC *dac;
};