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
#include "../ili9488/ili9488.h"
#include "point.h"

class Rectangle {
    public:
        Rectangle();
        Rectangle( Point position, Point size, ili9488_rgb_t color );
        ~Rectangle();

        Point getPosition() const { return this->position;}
        Point getSize() const { return this->size;}
        ili9488_rgb_t getColor() const { return this->color;}
        bool contains( Point p ) const ;

        void setPosition( Point position ){ this->position = position;}
        void setSize( Point size ){ this->size = size;}
        void setColor( ili9488_rgb_t color ){ this->color = color;}

        void draw();
        void erase(ili9488_rgb_t bgColor);

    private:
        Point position;
        Point size;

        ili9488_rgb_t color;
};