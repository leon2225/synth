#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/divider.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/interp.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "stdfix.h"
#include "math.h"


#include "DAC.h"


// Defines
extern const uint DEBUG1_PIN = 11;
extern const uint DEBUG2_PIN = 15;
extern const uint DEBUG3_PIN = 12;

const uint AUDIO_LRCLK = 28;
const uint AUDIO_BCLK = 27;
const uint AUDIO_DOUT = 3;

const uint32_t SAMPLE_RATE = 48000;

// Global variables
DAC *dac;
static char strBuffer[100];

// Peripheral variables
struct repeating_timer timer;
interp_config cfg;


// private function prototypes
bool alarm_callback(struct repeating_timer *timer);
void setup();

int main()
{
    setup();
    uint32_t attack = INT32_MAX/(48000*0.1);
    uint32_t sustain = INT32_MAX * 0.1;
    uint32_t decay = (INT32_MAX-sustain)/(48000*0.2);
    uint32_t release = sustain/(48000*0.3);
    uint32_t duration = SAMPLE_RATE * 0.3;


    //frequency creation
    float pitches[] = {64,65,66,67,68,69,70,71};
    uint32_t frequencies[sizeof(pitches)/sizeof(pitches[0])];

    for (int i = 0; i < sizeof(pitches)/sizeof(pitches[0]); i++)
    {
        frequencies[i] = powf( 2, (pitches[i] - 69) / 12) * 440;
    }
    uint32_t oldChannel = 0;
    uint32_t currentChannel = 0;

    uint32_t startTime = 0;
    uint32_t endTime = 0;
    
    uint32_t toneDistance = 1000 * 150;

    uint32_t frequency = 0;
    uint32_t frequencyIndex = 0;
    int32_t frequencyDir = 1;

    while (1)
    {
        dac->cyclicHandler();
        
        if (time_us_32() - startTime > toneDistance)
        {
            startTime = time_us_32();


            frequency = frequencies[frequencyIndex];
            frequencyIndex += frequencyDir;
            if (frequencyIndex == (sizeof(pitches)/sizeof(pitches[0]) - 1) || frequencyIndex == 0)
            {
                frequencyDir *= -1;
            }
            sprintf(strBuffer, ">f: %d\n", frequency);
            uart_puts(uart0, strBuffer);

            oldChannel = currentChannel;
            currentChannel = dac->setTone(frequency, duration, attack, decay, sustain, release);
        }
        
    }
    
    return 0;
}

void setup()
{
    stdio_init_all();

    // ** I²S initialisation **
    dac = &DAC::getInstance();
    dac->setup(AUDIO_LRCLK, AUDIO_BCLK, AUDIO_DOUT);

    // ** GPIO initialisation **
    // We will make this GPIO an input, and pull it up by default
    gpio_init(DEBUG1_PIN);
    gpio_set_dir(DEBUG1_PIN, GPIO_OUT);
    gpio_init(DEBUG2_PIN);
    gpio_set_dir(DEBUG2_PIN, GPIO_OUT);
    gpio_init(DEBUG3_PIN);
    gpio_set_dir(DEBUG3_PIN, GPIO_OUT);

    // ** timer initialisation **
    add_repeating_timer_ms(1000, alarm_callback, NULL, &timer);
 
    // ** Interpolator initialisation **
    cfg = interp_default_config();
    // Now use the various interpolator library functions for your use case
    // e.g. interp_config_clamp(&cfg, true);
    //      interp_config_shift(&cfg, 2);
    // Then set the config 
    interp_set_config(interp0, 0, &cfg);

    // ** UART initialisation **
        // Initialise UART 0
    uart_init(uart0, 921600);
 
    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

}


// Timer callback function
bool alarm_callback(struct repeating_timer *timer) {
    return true;
}


