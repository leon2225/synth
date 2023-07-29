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
#include "pico/multicore.h"
#include "stdfix.h"
#include "math.h"

#include "core1main.h"
#include "DAC.h"
#include "ToneSheduler.h"

// Defines
extern const uint DEBUG1_PIN = 19;
extern const uint DEBUG2_PIN = 20;
extern const uint DEBUG3_PIN = 21;
extern const uint DEBUG4_PIN = 22;

#define USE_DEBUG_PINS 0
#define FLAG_VALUE 123

const uint AUDIO_LRCLK = 28;
const uint AUDIO_BCLK = 27;
const uint AUDIO_DOUT = 3;

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

    ToneSheduler toneSheduler;

    //start core 1
    multicore_launch_core1(core1_entry);
    uint32_t g = multicore_fifo_pop_blocking(); //wait for core 1 to be ready
    multicore_fifo_push_blocking(FLAG_VALUE); //tell core 1 that core 0 is ready

    AdsrProfile defaultProfile = AdsrProfile(0.1, 0.1, 0.2, 0.3);

    float duration = 0.3;

    //frequency creation
    float pitches[] = {67.0,55.0,43.0,67.0,55.0,43.0,67.0,55.0,43.0,63.0,51.0,39.0,65.0,53.0,41.0,65.0,53.0,41.0,65.0,53.0,41.0,62.0,50.0,38.0};
    const uint32_t PITCH_NUMBER = sizeof(pitches)/sizeof(pitches[0]);
    float times[PITCH_NUMBER] = {0.25,0.25,0.25,0.5,0.5,0.5,0.75,0.75,0.75,1.0,1.0,1.0,2.25,2.25,2.25,2.5,2.5,2.5,2.75,2.75,2.75,3.0,3.0,3.0};
    float durations[PITCH_NUMBER] = {0.23,0.23,0.23,0.23,0.23,0.23,0.23,0.23,0.23,0.97,0.97,0.97,0.23,0.23,0.23,0.23,0.23,0.23,0.23,0.23,0.23,1.97,1.97,1.97};
    uint32_t frequencies[PITCH_NUMBER];


    while (1)
    {
        if(!toneSheduler.busy())
        {
            //toggle debug2 pin
            gpio_xor_mask(USE_DEBUG_PINS << DEBUG2_PIN);
            for (int i = 0; i < PITCH_NUMBER; i+=1)
            {
                frequencies[i] = powf( 2, (pitches[i] - 69) / 12) * 440;

                toneSheduler.addToneRel(frequencies[i], times[i], duration, defaultProfile);
            }
            //toggle debug2 pin
            gpio_xor_mask(USE_DEBUG_PINS << DEBUG2_PIN);
        }

        toneSheduler.cyclicHandler();
    }
    
    return 0;
}

void setup()
{
    stdio_init_all();

    // ** I²S initialisation **
    dac = &DAC::getInstance();
    dac->setup(AUDIO_LRCLK, AUDIO_BCLK, AUDIO_DOUT, SAMPLE_RATE);

    // ** GPIO initialisation **
    // We will make this GPIO an input, and pull it up by default
    gpio_init(DEBUG1_PIN);
    gpio_set_dir(DEBUG1_PIN, GPIO_OUT);
    gpio_init(DEBUG2_PIN);
    gpio_set_dir(DEBUG2_PIN, GPIO_OUT);
    gpio_init(DEBUG3_PIN);
    gpio_set_dir(DEBUG3_PIN, GPIO_OUT);
    gpio_init(DEBUG4_PIN);
    gpio_set_dir(DEBUG4_PIN, GPIO_OUT);

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


