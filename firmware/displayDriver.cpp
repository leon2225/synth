#include "displayDriver.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "math.h"

#include "ili9488.h"
#include "xpt2046.h"


extern const uint DEBUG1_PIN;
extern const uint DEBUG2_PIN;
extern const uint DEBUG3_PIN;
extern const uint DEBUG4_PIN;
 
void initDisplay();



uint32_t penRadius = 5;
uint32_t redrawDistance = 2;
uint32_t nextTime = 0;


void displayInit() {
    printf("Display init\n");
    initDisplay();

    ili9488_rect_attr_t rect_attr;
    rect_attr.position.x = 20;
    rect_attr.position.y  = 20;
    rect_attr.position.width  = penRadius * 2-1;
    rect_attr.position.height   = penRadius * 2-1;

    rect_attr.rounded.enable      = false;
    rect_attr.rounded.radius      = penRadius-1;

    rect_attr.border.enable       = false;

    rect_attr.fill.enable         = true;
    rect_attr.fill.color          =  ili9488_hex_to_rgb( 0xFF0000 );

    ili9488_rect_attr_t eraseRect = rect_attr;
    eraseRect.position.width += 1;
    eraseRect.position.height += 1;
    eraseRect.rounded.enable      = false;
    eraseRect.fill.color          =  ili9488_hex_to_rgb( 0xFFFFFF );

    ili9488_set_string_pen( ILI9488_COLOR_BLACK, ILI9488_COLOR_WHITE, eILI9488_FONT_24);
    ili9488_set_cursor( 20, 20 );

    int32_t x = 0;
    int32_t y = 0; 

    while (1) {
        tight_loop_contents();
        XPT2046_update();
        if( nextTime <= time_us_32() ) {
            XPT2046_TouchData_t touch = XPT2046_getTouch();
            if (touch.pressure) {
                eraseRect.position.x = x - penRadius;
                eraseRect.position.y = y - penRadius;
                ili9488_draw_rectangle( &eraseRect );
                
                if(abs(touch.x - x) > redrawDistance || abs(touch.y - y) > redrawDistance)
                {
                    x = touch.x;
                    y = touch.y;
                } 

                rect_attr.position.x = x - penRadius;
                rect_attr.position.y = y - penRadius;
                ili9488_draw_rectangle( &rect_attr );

                ili9488_printf("x: %4d, y: %4d", x, y);
            }
            nextTime = time_us_32() + 10000;
        }
    }
}

void initDisplay() {
    // init hardware
    ili9488_init();
    XPT2046_Init();

    // set background to bg
    ili9488_set_background( ILI9488_COLOR_WHITE );
}