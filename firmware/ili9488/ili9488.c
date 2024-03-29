// Copyright (c) 2021 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      ili9488.c
*@brief     Application layer of ILI9488 chip
*@author    Ziga Miklosic
*@date      31.04.2021
*@version	V1.0.1
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup ILI9488_API
* @{ <!-- BEGIN GROUP -->
*
* 	Application layer for ILI9488 chip
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "ili9488.h"
#include "ili9488_driver.h"
#include "ili9488_font.h"
#include "ili9488_cfg.h"
#include "ili9488_if.h"
#include "pico/stdlib.h"

// Strings
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

// Pen
typedef struct
{
	ili9488_rgb_t 	fg_color;
	ili9488_rgb_t 	bg_color;
	ili9488_font_opt_t	font_opt;
} ili9488_pen_t;

// Cursor
typedef struct
{
	uint16_t page;
	uint16_t col;
} ili9488_cursor_t;


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

// Pen
static ili9488_pen_t g_stringPen;

// Cursor
static ili9488_cursor_t g_stringCursor;

// Initialization flag
static bool gb_is_init;

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////
static ili9488_status_t ili9488_fill_round_rectangle(const uint16_t page, const uint16_t col, const uint16_t p_size, const uint16_t c_size, const ili9488_rgb_t color, const uint8_t radius);
static ili9488_status_t ili9488_fill_rectangle		(const uint16_t page, const uint16_t col, const uint16_t p_size, const uint16_t c_size, const ili9488_rgb_t color);
static ili9488_status_t ili9488_fill_circle			(const uint16_t page, const uint16_t col, const uint16_t radius, const ili9488_rgb_t color);

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*		Initialize display
*
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_init(void)
{
	ili9488_status_t status = eILI9488_OK;

	// Not jet initialized
	if ( false == gb_is_init )
	{
		// Display init procedure
		if ( eILI9488_OK != ili9488_driver_init() )
		{
			gb_is_init = false;
			status = eILI9488_ERROR;
		}
		else
		{
			gb_is_init = true;
		}

		// Init string pen
		g_stringPen.bg_color = ILI9488_COLOR_BLACK;
		g_stringPen.fg_color = ILI9488_COLOR_WHITE;
		g_stringPen.font_opt = eILI9488_FONT_16;

		// Init string cursor
		g_stringCursor.page = 0;
		g_stringCursor.col = 0;

	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set display background
*
* @param[in] 	color - Color of background
* @return 		status - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_set_background(const ili9488_rgb_t color)
{
	ili9488_status_t status = eILI9488_OK;

	// Check if init
	if ( true == gb_is_init )
	{
		status |= ili9488_driver_fill_rectangle(0, 0, ILI9488_DISPLAY_SIZE_PAGE, ILI9488_DISPLAY_SIZE_COLUMN, color );

		// NOTE: This is mandatory only first time background is set. Don't know why...
		status |= ili9488_driver_fill_rectangle(0, 0, 3, ILI9488_DISPLAY_SIZE_COLUMN, color );
	}
	else
	{
		status = eILI9488_ERROR;

		ILI9488_DBG_PRINT( "Module not initialized!" );
		ILI9488_ASSERT( 0 );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * 		Convert hex styled uint32_t to ili9488_rgb_t
 * 
 * @param[in] 	color - Hex value
 * @return 		rgb - RGB value
 */
////////////////////////////////////////////////////////////////////////////////

ili9488_rgb_t ili9488_hex_to_rgb (const uint32_t color)
{
	ili9488_rgb_t rgb;

	rgb.r = (color >> 16) & 0xFF;
	rgb.g = (color >> 8) & 0xFF;
	rgb.b = color & 0xFF;

	return rgb;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set string pen
*
* @param[in]	fg_color - Foreground color
* @param[in]	bg_color - Background color
* @param[in]	font_opt - Font of choise
* @return 		status - Always OK
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_set_string_pen(const ili9488_rgb_t fg_color, const ili9488_rgb_t bg_color, const ili9488_font_opt_t font_opt)
{
	ili9488_status_t status = eILI9488_OK;

	if ( gb_is_init )
	{
		g_stringPen.fg_color = fg_color;
		g_stringPen.bg_color = bg_color;
		g_stringPen.font_opt = font_opt;
	}
	else
	{
		status = eILI9488_ERROR;

		ILI9488_DBG_PRINT( "Module not initialized!" );
		ILI9488_ASSERT( 0 );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Display string
*
* @param[in]	str - String to display
* @param[in]	page - Start page
* @param[in]	col - Start column
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_set_string(const char* str, const uint16_t page, const uint16_t col)
{
	ili9488_status_t status = eILI9488_OK;

	if ( true == gb_is_init )
	{
		status = ili9488_driver_set_string( str, page, col, g_stringPen.fg_color, g_stringPen.bg_color, g_stringPen.font_opt );
	}
	else
	{
		status = eILI9488_ERROR;

		ILI9488_DBG_PRINT( "Module not initialized!" );
		ILI9488_ASSERT( 0 );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Get status of initialisation
*
* @return 		initialisation status
*/
////////////////////////////////////////////////////////////////////////////////
bool ili9488_is_init(void)
{
	return gb_is_init;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set cursor for printf functionality
*
* @param[in] 	page 	- Cursor page (x) coordinate
* @param[in] 	col 	- Cursor column (y) coordinate
* @return 		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_set_cursor(const uint16_t page, const uint16_t col)
{
	ili9488_status_t status = eILI9488_OK;

	// Check if init
	if ( true == gb_is_init )
	{
		// Check limits
		if 	(	( page < ILI9488_DISPLAY_SIZE_PAGE )
			&& 	( col < ILI9488_DISPLAY_SIZE_PAGE ))
		{
			g_stringCursor.page = page;
			g_stringCursor.col = col;
		}
		else
		{
			g_stringCursor.page = 0;
			g_stringCursor.col = 0;

			status = eILI9488_ERROR;

			ILI9488_DBG_PRINT( "Cursor set invalid coordinates!" );
			ILI9488_ASSERT( 0 );
		}
	}
	else
	{
		status = eILI9488_ERROR;

		ILI9488_DBG_PRINT( "Module not initialized!" );
		ILI9488_ASSERT( 0 );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Get current info of cursor
*
* @param[out] 	p_page	- Pointer to current cursor page
* @param[out] 	p_col	- Pointer to current cursor column
* @return 		void
*/
////////////////////////////////////////////////////////////////////////////////
void ili9488_get_cursor(uint16_t * const p_page, uint16_t * const p_col)
{
	if ( NULL != p_page )
	{
		*p_page = g_stringCursor.page;
	}

	if ( NULL != p_col )
	{
		*p_col = g_stringCursor.col;
	}
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Print formated string to display
*
* @param[in] 	format - Formated string
* @return 		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_printf(const char *format, ...)
{
	ili9488_status_t status = eILI9488_OK;
	va_list args;
	static uint8_t str[128];
	uint8_t ch_num;
	uint16_t page_walker;
	uint16_t col_walker;
	uint8_t font_width;
	uint8_t font_height;
	ili9488_font_opt_t font_opt;
	uint8_t i;

	// Check if init
	if ( true == gb_is_init )
	{
		// Taking args from stack
		va_start(args, format);
		vsprintf((char*) str, format, args);
		va_end(args);

		// Get font
		font_opt = g_stringPen.font_opt;

		// Set page/col
		page_walker = g_stringCursor.page;
		col_walker = g_stringCursor.col;

		// Get font width
		font_width = ili9488_font_get_width( font_opt );
		font_height = ili9488_font_get_height( font_opt );

		// Get number of chars
		ch_num = strlen((const char*) &str );

		// Get line break indexes inside strings
		for ( i = 0; i < ch_num; i++ )
		{
			// Ignore new line
			if (( '\r' == str[i] ) || ( '\n' == str[i] ))
			{
				col_walker += font_height;
				g_stringCursor.col = col_walker;
				page_walker = g_stringCursor.page;
			}

			// Set chars
			else
			{
				ili9488_driver_set_char( str[ i ], page_walker, col_walker, g_stringPen.fg_color, g_stringPen.bg_color, g_stringPen.font_opt );
				page_walker += font_width;
			}
		}
	}
	else
	{
		status = eILI9488_ERROR;

		ILI9488_DBG_PRINT( "Module not initialized!" );
		ILI9488_ASSERT( 0 );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Get width of currently used font
*
* @return 	font_width - Width of used font
*/
////////////////////////////////////////////////////////////////////////////////
uint8_t	ili9488_get_font_width(void)
{
	uint8_t font_width = 0;

	// Is init
	ILI9488_ASSERT( true == gb_is_init );

	font_width = ili9488_font_get_width( g_stringPen.font_opt );

	return font_width;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Get height of currently used font
*
* @return 	font_width - Width of used font
*/
////////////////////////////////////////////////////////////////////////////////
uint8_t	ili9488_get_font_height(void)
{
	uint8_t font_height = 0;

	// Is init
	ILI9488_ASSERT( true == gb_is_init );

	font_height = ili9488_font_get_height( g_stringPen.font_opt );

	return font_height;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Draw rectangle
*
* @param[in] 	p_rectanegle_attr - Pointer to rectangle attributes
* @return 		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_draw_rectangle(const ili9488_rect_attr_t * const p_rectanegle_attr)
{
	ili9488_status_t status = eILI9488_OK;
	uint16_t s_page;
	uint16_t s_col;
	uint16_t page_size;
	uint16_t col_size;
	ili9488_rgb_t border_color;
	ili9488_rgb_t fill_color;
	uint8_t border_width;
	uint8_t radius;

	// Check if init
	if ( true == gb_is_init )
	{
		// Get position data
		s_page 		= p_rectanegle_attr -> position.x;
		s_col 		= p_rectanegle_attr -> position.y;
		page_size 	= p_rectanegle_attr -> position.width;
		col_size 	= p_rectanegle_attr -> position.height;

		// Get color
		border_color = p_rectanegle_attr -> border.color;
		fill_color = p_rectanegle_attr -> fill.color;

		// Get border width
		border_width = p_rectanegle_attr -> border.width;

		// Get radius
		radius = p_rectanegle_attr -> rounded.radius;

		// Rounded rectangle
		if ( true == p_rectanegle_attr -> rounded.enable )
		{
			// Draw Border
			if ( true == p_rectanegle_attr -> border.enable )
			{
				// Fill border
				status |= ili9488_fill_round_rectangle( s_page, s_col, page_size, col_size, border_color, radius );

				// Fill without border
				status |= ili9488_fill_round_rectangle( s_page + border_width, s_col + border_width, page_size - 2U * border_width, col_size - 2U * border_width, fill_color, radius );
			}

			// Rounded & Filled
			else if ( true == p_rectanegle_attr -> fill.enable )
			{
				status |= ili9488_fill_round_rectangle( s_page, s_col, page_size, col_size, fill_color, radius );
			}

			else
			{
				// No actions...
			}
		}

		// Simple rectangle
		else
		{
			// With border
			if ( true == p_rectanegle_attr -> border.enable )
			{
				// Fill border
				status |= ili9488_fill_rectangle( s_page, s_col, page_size, col_size, border_color );

				// Fill without border
				status |= ili9488_fill_rectangle( s_page + border_width, s_col + border_width, page_size - ( 2U * border_width ), col_size - ( 2U * border_width ), fill_color );
			}

			// Filled
			else if ( true == p_rectanegle_attr -> fill.enable )
			{
				status |= ili9488_fill_rectangle( s_page, s_col, page_size, col_size, fill_color );
			}

			else
			{
				status = eILI9488_ERROR;
			}
		}
	}
	else
	{
		status = eILI9488_ERROR;

		ILI9488_DBG_PRINT( "Module not initialized!" );
		ILI9488_ASSERT( 0 );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Fill round rectangle
*
* @param[in]	page 	- Page start
* @param[in]	col 	- Column start
* @param[in]	p_size 	- Page size
* @param[in]	c_size 	- Column i+size
* @param[in]	color 	- Color of rectange
* @param[in]	radius 	- Size of radius
* @return		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static ili9488_status_t ili9488_fill_round_rectangle(const uint16_t page, const uint16_t col, const uint16_t p_size, const uint16_t c_size, const ili9488_rgb_t color, const uint8_t radius)
{
	ili9488_status_t status = eILI9488_OK;

	// Check if init
	if ( gb_is_init )
	{
		// Combinations with rectanangles and cicles to produce rounded rectangle
		status |= ili9488_fill_rectangle( page ,	 		col + radius, p_size + 1, 	c_size - 2 * radius, 				color );
		status |= ili9488_fill_rectangle( page + radius, 	col, p_size - 2 * radius, 	radius, 							color );
		status |= ili9488_fill_rectangle( page + radius, 	col + c_size - radius, 		p_size - 2 * radius, radius + 1,	color );

		status |= ili9488_fill_circle( page + radius, 			col + radius, 			radius, color );
		status |= ili9488_fill_circle( page + radius, 			col + c_size - radius, 	radius, color );
		status |= ili9488_fill_circle( page + p_size - radius, 	col + radius,			radius, color );
		status |= ili9488_fill_circle( page + p_size  - radius,	col + c_size - radius, 	radius, color );
	}
	else
	{
		status = eILI9488_ERROR;

		ILI9488_DBG_PRINT( "Module not initialized!" );
		ILI9488_ASSERT( 0 );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Fill rectangle
*
* @param[in]	page 		- Start page
* @param[in]	col 		- Start column
* @param[in]	c_size 		- Size of column
* @param[in]	p_size 		- Size of page
* @param[in]	color 		- Color of rectangle
* @return		status 		- Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_fill_rectangle(const uint16_t page, const uint16_t col, const uint16_t p_size, const uint16_t c_size, const ili9488_rgb_t color)
{
	ili9488_status_t status = eILI9488_OK;

	// Check if init
	if ( true == gb_is_init )
	{
		if ( eILI9488_OK != ili9488_driver_fill_rectangle(page, col, p_size, c_size, color ))
		{
			status = eILI9488_ERROR;
		}
	}
	else
	{
		status = eILI9488_ERROR;

		ILI9488_DBG_PRINT( "Module not initialized!" );
		ILI9488_ASSERT( 0 );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Draw circle
*
* @param[in] 	p_circle_attr 	- Pointer to circle attributes
* @return 		status 			- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_draw_circle(const ili9488_circ_attr_t * const p_circle_attr)
{
	ili9488_status_t status = eILI9488_OK;
	uint16_t s_page;
	uint16_t s_col;
	uint16_t radius;
	ili9488_rgb_t fill_color;
	ili9488_rgb_t border_color;
	uint16_t border_width;

	// Check if init
	if ( true == gb_is_init )
	{
		// Get positions
		s_page = p_circle_attr -> position.x;
		s_col = p_circle_attr -> position.y;
		radius = p_circle_attr -> position.radius;

		// Get colors
		fill_color = p_circle_attr -> fill.color;
		border_color = p_circle_attr -> border.color;

		// Get border width
		border_width = p_circle_attr -> border.width;

		// Circle with border
		if ( true == p_circle_attr -> border.enable )
		{
			// Fill border
			status |= ili9488_fill_circle( s_page, s_col, radius, border_color );

			// Fill background
			status |= ili9488_fill_circle( s_page, s_col, radius - border_width, fill_color );
		}

		// Simple filled circle
		else if ( true == p_circle_attr -> fill.enable )
		{
			status |= ili9488_fill_circle( s_page, s_col, radius, fill_color );
		}

		else
		{
			status = eILI9488_ERROR;
		}
	}
	else
	{
		status = eILI9488_ERROR;

		ILI9488_DBG_PRINT( "Module not initialized!" );
		ILI9488_ASSERT( 0 );
	}


	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Fill (draw) circle
*
* @param[in]	page 	- Start page
* @param[in]	col 	- Start column
* @param[in]	radius	- Size of radius
* @param[in]	color 	- Color of rectangle
* @return 		status 	- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static ili9488_status_t ili9488_fill_circle(const uint16_t page, const uint16_t col, const uint16_t radius, const ili9488_rgb_t color)
{
	ili9488_status_t status = eILI9488_OK;

	// Check if init
	if ( true == gb_is_init )
	{
		if ( eILI9488_OK != ili9488_driver_fill_circle(page, col, radius, color ))
		{
			status = eILI9488_ERROR;
		}
	}
	else
	{
		status = eILI9488_ERROR;

		ILI9488_DBG_PRINT( "Module not initialized!" );
		ILI9488_ASSERT( 0 );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set up display backlight
*
* @param[in] 	brightness 	- Brightness value from 0.0 - 1.0
* @return 		status 		- Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_set_backlight(const float brightness)
{
	ili9488_status_t status = eILI9488_OK;

	// Check if init
	if ( true == gb_is_init )
	{
		ili9488_if_set_led( brightness );
	}
	else
	{
		status = eILI9488_ERROR;

		ILI9488_DBG_PRINT( "Module not initialized!" );
		ILI9488_ASSERT( 0 );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

