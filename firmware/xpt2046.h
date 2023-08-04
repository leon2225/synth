#pragma once
// Copyright (c) 2023 Leon Farchau
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      xpt2046.c
*@brief     Interface to XPT2046 touch controller
*@author    Leon Farchau
*@date      31/07/2023
*@version	V1.0.0
*/

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdbool.h>
#include <stdint.h>

#include "point.h"

////////////////////////////////////////////////////////////////////////////////
// Data types
////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    Point       position;
    uint16_t    pressure;
} XPT2046_TouchData_t;

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#define XPT2046_SPI     spi1
#define XPT2046_CS      16
#define XPT2046_IRQ     17
#define XPT2046_FREQ    (2.5 * 1000 * 1000)

////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////
void                    XPT2046_Init            (void);
XPT2046_TouchData_t     XPT2046_getTouch        (void);
void                    XPT2046_update          (void);
