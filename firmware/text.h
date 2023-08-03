/**
 * @file text.h
 * @author Leon Farchau (leon2225)
 * @brief Class that represents a drawable text on the display
 * @version 0.1
 * @date 03.08.2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include "stdint.h"
#include "ili9488.h"
#include <string>

class Text{
    public:
        Text( uint16_t x, uint16_t y, std::string text, ili9488_rgb_t bgColor, ili9488_rgb_t textColor, ili9488_font_opt_t font, bool autoErase = true);
        ~Text();

        uint16_t getX(){ return this->x;}
        uint16_t getY(){ return this->y;}
        uint16_t getWidth(){ return this->width;}
        uint16_t getHeight(){ return this->height;}
        uint16_t getFont(){ return this->font;}
        ili9488_rgb_t getTextColor(){ return this->textColor;}
        ili9488_rgb_t getBgColor(){ return this->bgColor;}
        std::string getText(){ return this->text;}

        void setX( uint16_t x ){ this->x = x; updatePosition();}
        void setY( uint16_t y ){ this->y = y; updatePosition();}
        void setFont( ili9488_font_opt_t font ){ this->font = font; updateFontStyle();}
        void setBgColor( ili9488_rgb_t bgColor ){ this->bgColor = bgColor; updateFontStyle();}
        void setTextColor( ili9488_rgb_t textColor ){ this->textColor = textColor; updateFontStyle();}
        void setText( std::string text );

        void draw();
        void erase();

    private:
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t oldWidth;
        uint16_t height;
        uint16_t oldHeight;

        bool autoErase;

        ili9488_rgb_t bgColor;
        ili9488_rgb_t textColor;
        ili9488_font_opt_t font;

        std::string text;

        uint16_t calculateWidth();
        uint16_t calculateHeight();
        void updateFontStyle();
        void updatePosition();
};
