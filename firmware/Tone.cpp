#include "Tone.h"

#include "math.h"

#define LUT_SIZE 256

extern const uint DEBUG1_PIN;
extern const uint DEBUG2_PIN;
extern const uint DEBUG3_PIN;



typedef enum
{
    ADSR_RISING = 0,
    ADSR_FALLING,
    ADSR_SUSTAIN,
    ADSR_RELEASED,
    ADSR_DONE
} ADSRState_t;

// global variables
static int sineLUT[LUT_SIZE];
static char strBuffer[100];

void Tone::setupSine()
{
    const int32_t INT24_MAX = 0x7FFFFF;
    // Create a sine lookup table
    for (size_t i = 0; i < LUT_SIZE; i++)
    {
        sineLUT[i] = (int)(INT24_MAX * sinf((float)i * 2 * M_PI / (float)LUT_SIZE) * 0.15f);

        //add harmonics
        const uint32_t NUM_HARMONICS = 5;
        
        // for (size_t harmonic = 2; harmonic <= NUM_HARMONICS; harmonic++)
        // {

        //     sineLUT[i] += (int)(INT24_MAX * sinf((float)i * (2*harmonic) * M_PI / (float)LUT_SIZE)/ (float)(1<<harmonic));
        // }
        
        // sineLUT[i] += (int)(INT24_MAX * sinf((float)i * 4 * M_PI / (float)LUT_SIZE) * 0.125f/4);
        // sineLUT[i] += (int)(INT24_MAX * sinf((float)i * 8 * M_PI / (float)LUT_SIZE) * 0.125f/4);
        // sineLUT[i] += (int)(INT24_MAX * sinf((float)i * 16 * M_PI / (float)LUT_SIZE) * 0.125f/4);

    }
}

Tone::Tone()
{
    this->ADSRState = ADSR_DONE;
}

Tone::Tone(float frequency, uint32_t duration, uint32_t attack, uint32_t decay, uint32_t sustain, uint32_t release)
{
    this->stepSize = (uint32_t)((float)(frequency * (LUT_SIZE<<6))/ 48000.0f );

    this->attackStepSize = attack;
    this->decayStepSize = decay;
    this->sustainLevel = sustain;
    this->releaseStepSize = release;

    cyclesLeft = duration;

}

Tone::~Tone()
{
}

void Tone::stop()
{
    this->ADSRState = ADSR_RELEASED;
}

uint32_t Tone::nextSample()
{
    this->accumulator += this->stepSize;
    uint8_t index = this->accumulator >> 6;
    return sineLUT[index] * this->nextADSRVolume();
}

bool Tone::isDone()
{
    return this->ADSRState == ADSR_DONE;
}

uint32_t Tone::nextADSRVolume()
{
    uint32_t volume = this->ADSRVolume >> 23;
    cyclesLeft--;

    switch (this->ADSRState)
    {
    case ADSR_RISING:
        this->ADSRVolume += this->attackStepSize;
        if (this->ADSRVolume >= INT32_MAX)
        {
            this->ADSRVolume = INT32_MAX;
            this->ADSRState = ADSR_FALLING;
        }

        break;
    case ADSR_FALLING:
        this->ADSRVolume -= this->decayStepSize;
        if (this->ADSRVolume <= this->sustainLevel || this->ADSRVolume >= ((uint32_t)INT32_MAX+1))
        {
            this->ADSRVolume = this->sustainLevel;
            this->ADSRState = (this->ADSRVolume >= ((uint32_t) INT32_MAX+1) || this->ADSRVolume == 0) ? ADSR_DONE : ADSR_SUSTAIN; //if sustain level is 0, we are done
        }
        break;
    case ADSR_SUSTAIN:
        if (cyclesLeft <= 0)
        {
            this->ADSRState = ADSR_RELEASED;
        }
        break;
    case ADSR_RELEASED:
        
        this->ADSRVolume -= this->releaseStepSize;
        if (this->ADSRVolume >= ((uint32_t) INT32_MAX+1) || this->ADSRVolume == 0) //overflow occured
        {
            this->ADSRVolume = 0;
            this->ADSRState = ADSR_DONE;
        }
        break;
    case ADSR_DONE:
        this->ADSRVolume = 0;
        break;
    default:
        break;
    }

#if 0
    //debug
    if (oldState != this->ADSRState)
    {
        //printf("ADSR state changed from %d to %d\n", oldState, this->ADSRState);
        //toggle debug pin
        gpio_xor_mask(1<<DEBUG1_PIN);
        oldState = this->ADSRState;
        sprintf(strBuffer, ">V: %d\n", this->ADSRVolume);
        uart_puts(uart0, strBuffer);

        sprintf(strBuffer, ">S: %d\n", this->ADSRState);
        uart_puts(uart0, strBuffer);
    }
#endif

    return volume;
}