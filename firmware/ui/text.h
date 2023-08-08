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
#include "../ili9488/ili9488.h"
#include <string>
#include "point.h"

class Text{
    public:
        Text( Point position, std::string text, ili9488_rgb_t bgColor, ili9488_rgb_t textColor, ili9488_font_opt_t font, bool autoErase = true);
        ~Text();

        Point getPosition() const { return this->position;}
        Point getSize() const { return this->size;}
        uint16_t getFont() const { return this->font;}
        ili9488_rgb_t getTextColor() const { return this->textColor;}
        ili9488_rgb_t getBgColor() const { return this->bgColor;}
        std::string getText() const { return this->text;}

        void setPosition( Point position ){ this->position = position; updatePosition();}
        void setFont( ili9488_font_opt_t font ){ this->font = font; updateFontStyle();}
        void setBgColor( ili9488_rgb_t bgColor ){ this->bgColor = bgColor; updateFontStyle();}
        void setTextColor( ili9488_rgb_t textColor ){ this->textColor = textColor; updateFontStyle();}
        void setText( std::string text );

        void draw();
        void erase();

    private:
        Point position;
        Point size;
        Point oldSize;

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
