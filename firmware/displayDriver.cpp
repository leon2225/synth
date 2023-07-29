#include "displayDriver.h"

#include <stdio.h>
#include "pico/stdlib.h"

#include "ili9488_if.h"


extern const uint DEBUG1_PIN;
extern const uint DEBUG2_PIN;
extern const uint DEBUG3_PIN;
extern const uint DEBUG4_PIN;
 
void initDisplay();

void displayInit() {
    printf("Display init\n");
    initDisplay();

    ili9488_rect_attr_t rect_attr;
    rect_attr.position.x = 50;
    rect_attr.position.y  = 200;
    rect_attr.position.width  = 100;
    rect_attr.position.height   = 100;

    rect_attr.rounded.enable      = true;
    rect_attr.rounded.radius      = 9;

    rect_attr.fill.enable         = true;
    rect_attr.fill.color          = eILI9488_COLOR_PURPLE;

    rect_attr.border.enable       = true;
    rect_attr.border.width        = 2;
    rect_attr.border.color        = eILI9488_COLOR_BLACK;

    ili9488_draw_rectangle( &rect_attr );

    while (1) {
        tight_loop_contents();
    }
}

void initDisplay() {
    // init hardware
    ili9488_init();

    // set background to bg
    gpio_put(DEBUG3_PIN, 1);
    ili9488_set_background( eILI9488_COLOR_BLACK );
    gpio_put(DEBUG3_PIN, 0);
}