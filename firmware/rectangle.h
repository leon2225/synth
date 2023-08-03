/**
 * @file rectangle.h
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

class Rectangle {
    public:
        Rectangle( uint16_t x, uint16_t y, uint16_t width, uint16_t height, ili9488_rgb_t color );
        ~Rectangle();

        uint16_t getX(){ return this->x;}
        uint16_t getY(){ return this->y;}
        uint16_t getWidth(){ return this->width;}
        uint16_t getHeight(){ return this->height;}
        ili9488_rgb_t getColor(){ return this->color;}

        void setX( uint16_t x ){ this->x = x;}
        void setY( uint16_t y ){ this->y = y;}
        void setWidth( uint16_t width ){ this->width = width;}
        void setHeight( uint16_t height ){ this->height = height;}
        void setColor( ili9488_rgb_t color ){ this->color = color;}

        void draw();
        void erase(ili9488_rgb_t nackgroundColor);

    private:
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t height;

        ili9488_rgb_t color;
};