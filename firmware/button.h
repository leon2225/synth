/**
 * @file button.h
 * @author Leon Farchau (leon2225)
 * @brief Class that represents a drawable button on the display
 * @version 0.1
 * @date 04.08.2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include "stdint.h"
#include "ili9488.h"
#include "label.h"
#include "point.h"

class Button: public Label{
    public:
        Button( Point position, Point Size, std::string text, ili9488_rgb_t bgColor, ili9488_rgb_t activeBgColor, ili9488_rgb_t textColor, ili9488_font_opt_t font, LabelAlignment alignment = LabelAlignment::LEFT);
        ~Button();

        void setOnPress( void (*onPress)(Button* button) ){ this->onPress = onPress;}
        void setOnRelease( void (*onRelease)(Button* button) ){ this->onRelease = onRelease;}

        void handleOnPress();
        void handleOnRelease();

    protected:
        void (*onPress)(Button* button);
        void (*onRelease)(Button* button);

        ili9488_rgb_t activeBgColor;
        ili9488_rgb_t inactiveBgColor;
};