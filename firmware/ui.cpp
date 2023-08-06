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
void ui_buildMenu();
void ui_buildVolumeCtrl( const Point size, const Point pos, const uint16_t btnLeftBorder, const Button* btnTemplate, const ili9488_rgb_t borderColor);
void ui_updateVolumeSlider();

////////////////////////////////////////
// Variables
////////////////////////////////////////
uint32_t g_nextTime_display = 0;
uint32_t g_nextTime_touch = 0;

std::vector<Button*> g_buttons;
Button *g_menuBtn;
Button *g_playBtn;
Button *g_pauseBtn;
Button *g_stopBtn;
Button *g_louderBtn;
Button *g_quieterBtn;

Rectangle* g_volumeSlider;
Rectangle *g_volumeGrayBar;

uint8_t g_volume = 50;

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
    // draw buttons
    for(Button* btn : g_buttons) {
        btn->draw();
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
 * @brief Builds the user interface
 * 
 */
void ui_buildUI() {
    ui_buildMenu();
}

/**
 * @brief Builds the menu
 * 
 */
void ui_buildMenu()
{
    // create buttons
    uint16_t width = 100;
    uint16_t height = 128;
    uint16_t btnNum = 4;
    uint16_t border = 1;
    uint16_t leftBorder = 2;
    Point startPos = Point(DISPLAY_WIDTH - width, 0);
    ili9488_rgb_t textColor = ILI9488_COLOR_WHITE;
    ili9488_rgb_t bgColor = ili9488_hex_to_rgb(0xCA5858);
    ili9488_rgb_t activeColor = ili9488_hex_to_rgb(0xE08F8F);
    ili9488_rgb_t borderColor = ili9488_hex_to_rgb(0x5B5B5B);
    std::string text[] = {"< Back", "Pause", "Play", "Stop"};
    ili9488_font_opt_t font = eILI9488_FONT_16;

    Point buttonSize = Point(width-leftBorder, (height/btnNum) - border);
    Point elementOffset = Point(0, height/btnNum);

    // create borders
    for(uint16_t i = 1; i < (btnNum + 1); i++) {
        Point borderPos = startPos + Point(leftBorder, i * elementOffset.y - border);
        Point borderSize = Point(width - leftBorder, border);
        Rectangle* rect = new Rectangle(borderPos, borderSize, borderColor);
        rect->draw();
        delete rect;
    }
    Rectangle* leftBorderRect = new Rectangle(startPos, Point(leftBorder, height), borderColor);
    leftBorderRect->draw();
    startPos += Point(leftBorder, 0);
    delete leftBorderRect;

    // create buttons
    for(uint16_t i = 0; i < btnNum; i++) {
        Button* btn = new Button(startPos, buttonSize, text[i], bgColor, activeColor, textColor, font, LabelAlignment::CENTER);

        startPos += elementOffset;
        g_buttons.push_back(btn);
    }

    // create volume control
    Point volumeCtrlSize = Point(100, DISPLAY_HEIGHT - height);
    ui_buildVolumeCtrl(volumeCtrlSize, startPos, leftBorder, g_buttons[btnNum-1], borderColor);

    // assign buttons
    g_menuBtn = g_buttons[0];
    g_pauseBtn = g_buttons[1];
    g_playBtn = g_buttons[2];
    g_stopBtn = g_buttons[3];
    g_louderBtn = g_buttons[4];
    g_quieterBtn = g_buttons[5];

    // set touch callbacks

    g_pauseBtn->setOnPress([](Button* btn){
        btn->activate();
        g_playBtn->deactivate();
    });

    g_playBtn->setOnPress([](Button* btn){
        g_pauseBtn->deactivate();
        btn->activate();
    });

    g_stopBtn->setOnPress([](Button* btn){
        g_pauseBtn->deactivate();
        g_playBtn->deactivate();
    });
}

/**
 * @brief Builds the volume control
 * 
 * @param size          Size of the volume control
 * @param pos           Position of the volume control
 * @param btnLeftBorder Left Border width of the buttons
 * @param btnTemplate   Pointer to a button which is used as template for the buttons
 * @param borderColor   Color of the border
 */
void ui_buildVolumeCtrl( const Point size, const Point pos, const uint16_t btnLeftBorder, const Button* btnTemplate, const ili9488_rgb_t borderColor)
{
    uint16_t border = 1;
    Point startPos = pos;
    uint16_t btnNum = 2;
    uint16_t volumeWidth = 38;
    uint16_t volumePadding = 10;
    ili9488_font_opt_t font = eILI9488_FONT_16;
    ili9488_rgb_t bgColor = ili9488_hex_to_rgb(0xD9D9D9);
    ili9488_rgb_t sliderBgColor = ili9488_hex_to_rgb(0x999999);
    ili9488_rgb_t sliderColor = ili9488_hex_to_rgb(0x83C52F);

    Point buttonSize = btnTemplate->getSize();
    std::vector<Point> borderPos = {pos+buttonSize.yPart(), pos + Point(0, size.y - buttonSize.y - border)};
    Point sliderSize = Point(volumeWidth, size.y - 2 * (buttonSize.y + border + volumePadding));
    Point sliderPos = startPos + Point((size.x-volumeWidth)/2, buttonSize.y + border + volumePadding);
    Point sliderBgSize = Point(volumeWidth, sliderSize.y * g_volume / 100);
    Point sliderBgPos = sliderPos;
    
    // draw background
    Rectangle* rect = new Rectangle(startPos - Point(btnLeftBorder, 0), size, bgColor);
    rect->draw();
    delete rect;

    // create borders
    for(uint16_t i = 0; i < borderPos.size(); i++) {
        Point borderSize = Point(buttonSize.x, border);
        Rectangle* rect = new Rectangle(borderPos[i], borderSize, borderColor);
        rect->draw();
        delete rect;
    }
    Rectangle* leftBorder = new Rectangle(startPos - Point(btnLeftBorder,0), Point(btnLeftBorder, size.y), borderColor);
    leftBorder->draw();
    delete leftBorder;

    // create buttons
    Button* louderBtn = new Button(*btnTemplate);
    louderBtn->setText("Louder");
    louderBtn->setPosition(startPos);
    louderBtn->draw();

    Button* quiterBtn = new Button(*btnTemplate);
    quiterBtn->setText("Quiter");
    quiterBtn->setPosition(startPos + Point(0, size.y - buttonSize.y));
    quiterBtn->draw();

    g_buttons.push_back(louderBtn);
    g_buttons.push_back(quiterBtn);

    // create volume Slider
    g_volumeSlider = new Rectangle(sliderPos, sliderSize, sliderColor);
    g_volumeSlider->draw();

    g_volumeGrayBar = new Rectangle(sliderBgPos, sliderBgSize, sliderBgColor);
    g_volumeGrayBar->draw();

    // set touch callbacks
    louderBtn->setOnPress([](Button* btn){
        if(g_volume <= 95) {
            g_volume += 5;
            ui_updateVolumeSlider();
        }
    });

    quiterBtn->setOnPress([](Button* btn){
        if(g_volume >= 5) {
            g_volume -= 5;
            ui_updateVolumeSlider();
        }
    });
}

void ui_updateVolumeSlider()
{
    // calculate new size
    Point newSize = g_volumeSlider->getSize(); // get full size
    newSize.y = (newSize.y * (100 - g_volume)) / 100;
    volatile int16_t diff = newSize.y - g_volumeGrayBar->getSize().y;

    // erase/color green bar if new size is reduced
    if(diff < 0) {
        Point erasePos = g_volumeGrayBar->getPosition() + newSize.yPart();
        Point eraseSize = g_volumeGrayBar->getSize() - newSize.yPart();
        Rectangle* eraseRect = new Rectangle(erasePos, eraseSize, g_volumeSlider->getColor());
        eraseRect->draw();
        delete eraseRect;
    }

    // draw new size
    g_volumeGrayBar->setSize(newSize);
    g_volumeGrayBar->draw();
}