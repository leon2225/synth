/**
 * @file button.cpp
 * @author Leon Farchau (leon2225)
 * @brief Class that represents a drawable button on the display
 * @version 0.1
 * @date 04.08.2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "button.h"

Button::Button( Point position, Point size, std::string text, ili9488_rgb_t bgColor, ili9488_rgb_t activeBgColor, ili9488_rgb_t textColor, ili9488_font_opt_t font, LabelAlignment alignment ): Label(position, size, text, bgColor, textColor, font, alignment) {
    this->onPress = nullptr;
    this->onRelease = nullptr;
    this->activeBgColor = activeBgColor;
    this->inactiveBgColor = bgColor;
}

Button::~Button() {

}

/**
 * @brief Handles the press event of the button
 * 
 */
void Button::handleOnPress() {
    if( this->onPress != nullptr ){
        this->onPress(this);
    }
}

/**
 * @brief Handles the release event of the button
 * 
 */ 
void Button::handleOnRelease() {
    if( this->onRelease != nullptr ){
        this->onRelease(this);
    }  
}