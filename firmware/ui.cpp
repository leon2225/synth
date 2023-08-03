/**
 * @file ui.cpp
 * @author Leon Farchau (leon2225)
 * @brief Module that handles the user interface
 * @version 0.1
 * @date 03.08.2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */

////////////////////////////////////////
// Includes
////////////////////////////////////////
#include "ui.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "math.h"

#include "ili9488.h"
#include "xpt2046.h"

#include "rectangle.h"

////////////////////////////////////////
// Defines
////////////////////////////////////////
extern const uint DEBUG1_PIN;
extern const uint DEBUG2_PIN;
extern const uint DEBUG3_PIN;
extern const uint DEBUG4_PIN;

#define UPDATE_RATE_DISPLAY (120)
#define UPDATE_RATE_TOUCH   (120)

//periods for display and touch taks in us
const uint32_t DISPLAY_PERIOD = 1000000 / UPDATE_RATE_DISPLAY;
const uint32_t TOUCH_PERIOD = 1000000 / UPDATE_RATE_TOUCH;
 
////////////////////////////////////////
// Typedefs
////////////////////////////////////////

////////////////////////////////////////
// Prototypes
////////////////////////////////////////
void initHardware();
void ui_updateDisplay();
void ui_updateTouch();

////////////////////////////////////////
// Variables
////////////////////////////////////////
uint32_t g_nextTime_display = 0;
uint32_t g_nextTime_touch = 0;

////////////////////////////////////////
// Functions
////////////////////////////////////////

/**
 * @brief Sets up the user interface
 * 
 */
void ui_setup() {
    initHardware();

    // set background to bg
    ili9488_set_background( ILI9488_COLOR_WHITE );

    // set pen colors
    ili9488_set_string_pen( ILI9488_COLOR_GRAY, ILI9488_COLOR_YELLOW, eILI9488_FONT_24);
    ili9488_set_cursor( 20, 20 );
}

/**
 * @brief Tasksheduler for ui
 * 
 */
void ui_loop() {
    uint32_t now = time_us_32();
    if( g_nextTime_display <= now ) {
        ui_updateDisplay();
        g_nextTime_display = now + DISPLAY_PERIOD;
    }

    if( g_nextTime_touch <= now ) {
        ui_updateTouch();
        g_nextTime_touch = now + TOUCH_PERIOD;
    }
}

/**
 * @brief Cyclic handler for display
 * 
 */
void ui_updateDisplay() {
    static Rectangle rect = Rectangle(0, 0, 10, 10, ILI9488_COLOR_RED);
    const int redrawDistance = 2;

    XPT2046_TouchData_t touch = XPT2046_getTouch();

    if (touch.pressure) {
        
        if((abs(touch.x - rect.getX()) > redrawDistance) || (abs(touch.y - rect.getY()) > redrawDistance))
        {
            rect.erase(ILI9488_COLOR_WHITE);

            rect.setX(touch.x);
            rect.setY(touch.y);

            rect.draw();

            //ili9488_printf("x: %4d, y: %4d", x, y);
        } 
    }
}

/**
 * @brief Cyclic handler for touch
 * 
 */
void ui_updateTouch() {
    XPT2046_update();
}

/**
 * @brief Initialize display hardware and touch controller
 * 
 */
void initHardware() {
    // init hardware
    ili9488_init();
    XPT2046_Init();
}