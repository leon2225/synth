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

#include "ili9488.h"
#include "xpt2046.h"

#include "rectangle.h"
#include "text.h"
#include "label.h"

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
    const uint32_t crossSize = 20;

    static uint16_t x = 15, y = 15;

    static Rectangle verticalLine = Rectangle(x-1, y-10, 2, crossSize, ILI9488_COLOR_RED);
    static Rectangle horizontalLine = Rectangle(x-10, y-1, crossSize, 2, ILI9488_COLOR_RED);
    static Text text = Text(0, 0, "", ILI9488_COLOR_WHITE, ILI9488_COLOR_BLACK, eILI9488_FONT_24);
    static Label label = Label(100, 100, 250, 50, "Test", ILI9488_COLOR_YELLOW, ILI9488_COLOR_BLACK, eILI9488_FONT_24, LabelAlignment::CENTER);

    const int redrawDistance = 2;

    XPT2046_TouchData_t touch = XPT2046_getTouch();

    if (touch.pressure) {
        if((abs(touch.x - x) > redrawDistance) || (abs(touch.y - x) > redrawDistance))
        {
            x = touch.x;
            y = touch.y;

            gpio_put(DEBUG1_PIN, 1);
            verticalLine.erase(ILI9488_COLOR_WHITE);
            horizontalLine.erase(ILI9488_COLOR_WHITE);

            verticalLine.setX(x-1);
            verticalLine.setY(y -crossSize/2);
            horizontalLine.setX(x -crossSize/2);
            horizontalLine.setY(y-1);
            std::string str = "X:" + std::to_string(touch.x) + ", Y:" + std::to_string(touch.y);
            text.setText(str);
            label.setText(str);
            

            //text.draw();
            verticalLine.draw();
            horizontalLine.draw();
            label.draw();

            gpio_put(DEBUG1_PIN, 0);
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