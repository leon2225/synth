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
#include <string>
#include <vector>

#include "ili9488.h"
#include "xpt2046.h"

#include "rectangle.h"
#include "text.h"
#include "label.h"
#include "button.h"

////////////////////////////////////////
// Defines
////////////////////////////////////////
extern const uint DEBUG1_PIN;
extern const uint DEBUG2_PIN;
extern const uint DEBUG3_PIN;
extern const uint DEBUG4_PIN;

#define UPDATE_RATE_DISPLAY (30)
#define UPDATE_RATE_TOUCH   (30)

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

std::vector<Button*> g_buttons;


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
}

/**
 * @brief Tasksheduler for ui
 * 
 */
void ui_loop() {
    uint32_t now = time_us_32();
    if( g_nextTime_touch <= now ) {
        ui_updateTouch();
        g_nextTime_touch = now + TOUCH_PERIOD;
    }
    if( g_nextTime_display <= now ) {
        ui_updateDisplay();
        g_nextTime_display = now + DISPLAY_PERIOD;
    }
}

/**
 * @brief Cyclic handler for display
 * 
 */
void ui_updateDisplay() {
    static Point pointerPos = Point(15, 15);
    static bool pressed = false;

    static Label label = Label(Point(100, 100), Point(250, 50), "Test", ILI9488_COLOR_YELLOW, ILI9488_COLOR_BLACK, eILI9488_FONT_24, LabelAlignment::CENTER);

    const int redrawDistance = 2;

    XPT2046_TouchData_t touch = XPT2046_getTouch();

    if (touch.pressure) {
        if(!pressed)
        {
            pressed = true;
            label.setBgColor(ILI9488_COLOR_RED);
        }

        Point diff = touch.position - pointerPos;

        if((abs(diff.x) > redrawDistance) || (abs(diff.y) > redrawDistance))
        {
            pointerPos = touch.position;

            gpio_put(DEBUG1_PIN, 1);

            std::string str = "X:" + std::to_string(pointerPos.x) + ", Y:" + std::to_string(pointerPos.y);
            label.setText(str);
            

            label.draw();

            gpio_put(DEBUG1_PIN, 0);
        } 
    }
    else
    {
        if(pressed)
        {
            pressed = false;
            label.setBgColor(ILI9488_COLOR_YELLOW);
            label.draw();
        }
    }   
}

/**
 * @brief Cyclic handler for touch
 * 
 */
void ui_updateTouch() {
    static bool pressed = false;
    XPT2046_update();
    XPT2046_TouchData_t touch = XPT2046_getTouch();
    if(touch.pressure) {
        if(!pressed) {
            pressed = true;
            gpio_xor_mask(1 << DEBUG2_PIN);
        }
    }
    else {
        if(pressed) {
            pressed = false;
            gpio_xor_mask(1 << DEBUG2_PIN);
        }
    }
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

/**
 * @brief Handler for touch events
 * 
 */
void touchHandler(uint16_t x, uint16_t y) {
    XPT2046_update();
    XPT2046_TouchData_t touch = XPT2046_getTouch();
    if(touch.pressure) {
        gpio_xor_mask(1 << DEBUG2_PIN);
    }
}