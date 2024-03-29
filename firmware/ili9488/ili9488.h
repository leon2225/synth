#pragma once
// Copyright (c) 2021 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      ili9488.h
*@brief     API for ILI9488 chipset
*@author    Ziga Miklosic
*@date      31.04.2021
*@version	V1.0.1
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup ILI9488_API
* @{ <!-- BEGIN GROUP -->
*
* 	Application functions for ILI9488
*/
////////////////////////////////////////////////////////////////////////////////

#ifndef _ILI9488_H_
#define _ILI9488_H_
#ifdef __cplusplus
extern "C" {
#endif
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
//#include "../../ili9488.h"
#include <stdint.h>
#include <stdbool.h>
#include "ili9488_types.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
/**
 * 	Module version
 */
#define ILI9488_VER_MAJOR		( 1 )
#define ILI9488_VER_MINOR		( 0 )
#define ILI9488_VER_DEVELOP		( 0 )

// Colors
#define ILI9488_COLOR_BLACK			( {ili9488_rgb_t c = {0x00, 0x00, 0x00}; c;} )
#define ILI9488_COLOR_BLUE			( {ili9488_rgb_t c = {0x00, 0x00, 0xFF}; c;} )
#define ILI9488_COLOR_GREEN			( {ili9488_rgb_t c = {0x00, 0xFF, 0x00}; c;} )
#define ILI9488_COLOR_TURQUOISE		( {ili9488_rgb_t c = {0x00, 0xFF, 0xFF}; c;} )
#define ILI9488_COLOR_RED			( {ili9488_rgb_t c = {0xFF, 0x00, 0x00}; c;} )
#define ILI9488_COLOR_PURPLE		( {ili9488_rgb_t c = {0xFF, 0x00, 0xFF}; c;} )
#define ILI9488_COLOR_YELLOW		( {ili9488_rgb_t c = {0xFF, 0xFF, 0x00}; c;} )
#define ILI9488_COLOR_WHITE			( {ili9488_rgb_t c = {0xFF, 0xFF, 0xFF}; c;} )
#define ILI9488_COLOR_LIGHT_GRAY	( {ili9488_rgb_t c = ili9488_hex_to_rgb(0x8e8181); c;} )
#define ILI9488_COLOR_GRAY			( {ili9488_rgb_t c = ili9488_hex_to_rgb(0x4d4747); c;} )
#define ILI9488_COLOR_DARK_GRAY		( {ili9488_rgb_t c = ili9488_hex_to_rgb(0x232020); c;} )

#define DISPLAY_WIDTH		( 480 )
#define DISPLAY_HEIGHT		( 320 )

////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////

// General functions
ili9488_status_t 	ili9488_init			(void);
bool				ili9488_is_init			(void);
ili9488_status_t 	ili9488_set_backlight	(const float brightness);
ili9488_rgb_t		ili9488_hex_to_rgb		(const uint32_t color);

// Graphics functions
ili9488_status_t 	ili9488_set_background	(const ili9488_rgb_t color);
ili9488_status_t	ili9488_draw_rectangle	(const ili9488_rect_attr_t * const p_rectanegle_attr);
ili9488_status_t	ili9488_draw_circle		(const ili9488_circ_attr_t * const p_circ_attr);

// Text functions
ili9488_status_t 	ili9488_set_string_pen	(const ili9488_rgb_t fg_color, const ili9488_rgb_t bg_color, const ili9488_font_opt_t font_opt);
ili9488_status_t 	ili9488_set_string		(const char* str, const uint16_t page, const uint16_t col);
ili9488_status_t	ili9488_set_cursor		(const uint16_t page, const uint16_t col);
void				ili9488_get_cursor		(uint16_t * const p_page, uint16_t * const p_col);
ili9488_status_t	ili9488_printf			(const char *args, ...);

uint8_t	ili9488_get_font_width	(void);
uint8_t	ili9488_get_font_height	(void);

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
#endif // _ILI9488_H_
