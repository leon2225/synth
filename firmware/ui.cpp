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
void ui_initHardware();
void ui_updateDisplay();
void ui_updateTouch();
void ui_handleOnPress(Point pos);
void ui_handleOnRelease(Point start, Point end);
void ui_buildUI();
void handleButtonPress(Button* button);
void handleButtonRelease(Button* button);

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
    ui_initHardware();

    // set background to bg
    ili9488_set_background( ILI9488_COLOR_WHITE );

    // create Layout
    ui_buildUI();
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
    static Point firstPos = {0,0};
    XPT2046_update();
    XPT2046_TouchData_t touch = XPT2046_getTouch();
    if(touch.pressure) {
        if(!pressed) {
            pressed = true;
            ui_handleOnPress(touch.position);
            firstPos = touch.position;
        }
    }
    else {
        if(pressed) {
            pressed = false;
            ui_handleOnRelease(firstPos, touch.position);
        }
    }
}

/**
 * @brief Initialize display hardware and touch controller
 * 
 */
void ui_initHardware() {
    // init hardware
    ili9488_init();
    XPT2046_Init();
}

/**
 * @brief Handler for press events
 * 
 * @param pos   Position of the pointer on press
 */
void ui_handleOnPress(Point pos) {
    gpio_xor_mask(1 << DEBUG2_PIN);
    for(Button* btn : g_buttons) {
        if(btn->contains(pos)) {
            btn->handleOnPress();
        }
    }
}

/**
 * @brief Handler for release events
 * 
 * @param pos   Last position of the pointer
 */
void ui_handleOnRelease(Point startPos, Point endPos) {
    gpio_xor_mask(1 << DEBUG2_PIN);
    for(Button* btn : g_buttons) {
        if(btn->contains(startPos) && btn->contains(endPos)) {
            btn->handleOnRelease();
        }
    }
}


/**
 * @brief Handler for button presses
 * 
 * @param button 
 */
void handleButtonPress(Button* button){
    gpio_xor_mask(1 << DEBUG1_PIN);
    button->draw();

    printf("Button %s pressed\n", button->getText().c_str());
}

/**
 * @brief Handler for button releases
 * 
 * @param button 
 */
void handleButtonRelease(Button* button){
    gpio_xor_mask(1 << DEBUG1_PIN);
    button->draw();

    printf("Button %s released\n", button->getText().c_str());
}