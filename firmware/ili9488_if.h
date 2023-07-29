#pragma once
// Copyright (c) 2021 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      ili9488_if.h
*@brief     Application level interface with ILI9488 chip
*@author    Ziga Miklosic
*@date      31.04.2021
*@version	V1.0.1
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup ILI9488_IF
* @{ <!-- BEGIN GROUP -->
*
* 	Application level interface with ILI9488
*
* 	@note User shall not change this file at all!
*/
////////////////////////////////////////////////////////////////////////////////

#ifndef _ILI9488_IF_H_
#define _ILI9488_IF_H_

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "ili9488_cfg.h"
#include <stdbool.h>
#include <stdint.h>

#include "ili9488.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define eGPIO_D_CS      9
#define eGPIO_D_DC      8
#define eGPIO_D_RESET   15
#define eGPIO_D_LED     13
#define eGPIO_D_CLK     10
#define eGPIO_D_MOSI    11
#define eGPIO_D_MISO    12

#define eGPIO_SPI       spi1

#define eGPIO_HIGH      1
#define eGPIO_LOW       0


////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t	ili9488_if_init			    (void);
void 				ili9488_if_set_cs		    (const bool state);
void 				ili9488_if_set_dc		    (const bool state);
void 				ili9488_if_set_reset	    (const bool state);
void 				ili9488_if_set_led		    (const float brigthness);
void                ili9488_if_wait_for_ready   (void);
ili9488_status_t 	ili9488_if_spi_transmit	    (const uint16_t * p_data, const uint32_t size, const bool incrementSrc, const bool blocking);
ili9488_status_t 	ili9488_if_spi_transmit_8b	(const uint8_t * p_data, const uint32_t size);
ili9488_status_t 	ili9488_if_spi_receive	    (uint16_t * p_data, const uint32_t size);
ili9488_status_t 	ili9488_if_spi_receive_8b   (uint8_t * p_data, const uint32_t size);

#endif // _ILI9488_IF_H_
