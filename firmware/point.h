/**
 * @file point.h
 * @author Leon Farchau (leon2225)
 * @brief Class that represents a point
 * @version 0.1
 * @date 04.08.2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include "stdint.h"

class Point
{
    public:
        Point(){Point(0,0);}
        Point( uint16_t x, uint16_t y ){ this->x = x; this->y = y;}
        ~Point(){};

        Point xPart() const { return Point(this->x, 0);}
        Point yPart() const { return Point(0, this->y);}

        uint16_t x;
        uint16_t y;

        bool operator==(const Point& other) const {
            return (this->x == other.x) && (this->y == other.y);}
        bool operator!=(const Point& other) const {
            return !(*this == other);}
        Point operator+(const Point& other) const {
            return Point(this->x + other.x, this->y + other.y);}
        Point operator-(const Point& other) const {
            return Point(this->x - other.x, this->y - other.y);}
        Point operator+=(const Point& other) {
            this->x += other.x; this->y += other.y; return *this;}
        Point operator-=(const Point& other) {
            this->x -= other.x; this->y -= other.y; return *this;}

};