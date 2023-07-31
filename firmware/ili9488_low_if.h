#pragma once
// Copyright (c) 2021 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      ili9488_low_if.h
*@brief     Low level interface with ILI9488 chip
*@author    Ziga Miklosic
*@date      31.04.2021
*@version	V1.0.1
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup ILI9488_LOW_IF
* @{ <!-- BEGIN GROUP -->
*
* 	Low level interface with ILI9488
*/
////////////////////////////////////////////////////////////////////////////////

#ifndef _ILI9488_LOW_IF_H_
#define _ILI9488_LOW_IF_H_

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdbool.h>

#include "ili9488_regdef.h"
#include "ili9488_types.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 * 	RGB color code
 */
typedef struct
{
	unsigned int B : 5;	/**<Red color */
	unsigned int G : 6;	/**<Green color */
	unsigned int R : 5;	/**<Blue color */
} ili9488_rgb565_t;

////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t 	ili9488_low_if_write_register		(const ili9488_cmd_t cmd, const uint8_t * const tx_data, const uint32_t size);
ili9488_status_t 	ili9488_low_if_read_register		(const ili9488_cmd_t cmd, uint8_t * const rx_data, const uint32_t size);
ili9488_status_t 	ili9488_low_if_write_rgb_to_gram 	(const ili9488_rgb565_t * const p_rgb, const uint32_t size);

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
#endif // _ILI9488_LOW_IF_H_
