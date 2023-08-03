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

Rectangle::Rectangle( uint16_t x, uint16_t y, uint16_t width, uint16_t height, ili9488_rgb_t color ) {
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;

    this->color = color;
}

Rectangle::~Rectangle() {

}

void Rectangle::draw() {
    erase(this->color);
}

void Rectangle::erase(ili9488_rgb_t backgroundColor) {
    ili9488_rect_attr_t rect_attr;
    rect_attr.position.x = this->x;
    rect_attr.position.y = this->y;
    rect_attr.position.width = this->width;
    rect_attr.position.height = this->height;

    rect_attr.fill.enable = true;
    rect_attr.fill.color = backgroundColor;

    rect_attr.border.enable = false;
    rect_attr.rounded.enable = false;

    ili9488_draw_rectangle(&rect_attr);
}

