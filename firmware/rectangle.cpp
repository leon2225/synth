/**
 * @file rectangle.cpp
 * @author Leon Farchau (leon2225)
 * @brief 
 * @version 0.1
 * @date 03.08.2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "rectangle.h"

Rectangle::Rectangle( Point position, Point size, ili9488_rgb_t color ) {
    this->position = position;
    this->size = size;

    this->color = color;
}

Rectangle::~Rectangle() {

}

bool Rectangle::contains( Point p ) {

    return  (position.x <= p.x) && (p.x <= (position+size).x) && 
            (position.y <= p.y) && (p.y <= (position+size).y);
}

void Rectangle::draw() {
    erase(this->color);
}

void Rectangle::erase(ili9488_rgb_t backgroundColor) {
    ili9488_rect_attr_t rect_attr;
    rect_attr.position.x = position.x;
    rect_attr.position.y = position.y;
    rect_attr.position.width = size.x;
    rect_attr.position.height = size.y;

    rect_attr.fill.enable = true;
    rect_attr.fill.color = backgroundColor;

    rect_attr.border.enable = false;
    rect_attr.rounded.enable = false;

    ili9488_draw_rectangle(&rect_attr);
}

