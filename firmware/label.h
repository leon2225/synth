/**
 * @file label.h
 * @author Leon Farchau (leon2225)
 * @brief 
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

#include "rectangle.h"
#include "text.h"

enum class LabelAlignment{
    LEFT,
    CENTER,
    RIGHT
};

class Label {
    public:
        Label( uint16_t x, uint16_t y, uint16_t width, uint16_t height, std::string text, ili9488_rgb_t bgColor, ili9488_rgb_t textColor, ili9488_font_opt_t font, LabelAlignment alignment = LabelAlignment::LEFT);
        ~Label();

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
        void setWidth( uint16_t width ){ this->width = width; updateSize();}
        void setHeight( uint16_t height ){ this->height = height; updateSize();}
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
        uint16_t height;

        ili9488_rgb_t bgColor;
        ili9488_rgb_t textColor;
        ili9488_font_opt_t font;
        LabelAlignment alignment;

        std::string text;

        Rectangle *bgObj;
        Text *textObj;

        bool updateBg = true;
        bool updateText = true;

        uint16_t calculateWidth();
        uint16_t calculateHeight();
        void updateFontStyle();
        void updatePosition(bool updateBg = true);
        void updateSize();
};