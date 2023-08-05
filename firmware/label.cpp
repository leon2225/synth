/**
 * @file label.cpp
 * @author Leon Farchau (leon2225)
 * @brief 
 * @version 0.1
 * @date 03.08.2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */

////////////////////////////////////////
// Includes
////////////////////////////////////////
#include "label.h"
#include "pico/stdlib.h"

////////////////////////////////////////
// Defines
////////////////////////////////////////
extern const uint DEBUG1_PIN;
extern const uint DEBUG2_PIN;
extern const uint DEBUG3_PIN;
extern const uint DEBUG4_PIN;

Label::Label( Point position, Point size, std::string text, ili9488_rgb_t color, ili9488_rgb_t textColor, ili9488_font_opt_t font, LabelAlignment alignment )
{
    this->position = position;
    this->size = size;

    this->color = color;
    this->text = text;
    this->font = font;
    this->textColor = textColor;
    this->alignment = alignment;
    
    this->textObj = new Text( position, text, color, textColor, font);
    this->bgObj = new Rectangle( position, size, color);
    updateSize();
    updateFontStyle();
}

Label::~Label()
{
    delete textObj;
    delete bgObj;
}

void Label::setText( std::string text )
{
    this->text = text;
    textObj->setText( text );
    updatePosition(false);
    updateText = true;
}

void Label::updateSize()
{
    bgObj->setSize( size );
    updatePosition();
    updateBg = updateText = true;
}

/**
 * @brief Updates the position of the label and the text and does the alignment
 * 
 * @param updateBg  If true, the background will be updated with the next call to draw()
 */
void Label::updatePosition(bool updateBg)
{
    Point offset;
    this->updateBg |= updateBg;
    bgObj->setPosition( position );
    uint16_t textMargin = (size.y / 2) - (textObj->getSize().y / 2);
    offset.y = textMargin;

    switch (alignment)
    {
    case LabelAlignment::LEFT:
        offset.x = textMargin;
        break;
    
    case LabelAlignment::CENTER:
        offset.x = (size.x / 2) - (textObj->getSize().x / 2);
        break;

    case LabelAlignment::RIGHT:
        offset.x = textObj->getSize().x + size.x - textMargin;
        break;
    
    default:
        break;
    }

    // Erase the old background if the text is moved to the right
    if((position + offset).x > textObj->getPosition().x)
    {
        Point eraseSize = Point( (position + offset).x - textObj->getPosition().x, textObj->getSize().y);
        Rectangle rect = Rectangle( textObj->getPosition(), eraseSize, color);
        rect.draw();
    }
    textObj->setPosition( position + offset );
}

void Label::updateFontStyle()
{
    updateBg = !(bgObj->getColor() == color);
    textObj->setFont( font );
    textObj->setBgColor( color );
    bgObj->setColor( color );
    textObj->setTextColor( textColor );
    updateSize();
}

void Label::draw()
{
    if(updateBg){
        updateBg = false;
        bgObj->draw();
        updateText = true; // Text has to be updated if the background is updated
    }
    if(updateText){
        updateText = false;
        textObj->draw();
    }
}

void Label::erase(ili9488_rgb_t color)
{
    bgObj->erase(color);
    textObj->setBgColor( color );
    textObj->erase();
}

