/**
 * @file text.cpp 
 * @author Leon Farchau (leon2225)
 * @brief Class that represents a drawable text on the display
 * @version 0.1
 * @date 03.08.2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "text.h"
#include "ili9488_font.h"

Text::Text( Point position, std::string text, ili9488_rgb_t bgColor, ili9488_rgb_t textColor, ili9488_font_opt_t font, bool autoErase) {
    this->position = position;
    this->bgColor = bgColor;
    this->textColor = textColor;
    this->font = font;
    this->text = text;

    this->autoErase = true;

    updateFontStyle();
    updatePosition();
}

Text::~Text() {

}

/**
 * @brief Updates the text and optionally the erases the old text
 *          if the size of the text has decreased
 * 
 * @param text          The new text
 */
void Text::setText( std::string text) {
    this->text = text;
    if( this->autoErase ){
        oldSize = size;
    }
    size.x = calculateWidth();
    size.y = calculateHeight();
}

void Text::draw() {
    if( this->autoErase )
    {
        if( oldSize.y > size.y )
        {
            ili9488_rect_attr_t rect_attr;
            rect_attr.position.x = position.x;
            rect_attr.position.y = (position + size).y;
            rect_attr.position.width = oldSize.x;
            rect_attr.position.height = (oldSize- size).y;

            rect_attr.fill.enable = true;
            rect_attr.fill.color = this->bgColor;

            rect_attr.border.enable = false;
            rect_attr.rounded.enable = false;

            ili9488_draw_rectangle(&rect_attr);
        }
        if( oldSize.x > size.x )
        {
            ili9488_rect_attr_t rect_attr;
            rect_attr.position.x = (position + size).x;
            rect_attr.position.y = position.y;
            rect_attr.position.width = (oldSize- size).x;
            rect_attr.position.height = oldSize.y;

            rect_attr.fill.enable = true;
            rect_attr.fill.color = this->bgColor;

            rect_attr.border.enable = false;
            rect_attr.rounded.enable = false;

            ili9488_draw_rectangle(&rect_attr);
        }
    }

    ili9488_set_string(this->text.c_str(), position.x, position.y);
}

void Text::erase() {
    ili9488_rect_attr_t rect_attr;
    rect_attr.position.x = position.x;
    rect_attr.position.y = position.y;
    rect_attr.position.width = size.x;
    rect_attr.position.height = size.y;

    rect_attr.fill.enable = true;
    rect_attr.fill.color = this->bgColor;

    rect_attr.border.enable = false;
    rect_attr.rounded.enable = false;

    ili9488_draw_rectangle(&rect_attr);
}

// Private methods
uint16_t Text::calculateWidth() {
    return ili9488_font_get_width(this->font) * this->text.length();
}

uint16_t Text::calculateHeight() {
    return ili9488_font_get_height(this->font);
}

void Text::updateFontStyle() {
    this->font = font;
    if( this->autoErase ){
        oldSize = size;
    }
    size.x = calculateWidth();
    size.y = calculateHeight();
    ili9488_set_string_pen(this->textColor, this->bgColor, this->font);
}

void Text::updatePosition() {
    ili9488_set_cursor(position.x, position.y);
}