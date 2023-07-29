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
extern const uint DEBUG4_PIN;

#define OUTPUT_BUFFER_SIZE 2048

static char strBuffer[100];
volatile uint32_t outputBuffer[OUTPUT_BUFFER_SIZE] = {0};

void dma_handler();


void DAC::setup(uint32_t lrclkPin, uint32_t bclkPin, uint32_t doutPin, uint32_t sampleRate)
{
    // Set up the GPIO pins
    this->lrclkPin = lrclkPin;
    this->bclkPin = bclkPin;
    this->doutPin = doutPin;

    // ** PIO initialisation **
    pio = pio0;
    sm = pio_claim_unused_sm(pio, true);
    offset = pio_add_program(pio, &i2s_program);
    i2s_program_init(pio, sm, offset, lrclkPin, bclkPin, doutPin, sampleRate);

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

    bufferToFill = completedChannel + 1;

    // Clear the interrupt request.
    dma_hw->ints0 = 1u << completedChannel;

}

bool DAC::cyclicHandler(volatile uint32_t** buffer, uint32_t *bufferLength)
{
    if (bufferToFill == 0)
    {
        return 0;
    }

    const size_t start = bufferToFill == 1 ? 0 : (OUTPUT_BUFFER_SIZE >> 1);
    const size_t end = start + (OUTPUT_BUFFER_SIZE >> 1);

    *buffer = &outputBuffer[start];
    *bufferLength = OUTPUT_BUFFER_SIZE >> 1;


    bufferToFill = 0;
    return true;
}

void dma_handler()
{
    DAC *dac = &DAC::getInstance();
    dac->interruptHandler();
}
