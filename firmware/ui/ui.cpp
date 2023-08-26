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
#include <map>
#include <span>

#include "../ili9488/ili9488.h"
#include "../xpt2046/xpt2046.h"

#include "rectangle.h"
#include "text.h"
#include "label.h"
#include "button.h"
#include "../ui_songs/ui_song.h"

#include "../ui_songs/songs/ui_song_interface.h"

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
const uint32_t DISPLAY_PERIOD = 1'000'000 / UPDATE_RATE_DISPLAY;
const uint32_t TOUCH_PERIOD = 1'000'000 / UPDATE_RATE_TOUCH;
const uint32_t TONES_PERIOD = 50'000;
const uint32_t TIME_UPDATE_PERIOD = 1'000'000;

const uint32_t TIME_HORIZON = 10'000'000; // 10 seconds
 
////////////////////////////////////////
// Typedefs
////////////////////////////////////////

////////////////////////////////////////
// Prototypes
////////////////////////////////////////
void ui_initHardware();
void ui_updateUI();
void ui_updateTouch();
void ui_handleOnPress(Point pos);
void ui_handleOnRelease(Point start, Point end);
void ui_buildUI();
void ui_buildMenu();
void ui_buildVolumeCtrl( const Point size, const Point pos, const uint16_t btnLeftBorder, const Button* btnTemplate, const ili9488_rgb_t borderColor);
void ui_updateVolumeSlider();
void ui_drawTones(std::span<ui_tone> tones);
void ui_drawSongMetadata(ui_song &song);

void ui_updateSong(std::string name );
void ui_updateTime(uint32_t deltaTime);
void ui_updateVisibleTones();

void ui_timeToString(uint32_t time, std::string* str);

////////////////////////////////////////
// Variables
////////////////////////////////////////
uint32_t g_nextTime_display = 0;
uint32_t g_nextTime_touch = 0;
uint32_t g_nextTime_tones = 0;
uint32_t g_nextTime_timeUpdate = 0;

std::vector<Button*> g_buttons;
Button *g_menuBtn;
Button *g_playBtn;
Button *g_pauseBtn;
Button *g_stopBtn;
Button *g_louderBtn;
Button *g_quieterBtn;
Text *g_currentTimeText;
std::map<uint32_t, Rectangle*> g_tones;

Rectangle* g_volumeSlider;
Rectangle *g_volumeGrayBar;
std::span<Rectangle*> g_volumeBars;

uint8_t g_volume = 50;
ui_song g_song; 
std::span<ui_tone> g_visibleTones;

uint32_t cppVersion = __cplusplus;

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

    ui_updateSong("test");
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
        ui_updateUI();
        g_nextTime_display = now + DISPLAY_PERIOD;
    }
    if( g_nextTime_tones <= now ) {
            ui_updateTime(TONES_PERIOD);
            ui_updateVisibleTones();
            ui_drawTones(g_visibleTones);
        
        g_nextTime_tones = now + TONES_PERIOD;
    }
    if( g_nextTime_timeUpdate <= now ) {
        std::string timeStr;
        ui_timeToString(g_song.getProgress(), &timeStr);
        g_currentTimeText->setText(timeStr);
        g_currentTimeText->draw();
        g_nextTime_timeUpdate = now + TIME_UPDATE_PERIOD;
    }
}

/**
 * @brief Updates the internal time of the song that is used for displaying
 * 
 * @param time 
 */
void ui_updateTime(uint32_t deltaTime)
{
    g_song.updateProgress(deltaTime);
}

/**
 * @brief Updates the tones that should be displayed
 * 
 * @param tones     Tones that should be displayed
 */
void ui_updateVisibleTones()
{
    g_song.getActiveTones(TIME_HORIZON, g_visibleTones);
}

/**
 * @brief Cyclic handler for display
 * 
 */
void ui_updateUI() {
    // draw buttons
    for(Button* btn : g_buttons) {
        btn->draw();
    }
}

/**
 * @brief Updates the the displayed information of the song
 * 
 * @param name      Name of the song
 */
