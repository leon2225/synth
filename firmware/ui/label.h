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
#include "../ili9488/ili9488.h"
#include <string>

#include "rectangle.h"
#include "text.h"

#include "point.h"

enum class LabelAlignment{
    LEFT,
    CENTER,
    RIGHT
};

class Label {
    public:
        Label(){};
        Label( Point position, Point size, std::string text, ili9488_rgb_t color, ili9488_rgb_t textColor, ili9488_font_opt_t font, LabelAlignment alignment = LabelAlignment::LEFT);
        ~Label();
        Label( const Label& other ); // copy constructor

        Point getPosition() const { return position;}
        Point getSize() const { return size;}
        uint16_t getFont() const { return this->font;}
        ili9488_rgb_t getTextColor() const { return this->textColor;}
        ili9488_rgb_t getBgColor() const { return this->color;}
        std::string getText() const { return this->text;}
        bool contains( Point p ) const { return bgObj->contains(p);}

        void setPosition( Point position ){ this->position = position; updatePosition();}
        void setSize( Point size ){ this->size = size; updateSize();}
        void setFont( ili9488_font_opt_t font ){ this->font = font; updateFontStyle();}
        void setBgColor( ili9488_rgb_t color ){ this->color = color; updateFontStyle(); updateBg = true;}
        void setTextColor( ili9488_rgb_t textColor ){ this->textColor = textColor; updateFontStyle();}
        void setText( std::string text );

        void draw();
        void erase(ili9488_rgb_t color);

    protected:
        Point position;
        Point size;

        ili9488_rgb_t color;
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