//***************************************************************************************
//* Handler for the DAC

//* Receives a value and outputs it via i2s to the DAC

//***************************************************************************************

#include "DAC.h"

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

#define OUTPUT_BUFFER_SIZE 2048

static char strBuffer[100];
volatile uint32_t outputBuffer[OUTPUT_BUFFER_SIZE] = {0};

void dma_handler();
inline int32_t sadd(int32_t a, int32_t b);


//***************************************************************************************
//* Starts a new Tone
//*
//* returns the channel number of the new tone, or -1 if no channel is available
//***************************************************************************************
int DAC::setTone(float frequency, uint32_t duration, uint32_t attack, uint32_t decay, uint32_t sustain, uint32_t release)
{
    //find a free channel
    for (int i = 0; i < CHANNEL_NUMBER; i++)
    {
        if (currentTones[i].isDone())
        {
            currentTones[i] = Tone(frequency, duration, attack, decay, sustain, release);

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

void DAC::release(uint8_t channel)
{
    currentTones[channel].stop();
}

bool DAC::isDone(uint8_t channel)
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

void DAC::setup(uint lrclkPin, uint bclkPin, uint doutPin)
{
    Tone::setupSine();

    // Set up the GPIO pins
    this->lrclkPin = lrclkPin;
    this->bclkPin = bclkPin;
    this->doutPin = doutPin;

    // ** PIO initialisation **
    pio = pio0;
    sm = pio_claim_unused_sm(pio, true);
    offset = pio_add_program(pio, &i2s_program);
    i2s_program_init(pio, sm, offset, lrclkPin, bclkPin, doutPin, 48000);

    pio_sm_set_enabled(pio, sm, true);

    // Set up the DMA
    DMASetup();
}

// Set up the DMA to feed the FIFO of PIO0_SM0.
// Two channels are used in a way that one channel is always active
// while the other one can be written to by the CPU. If the active burst is
// finished, the DMA switches to the other channel.
void DAC::DMASetup()
{
    // Set up the DMA channels
    dmaChannels[0] = dma_claim_unused_channel(true);
    dmaChannels[1] = dma_claim_unused_channel(true);

    // Set up the DMA control block
    dma_channel_config dmaConfig1 = dma_channel_get_default_config(dmaChannels[0]);
    channel_config_set_transfer_data_size(&dmaConfig1, DMA_SIZE_32);
    channel_config_set_read_increment(&dmaConfig1, true);
    channel_config_set_write_increment(&dmaConfig1, false);
    channel_config_set_dreq(&dmaConfig1, pio_get_dreq(pio, sm, true));
    channel_config_set_chain_to(&dmaConfig1, dmaChannels[1]);

    dma_channel_configure(dmaChannels[0], &dmaConfig1,
                          &pio->txf[sm],          // write address
                          outputBuffer,           // read address
                          OUTPUT_BUFFER_SIZE / 2, // element count
                          false                   // start immediately
    );

    dma_channel_config dmaConfig2 = dma_channel_get_default_config(dmaChannels[1]);
    channel_config_set_transfer_data_size(&dmaConfig2, DMA_SIZE_32);
    channel_config_set_read_increment(&dmaConfig2, true);
    channel_config_set_write_increment(&dmaConfig2, false);
    channel_config_set_dreq(&dmaConfig2, pio_get_dreq(pio, sm, true));
    channel_config_set_chain_to(&dmaConfig2, dmaChannels[0]);

    dma_channel_configure(dmaChannels[1], &dmaConfig2,
                          &pio->txf[sm],                         // write address
                          outputBuffer + OUTPUT_BUFFER_SIZE / 2, // read address
                          OUTPUT_BUFFER_SIZE / 2,                // element count
                          false                                  // start immediately
    );

    // Set up the DMA interrupt
    dma_channel_set_irq0_enabled(dmaChannels[0], true);
    dma_channel_set_irq0_enabled(dmaChannels[1], true);

    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Start the DMA
    dma_channel_start(dmaChannels[0]);
}

int32_t DAC::getChannelThatFired()
{
    return dma_channel_get_irq0_status(dmaChannels[0]) ? 0 : 1;
}


void DAC::interruptHandler()
{
    uint32_t completedChannel = getChannelThatFired();
    dma_channel_set_read_addr(completedChannel, outputBuffer + (completedChannel == 1 ? OUTPUT_BUFFER_SIZE / 2 : 0), 0);

    gpio_put(DEBUG3_PIN, dma_channel_is_busy(0));

    bufferToFill = completedChannel + 1;

    // Clear the interrupt request.
    dma_hw->ints0 = 1u << completedChannel;

}

void DAC::cyclicHandler()
{
    if (bufferToFill == 0)
    {
        return;
    }

    gpio_put(DEBUG2_PIN, 1);

    const size_t start = bufferToFill == 1 ? 0 : OUTPUT_BUFFER_SIZE / 2;
    const size_t end = bufferToFill == 1 ? OUTPUT_BUFFER_SIZE / 2 : OUTPUT_BUFFER_SIZE;

    uint32_t sample = 0;

    // Fill the buffer
    for (size_t i = start; i < end; i+=2)
    {
        sample = 0;
        for (size_t channel = 0; channel <= highestActiveChannel; channel++)
        {
            sample = sadd(sample, currentTones[channel].nextSample());
        }

        outputBuffer[i] = sample;
        outputBuffer[i+1] = sample;
    }
    gpio_put(DEBUG2_PIN, 0);

    bufferToFill = 0;
}

void dma_handler()
{
    DAC *dac = &DAC::getInstance();
    dac->interruptHandler();
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