void ui_updateSong(std::string name ){
    g_song.loadMetaData();
    std::span<ui_tone> tones;
    g_song.getTones(0,TIME_HORIZON,tones);

    // draw tones
    ui_drawSongMetadata(g_song);
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
    ili9488_rgb_t bgColor = ili9488_hex_to_rgb(0xDB4747); //Value changed to match actual color
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
    ili9488_rgb_t sliderColor = ili9488_hex_to_rgb(0x80C725);

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

void ui_drawTones(std::span<ui_tone> tones)
{
    Point startPos = Point(0, 20);
    Point size = Point(380, 280);

    const uint32_t timeFrameShown = 10'000'000; // 10 seconds

    ili9488_rgb_t noteColor = ili9488_hex_to_rgb(0xF08B14);

    ili9488_rgb_t channel1Color = ili9488_hex_to_rgb(0xF08B14);
    ili9488_rgb_t channel2Color = ili9488_hex_to_rgb(0x3141CF);
    uint32_t mapLength = g_tones.size();
    auto channelColors = std::vector<ili9488_rgb_t>{channel1Color, channel2Color};

    // delete old notes
    
    for (auto it = g_tones.cbegin(); it != g_tones.cend() /* not hoisted */; /* no increment */)
    {
        if(!tones.size() || it->first < tones[0].startTime)
        {
            it->second->erase(ILI9488_COLOR_WHITE);
            delete it->second;
            g_tones.erase(it++);
        }
        else
        {
            break;
        }
    }

    // draw notes
    for(auto tone: tones)
    {
        int32_t relStartTime = (int32_t)tone.startTime - (int32_t)g_song.getProgress();
        volatile uint32_t noteDuration = tone.duration;

        // handle notes that start before frame
        if(relStartTime < 0){
            noteDuration = tone.duration + relStartTime;

            // skip notes that end before frame
            if (((int32_t)tone.duration + relStartTime) < 0) continue;   
        }

        Point noteSize(size.x * noteDuration / timeFrameShown, 4);
        Point offset(size.x * MAX(relStartTime, 0) / timeFrameShown, size.y * tone.frequency / 128 - noteSize.y / 2);

        noteSize.x = MIN(noteSize.x, size.x - offset.x);
        noteSize.x = MAX(noteSize.x, 1);

        Rectangle* rect;
        if(!g_tones.contains(tone.startTime))
        {
            rect = new Rectangle(startPos + offset, noteSize, channelColors[(tone.channelIdx+1)%2]);
        }
        else
        {
            rect = g_tones[tone.startTime];

            // clear difference between old and new note
            auto StartPos = startPos + offset + noteSize.xPart();
            auto Size = rect->getPosition().xPart() + rect->getSize() - noteSize.xPart() - offset.xPart();
            Rectangle* eraseRect = new Rectangle(StartPos, Size, ILI9488_COLOR_WHITE);
            eraseRect->draw();
            delete eraseRect;
            rect->setSize(noteSize);
            rect->setPosition(startPos + offset);
        }

        rect->draw();
        

        g_tones[tone.startTime] = rect;
    }
    
}

void ui_drawSongMetadata(ui_song &song)
{
    Point namePos = Point(8, 4);
    Point timePos = Point(8, 300);
    Point channelsPos = Point(200, 300);

    ili9488_rgb_t textColor = ili9488_hex_to_rgb(0x000000);
    ili9488_rgb_t channel1Color = ili9488_hex_to_rgb(0xF08B14);
    ili9488_rgb_t channel2Color = ili9488_hex_to_rgb(0x3141CF);
    ili9488_font_opt_t fontTitle = eILI9488_FONT_24;
    ili9488_font_opt_t fontTime = eILI9488_FONT_16;
    ili9488_font_opt_t fontChannels = eILI9488_FONT_16;

    // draw song name
    Text* text = new Text(namePos, song.getName(), ILI9488_COLOR_WHITE, textColor, fontTitle);
    text->draw();
    delete text;

    // draw song time
    std::string timeStr;
    ui_timeToString(song.getDuration(), &timeStr);
    g_currentTimeText = new Text(timePos, "0:00", ILI9488_COLOR_WHITE, textColor, fontTime);
    g_currentTimeText->draw();
    text = new Text(timePos + g_currentTimeText->getSize().xPart(), " / " + timeStr, ILI9488_COLOR_WHITE, textColor, fontTime);
    text->draw();
    delete text;

    // draw channels
    Point channelOffset = Point(0, 0);
    std::vector<std::string> channels;
    song.getChannels(channels);
    std::string strings[] = {channels[3], "|", channels[4]};
    ili9488_rgb_t colors[] = {channel1Color, textColor, channel2Color};
    for(uint16_t i = 0; i < 3; i++) {
        text = new Text(channelsPos + channelOffset, strings[i], ILI9488_COLOR_WHITE, colors[i], fontChannels);
        text->draw();
        delete text;
        channelOffset += text->getSize().xPart();
    }
}

/**
 * @brief Converts a time in microseconds to a string
 * 
 * @param timeUs    Time in microseconds
 * @param str       Pointer to the string where the result should be stored
 */
void ui_timeToString(uint32_t timeUs, std::string* str)
{
    uint32_t timeS = timeUs / 1000000;
    char buffer[6]; sprintf(buffer, "%01d:%02d", timeS/60, timeS%60);
    *str = std::string(buffer);
}