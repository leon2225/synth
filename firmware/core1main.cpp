#include "core1main.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "displayDriver.h"

#define FLAG_VALUE 123
 

void core1_entry() {
 
    multicore_fifo_push_blocking(FLAG_VALUE);
 
    uint32_t g = multicore_fifo_pop_blocking();
 
    if (g != FLAG_VALUE)
        printf("Hmm, that's not right on core 1!\n");
    else
        printf("Its all gone well on core 1!");
 
    displayInit();
    while (1)
        tight_loop_contents();
}