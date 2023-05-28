#pragma once

#include "pico/stdlib.h"
#include <stdio.h>


class Tone
{
    public:
        static void setupSine();

        Tone();
        Tone(float frequency, uint32_t duration, uint32_t attack, uint32_t decay, uint32_t sustain, uint32_t release);
        ~Tone();
        void stop();
        bool isDone();
        uint32_t nextSample(); // should be called at 48kHz
    private:
        //debug
        uint32_t oldState = 0;

        uint32_t stepSize;
        volatile uint32_t accumulator;

        volatile uint32_t ADSRVolume = 0;
        volatile uint32_t ADSRState = 0;

        volatile int32_t cyclesLeft = 0;

        uint32_t attackStepSize;
        uint32_t decayStepSize;
        uint32_t sustainLevel;
        uint32_t releaseStepSize;

        uint32_t nextADSRVolume();

};