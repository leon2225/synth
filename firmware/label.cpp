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

Label::Label( uint16_t x, uint16_t y, uint16_t width, uint16_t height, std::string text, ili9488_rgb_t bgColor, ili9488_rgb_t textColor, ili9488_font_opt_t font, LabelAlignment alignment )
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;

    this->bgColor = bgColor;
    this->text = text;
    this->font = font;
    this->textColor = textColor;
    this->alignment = alignment;
    
    this->textObj = new Text( x, y, text, bgColor, textColor, font);
    this->bgObj = new Rectangle( x, y, width, height, bgColor);
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
    bgObj->setWidth( width );
    bgObj->setHeight( height );
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
    this->updateBg |= updateBg;
    bgObj->setX( x );
    bgObj->setY( y );
    uint16_t textMargin = (height / 2) - (textObj->getHeight() / 2);
    textObj->setY( textMargin + y);

    uint16_t newX = 0;
    switch (alignment)
    {
    case LabelAlignment::LEFT:
        newX = x + textMargin;
        break;
    
    case LabelAlignment::CENTER:
        newX = x + (width / 2) - (textObj->getWidth() / 2);
        break;

    case LabelAlignment::RIGHT:
        newX = x - textObj->getWidth() + width - textMargin;
        break;
    
    default:
        break;
    }

    // Erase the old background if the text is moved to the right
    if(newX > textObj->getX())
    {
        Rectangle rect = Rectangle( textObj->getX(), textObj->getY(), newX - textObj->getX(), textObj->getHeight(), bgColor);
        rect.draw();
    }
    textObj->setX( newX );
}

void Label::updateFontStyle()
{
    updateBg = !(bgObj->getColor() == bgColor);
    textObj->setFont( font );
    textObj->setBgColor( bgColor );
    bgObj->setColor( bgColor );
    textObj->setTextColor( textColor );
    updatePosition(false);
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

void Label::erase()
{
    bgObj->erase(bgColor);
    textObj->erase();
}

