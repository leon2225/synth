// Copyright (c) 2021 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      ili9488_driver.c
*@brief     Interface driver for ILI9488 chip
*@author    Ziga Miklosic
*@date      31.04.2021
*@version	V1.0.1
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup ILI9488_DRIVER
* @{ <!-- BEGIN GROUP -->
*
* 	Interface driver for ILI9488 chip
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "ili9488_driver.h"
#include "ili9488_font.h"
#include "ili9488_cfg.h"
#include "ili9488_if.h"
#include "pico/stdlib.h"
#include "pico/mem_ops.h"

// Strings
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <math.h>

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
extern const uint DEBUG1_PIN;
extern const uint DEBUG2_PIN;
extern const uint DEBUG3_PIN;
extern const uint DEBUG4_PIN;


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
uint16_t g_charBuffer[2][ILI9488_MAX_FONT_HEIGHT * ILI9488_MAX_FONT_WIDTH];	
uint8_t g_charBufferIdx = 0;

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////
static ili9488_status_t ili9488_driver_set_pos_gamma			(void);
static ili9488_status_t ili9488_driver_set_neg_gamma			(void);
static ili9488_status_t ili9488_driver_set_power_control		(void);
static ili9488_status_t ili9488_driver_set_interface_mode		(void);
static ili9488_status_t ili9488_driver_set_frame_rate			(void);
static ili9488_status_t ili9488_driver_set_inversion_control	(void);
static ili9488_status_t ili9488_driver_set_function_control		(void);
static ili9488_status_t ili9488_driver_set_image_function		(void);

static uint16_t 		ili9488_driver_convert_to_rgb565		(const ili9488_rgb_t color);
static ili9488_status_t ili9488_driver_draw_hline				(const uint16_t page, const uint16_t col, const uint16_t length, const ili9488_rgb_t color);

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*		Initializing display driver
*
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_init(void)
{
	ili9488_status_t status = eILI9488_OK;

	// Initialize low level interface
	if ( eILI9488_OK != ili9488_if_init())
	{
		status = eILI9488_ERROR;
	}
	else
	{
		// Set control lines
		ili9488_if_set_reset( false );

		// Set up initial backlight brightness
		ili9488_if_set_led( ILI9488_LED_STARTUP_VAL );

		// Soft Reset
		status |= ili9488_driver_soft_reset();
		sleep_ms( 100 );


		// Positive / Negative GAMMA
		status |= ili9488_driver_set_pos_gamma();
		status |= ili9488_driver_set_neg_gamma();

		// Power Control
		status |= ili9488_driver_set_power_control();

		// Invert display
		status |= ili9488_driver_set_display_inversion( eILI9488_DISPLAY_INVERSION_ON );

		// Interface pixel format
		status |= ili9488_driver_set_pixel_format( eILI9488_PIXEL_FORMAT_16_BIT );

		// Interface mode control
		status |= ili9488_driver_set_interface_mode();

		// Frame rate
		status |= ili9488_driver_set_frame_rate();

		// Display inversion control
		status |= ili9488_driver_set_inversion_control();

		// Display function control RGB/MCU interface control
		status |= ili9488_driver_set_function_control();

		// Image function
		status |= ili9488_driver_set_image_function();

		// Exit sleep
		status |= ili9488_driver_set_sleep_on_off( eILI9488_SLEEP_OFF );

		// Display on
		status |= ili9488_driver_set_display_on_off( eILI9488_DISPLAY_ON );

		// Memory data access control
		#if ( 0 == ILI9488_DISPLAY_FLIP )
			status |= ili9488_driver_set_orientation( eILI9488_ORIENTATION_LANDSCAPE );
		#else
			status |= ili9488_driver_set_orientation( eILI9488_ORIENTATION_LANDSCAPE_FLIP );
		#endif

	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Soft Reset of display
*
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_soft_reset(void)
{
	ili9488_status_t status = eILI9488_OK;

	status = ili9488_low_if_write_register( eILI9488_SOFTRST_CMD, NULL, 0U );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set up positive gamma.
*
*		This values are taken from other github project.
*
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
static ili9488_status_t ili9488_driver_set_pos_gamma(void)
{
	ili9488_status_t status = eILI9488_OK;
	const uint8_t gamma[15] = "\x00\x03\x09\x08\x16\x0A\x3F\x78\x4C\x09\x0A\x08\x16\x1A\x0F";

	status = ili9488_low_if_write_register( eILI9488_SET_POS_GAMMA_CTRL_CMD,	(uint8_t*) &gamma, 15U);

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set up negative gamma.
*
*		This values are taken from other github project.
*
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
static ili9488_status_t ili9488_driver_set_neg_gamma(void)
{
	ili9488_status_t status = eILI9488_OK;
	const uint8_t gamma[15] = "\x00\x16\x19\x03\x0F\x05\x32\x45\x46\x04\x0E\x0D\x35\x37\x0F";

	status = ili9488_low_if_write_register( eILI9488_SET_NEG_GAMMA_CTRL_CMD, (uint8_t*) &gamma, 15U);

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set up voltage regulators.
*
*		This values are taken from other github project.
*
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
static ili9488_status_t ili9488_driver_set_power_control(void)
{
	ili9488_status_t status = eILI9488_OK;

	status |= ili9488_low_if_write_register( eILI9488_SET_POWER_CTRL_1_CMD, (uint8_t*) "\x17\x15", 		2U );
	status |= ili9488_low_if_write_register( eILI9488_SET_POWER_CTRL_2_CMD, (uint8_t*) "\x41", 			1U );
	status |= ili9488_low_if_write_register( eILI9488_SET_POWER_CTRL_3_CMD, (uint8_t*) "\x00\x12\x80", 	3U );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set interface mode control. Using 4 wire SPI.
*
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
static ili9488_status_t ili9488_driver_set_interface_mode(void)
{
	ili9488_status_t status = eILI9488_OK;

	status = ili9488_low_if_write_register( eILI9488_SET_IF_MODE_CMD, (uint8_t*) "\x00", 1U );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set up frame rate.
*
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
static ili9488_status_t ili9488_driver_set_frame_rate(void)
{
	ili9488_status_t status = eILI9488_OK;

	// 60 Hz
	status |= ili9488_low_if_write_register( eILI9488_SET_FR_NOR_MODE_CMD, (uint8_t*) "\xA0", 1U );

	// 8-bit color mode, osc no div, 16 clock per line
	//status |= ili9488_low_if_write_register( eILI9488_SET_FR_8_COLOR_CMD, (uint8_t*) "\x00\x10", 2U );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set up inversion control
*
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
static ili9488_status_t ili9488_driver_set_inversion_control(void)
{
	ili9488_status_t status = eILI9488_OK;

	// 2-dot inversion
	status |= ili9488_low_if_write_register( eILI9488_SET_INV_CTRL_CMD, (uint8_t*) "\x02", 1U );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set up function control
*
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
static ili9488_status_t ili9488_driver_set_function_control(void)
{
	ili9488_status_t status = eILI9488_OK;

	// Normal scan, scan cycle 5 frames
	status |= ili9488_low_if_write_register( eILI9488_SET_FUNC_CTRL_CMD, (uint8_t*) "\x02\0x02", 2U );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set up image function
*
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
static ili9488_status_t ili9488_driver_set_image_function(void)
{
	ili9488_status_t status = eILI9488_OK;

	// Disable 24-bit bus
	status |= ili9488_low_if_write_register( eILI9488_SET_IMG_FUNC_CMD, (uint8_t*) "\x00", 1U );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Turn display ON/OFF
*
* @param[in] 	on_off - Switch state
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_set_display_on_off(const ili9488_display_t on_off)
{
	ili9488_status_t status = eILI9488_OK;

	if ( eILI9488_DISPLAY_OFF == on_off )
	{
		status = ili9488_low_if_write_register( eILI9488_DISPLAY_OFF_CMD, NULL, 0U );
	}
	else
	{
		status = ili9488_low_if_write_register( eILI9488_DISPLAY_ON_CMD, NULL, 0U );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Put display to sleep ON/OFF
*
* @param[in] 	on_off - Switch state
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_set_sleep_on_off(const ili9488_sleep_t on_off)
{
	ili9488_status_t status = eILI9488_OK;

	if ( eILI9488_SLEEP_OFF == on_off )
	{
		status = ili9488_low_if_write_register( eILI9488_SLEEP_OUT_CMD, NULL, 0U );
	}
	else
	{
		status = ili9488_low_if_write_register( eILI9488_SLEEP_IN_CMD, NULL, 0U );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Put display into IDLE mode ON/OFF
*
* @param[in] 	on_off - Switch state
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_set_idle_on_off(const ili9488_idle_t on_off)
{
	ili9488_status_t status = eILI9488_OK;

	if ( eILI9488_IDLE_OFF == on_off )
	{
		status = ili9488_low_if_write_register( eILI9488_IDLE_MODE_OFF_CMD, NULL, 0U );
	}
	else
	{
		status = ili9488_low_if_write_register( eILI9488_IDLE_MODE_ON_CMD, NULL, 0U );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set normal or partial display mode
*
* @param[in] 	mode - Either normal or partial
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_set_mode(const ili9488_mode_t mode)
{
	ili9488_status_t status = eILI9488_OK;

	if ( eILI9488_MODE_NORMAL == mode )
	{
		status = ili9488_low_if_write_register( eILI9488_NORMAL_MODE_CMD, NULL, 0U );
	}
	else
	{
		status = ili9488_low_if_write_register( eILI9488_PARTIAL_MODE_CMD, NULL, 0U );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Invert all pixels of display
*
* @param[in] 	inv - Invertion state
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_set_display_inversion(const ili9488_inversion_t inv)
{
	ili9488_status_t status = eILI9488_OK;

	if ( eILI9488_DISPLAY_INVERSION_OFF == inv )
	{
		status = ili9488_low_if_write_register( eILI9488_DISINV_OFF_CMD, NULL, 0U );
	}
	else
	{
		status = ili9488_low_if_write_register( eILI9488_DISINV_ON_CMD, NULL, 0U );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Turn all pixels ON/OFF
*
* @param[in] 	on_off - Switch state
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_set_all_pixels(const ili9488_all_pixels_t on_off)
{
	ili9488_status_t status = eILI9488_OK;

	if ( eILI9488_ALL_PIXELS_OFF == on_off )
	{
		status = ili9488_low_if_write_register( eILI9488_ALLPIX_ON_CMD, NULL, 0U );
	}
	else
	{
		status = ili9488_low_if_write_register( eILI9488_ALLPIX_OFF_CMD, NULL, 0U );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*	Set up memory access data control
*
* @note	 Hardcoded RGB format of pixel (not BGR).
*
* @param[in] 	orientation - Orientation of display
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_set_orientation(const ili9488_orientation_t orientation)
{
	ili9488_status_t status = eILI9488_OK;
	uint8_t madctl = 0;
	ili9488_orientation_t orientation_read;

	switch( orientation )
	{
		case eILI9488_ORIENTATION_LANDSCAPE:
			madctl = 0xC8U;
			break;

		case eILI9488_ORIENTATION_LANDSCAPE_FLIP:
			madctl = 0x08U;
			break;

		default:
			madctl = 0x48U;
			ILI9488_ASSERT( 0 );
			break;
	}

	status = ili9488_low_if_write_register( eILI9488_SET_MADCTL_CMD, (uint8_t*) &madctl, 1U );

	// Verify
	if ( eILI9488_OK == status )
	{
		status |= ili9488_driver_get_orientation( &orientation_read );

		if ( orientation != orientation_read )
		{
			ILI9488_DBG_PRINT( "Verifying orientation error ..." );
			ILI9488_ASSERT( 0 );
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set up memory access data control
*
* @note	Hardcoded RGB format of pixel (not BGR).
*
* @param[in] 	p_orientation 	- Pointer to orientation of display
* @return 		status 			- Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_get_orientation(ili9488_orientation_t * const p_orientation)
{
	ili9488_status_t status = eILI9488_OK;
	uint8_t madctl;

	status = ili9488_low_if_read_register( eILI9488_READ_MADCTL_CMD, (uint8_t*) &madctl, 1U );

	switch( madctl )
	{
		case 0xC8U:
			*p_orientation = eILI9488_ORIENTATION_LANDSCAPE;
			break;

		case 0x08U:
			*p_orientation = eILI9488_ORIENTATION_LANDSCAPE_FLIP;
			break;

		default:
			*p_orientation = eILI9488_ORIENTATION_LANDSCAPE;
			ILI9488_ASSERT( 0 );
			break;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set up pixel format
*
* @param[in] 	format - Format of pixel
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_set_pixel_format(const ili9488_pixel_format_t format)
{
	ili9488_status_t status = eILI9488_OK;
	uint8_t opt = 0;
	ili9488_pixel_format_t pf;

	switch( format )
	{
		case eILI9488_PIXEL_FORMAT_3_BIT:
			opt = 0x01U;
			break;

		case eILI9488_PIXEL_FORMAT_16_BIT:
			opt = 0x05U;
			break;

		case eILI9488_PIXEL_FORMAT_18_BIT:
			opt = 0x06U;
			break;

		case eILI9488_PIXEL_FORMAT_24_BIT:
			opt = 0x07U;
			break;

		default:
			opt = 0x01U;
			ILI9488_DBG_PRINT( "Pixel Format set invalid...." );
			ILI9488_ASSERT( 0 );
			break;
	}

	status = ili9488_low_if_write_register( eILI9488_SET_PF_CMD, (uint8_t*) &opt, 1U );

	// Verify
	if ( eILI9488_OK == status )
	{
		status |= ili9488_driver_read_pixel_format( &pf );

		if ( format != pf )
		{
			ILI9488_DBG_PRINT( "Verifying pixel format error..." );
			ILI9488_ASSERT( 0 );
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Read pixel format
*
* @param[in] 	p_format - Pointer to pixel format
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_read_pixel_format(ili9488_pixel_format_t * const p_format)
{
	ili9488_status_t status = eILI9488_OK;
	uint8_t rx_buf;

	// Read register
	status = ili9488_low_if_read_register( eILI9488_READ_PF_CMD, (uint8_t*) &rx_buf, 1U );

	switch( rx_buf )
	{
		case 0x01U:
			*p_format = eILI9488_PIXEL_FORMAT_3_BIT;
			break;

		case 0x05U:
			*p_format = eILI9488_PIXEL_FORMAT_16_BIT;
			break;

		case 0x06U:
			*p_format = eILI9488_PIXEL_FORMAT_18_BIT;
			break;

		case 0x07U:
			*p_format = eILI9488_PIXEL_FORMAT_24_BIT;
			break;

		default:
			*p_format = eILI9488_PIXEL_FORMAT_3_BIT;
			ILI9488_DBG_PRINT( "Pixel Format read invalid...." );
			ILI9488_ASSERT( 0 );
			break;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set cursor
*
* @param[in]	col_s - Start column
* @param[in]	col_e - End column
* @param[in]	page_s - Start page
* @param[in]	page_e - End page
* @return		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_set_cursor(const uint16_t col_s, const uint16_t col_e, const uint16_t page_s, const uint16_t page_e)
{
	ili9488_status_t status = eILI9488_OK;
	uint8_t baundary[4];

	// Check column/page limitations
	if 	(	( page_s > page_e )
		|| 	( col_s > col_e )
		||	( page_e > ILI9488_DISPLAY_SIZE_PAGE )
		||	( col_e > ILI9488_DISPLAY_SIZE_COLUMN ))
	{
		ILI9488_DBG_PRINT( "Invalid column or page cursor setting..." );
		ILI9488_ASSERT( 0 );
		status = eILI9488_ERROR;
	}
	else
	{
		// Column boundary
		baundary[0] = (( col_s >> 8U ) & 0xFFU );
		baundary[1] = (( col_s >> 0U ) & 0xFFU );
		baundary[2] = (( col_e >> 8U ) & 0xFFU );
		baundary[3] = (( col_e >> 0U ) & 0xFFU );

		status |= ili9488_low_if_write_register( eILI9488_SET_COL_ADDR_CMD, (uint8_t*) &baundary, 4U );

		// Page boundary
		baundary[0] = (( page_s >> 8U ) & 0xFFU );
		baundary[1] = (( page_s >> 0U ) & 0xFFU );
		baundary[2] = (( page_e >> 8U ) & 0xFFU );
		baundary[3] = (( page_e >> 0U ) & 0xFFU );

		status |= ili9488_low_if_write_register( eILI9488_SET_PAGE_ADDR_CMD, (uint8_t*) &baundary, 4U );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Write to memory
*
* @param[in] 	p_mem - Pointer to data
* @param[in] 	size - Number of bytes to be writen
* @return 		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_write_memory(const uint8_t * const p_mem, const uint32_t size)
{
	ili9488_status_t status = eILI9488_OK;

	status = ili9488_low_if_write_register( eILI9488_WRITE_MEM_CMD, p_mem, size );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Read number of size bytes from memory
*
* @param[in] 	p_mem - Pointer to read data
* @param[in] 	size - Number of bytes to be writen
* @return		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_read_memory(uint8_t * const p_mem, const uint32_t size)
{
	ili9488_status_t status = eILI9488_OK;

	status = ili9488_low_if_read_register( eILI9488_READ_MEM_CMD, p_mem, size );

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Convert color to RGB code
*
* @param[in] 	color 	- Color to convert
* @return 		rgb		- RGB coded color
*/
////////////////////////////////////////////////////////////////////////////////
static uint16_t ili9488_driver_convert_to_rgb565(const ili9488_rgb_t color)
{
	ili9488_rgb565_t rgb565;

	// Convert color
	rgb565.R = (uint8_t) ( color.r >> 3U );
	rgb565.G = (uint8_t) ( color.g >> 2U );
	rgb565.B = (uint8_t) ( color.b >> 3U );

	return *((uint16_t*)(&rgb565));
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set pixel
*
* @param[in] 	col - Column
* @param[in] 	page - Page
* @param[in] 	color - Color of pixel
* @return[in]	status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_set_pixel(const uint16_t page, const uint16_t col, const ili9488_rgb_t color)
{
	ili9488_status_t status = eILI9488_OK;
	uint16_t rgb;

	// Convert color
	rgb = ili9488_driver_convert_to_rgb565( color );

	// Set cursor
	status |= ili9488_driver_set_cursor( col, col, page, page );

	// Write memory
	ili9488_low_if_write_rgb_to_gram( &rgb, 1U, false);

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Fill rectangle
*
* @param[in]	col - Start column
* @param[in]	page - Start page
* @param[in]	col_size - Size of column
* @param[in]	page_size - Size of page
* @param[in]	color - Color of rectangle
* @return		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_fill_rectangle(const uint16_t page, const uint16_t col, const uint16_t page_size, const uint16_t col_size, const ili9488_rgb_t color)
{
	ili9488_status_t status = eILI9488_OK;
	uint32_t pixel_size;
	uint16_t rgb;

	// Check limits
	if 	(	(( col + col_size ) > ILI9488_DISPLAY_SIZE_COLUMN )
		||	(( page + page_size ) > ILI9488_DISPLAY_SIZE_PAGE ))
	{
		status = eILI9488_ERROR;
		ILI9488_DBG_PRINT( "Filling rectangle invalid spacing..." );
		ILI9488_ASSERT( 0 );
	}
	else
	{
		// Convert color
		rgb = ili9488_driver_convert_to_rgb565( color );

		// Calculate size of pixels
		pixel_size = (uint32_t) ( col_size * page_size );

		// Wait until previous operation is finished
		ili9488_if_wait_for_ready();

		// Set cursor
		status |= ili9488_driver_set_cursor( col, col + col_size - 1, page, page + page_size - 1);

		// Write to memory
		status |= ili9488_low_if_write_rgb_to_gram( &rgb, pixel_size, false);
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Draw horizontal line
*
* @param[in]	page 	- Start page
* @param[in]	col 	- Start column
* @param[in]	length 	- Length of line
* @param[in]	color 	- Color of line
* @return		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
static ili9488_status_t ili9488_driver_draw_hline(const uint16_t page, const uint16_t col, const uint16_t length, const ili9488_rgb_t color)
{
	ili9488_status_t status = eILI9488_OK;
	uint16_t i;

	for ( i = 0; i < length; i++ )
	{
		status |= ili9488_driver_set_pixel( page + i, col, color );
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Fill (draw) circle
*
* @note	This function is copied from github: https://github.com/RobertoBenjami/stm32_graphics_display_drivers/blob/master/Drivers/stm32_adafruit_lcd.c
*
* @param[in]	page 	- Start page
* @param[in]	col 	- Start column
* @param[in]	radius 	- Radius of circle
* @param[in]	color 	- Color of circle
* @return		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_fill_circle(const uint16_t page, const uint16_t col, const uint16_t radius, const ili9488_rgb_t color)
{
	ili9488_status_t status = eILI9488_OK;
	int32_t  D;
	uint32_t  CurX;
	uint32_t  CurY;

	// Check limits
	if 	(	(( col + radius ) > ILI9488_DISPLAY_SIZE_COLUMN )
		||	(( page + radius ) > ILI9488_DISPLAY_SIZE_PAGE )
		||	(( col - radius ) < 0 )
		||	(( page - radius ) < 0 ))
	{
		status = eILI9488_ERROR;
		ILI9488_DBG_PRINT( "Filling circle invalid spacing..." );
		ILI9488_ASSERT( 0 );
	}
	else
	{
		D = ( 3U - ( radius << 1U ));
		CurX = 0U;
		CurY = radius;

		while ( CurX <= CurY )
		{
			if(CurY > 0)
			{
				status |= ili9488_driver_draw_hline( page - CurY, col + CurX, 2*CurY, color );
				status |= ili9488_driver_draw_hline( page - CurY, col - CurX, 2*CurY, color );
			}

			if(CurX > 0)
			{
				status |= ili9488_driver_draw_hline( page - CurX, col - CurY, 2*CurX, color );
				status |= ili9488_driver_draw_hline( page - CurX, col + CurY, 2*CurX, color );
			}

			if (D < 0)
			{
				D += (CurX << 2) + 6;
			}
			else
			{
				D += ((CurX - CurY) << 2) + 10;
				CurY--;
			}

			CurX++;
		}

		D = 3 - (radius << 1);
		CurX = 0;
		CurY = radius;

		while (CurX <= CurY)
		{
			status |= ili9488_driver_set_pixel(( page + CurX ), ( col - CurY ), color );
			status |= ili9488_driver_set_pixel(( page - CurX ), ( col - CurY ), color );
			status |= ili9488_driver_set_pixel(( page + CurY ), ( col - CurX ), color );
			status |= ili9488_driver_set_pixel(( page - CurY ), ( col - CurX ), color );
			status |= ili9488_driver_set_pixel(( page + CurX ), ( col + CurY ), color );
			status |= ili9488_driver_set_pixel(( page - CurX ), ( col + CurY ), color );
			status |= ili9488_driver_set_pixel(( page + CurY ), ( col + CurX ), color );
			status |= ili9488_driver_set_pixel(( page - CurY ), ( col + CurX ), color );

			if (D < 0)
			{
				D += (CurX << 2) + 6;
			}
			else
			{
				D += ((CurX - CurY) << 2) + 10;
				CurY--;
			}

			CurX++;
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set (draw) circle
*
*
* @note	This function is copied from github: https://github.com/RobertoBenjami/stm32_graphics_display_drivers/blob/master/Drivers/stm32_adafruit_lcd.c
*
* @param[in]	page 	- Start page
* @param[in] 	col 	- Start column
* @param[in] 	radius 	- Radius of circle
* @param[in]	color 	- Color of circle
* @return		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_set_circle(const uint16_t page, const uint16_t col, const uint16_t radius, const ili9488_rgb_t color)
{
	ili9488_status_t status = eILI9488_OK;
	int32_t  D;
	uint32_t  CurX;
	uint32_t  CurY;

	// Check limits
	if 	(	(( col + radius ) > ILI9488_DISPLAY_SIZE_COLUMN )
		||	(( page + radius ) > ILI9488_DISPLAY_SIZE_PAGE )
		||	(( col - radius ) < 0 )
		||	(( page - radius ) < 0 ))
	{
		status = eILI9488_ERROR;
		ILI9488_DBG_PRINT( "Filling circle invalid spacing..." );
		ILI9488_ASSERT( 0 );
	}
	else
	{
		D = 3 - (radius << 1);
		CurX = 0;
		CurY = radius;

		while (CurX <= CurY)
		{
			status |= ili9488_driver_set_pixel(( page + CurX ), ( col - CurY ), color );
			status |= ili9488_driver_set_pixel(( page - CurX ), ( col - CurY ), color );
			status |= ili9488_driver_set_pixel(( page + CurY ), ( col - CurX ), color );
			status |= ili9488_driver_set_pixel(( page - CurY ), ( col - CurX ), color );
			status |= ili9488_driver_set_pixel(( page + CurX ), ( col + CurY ), color );
			status |= ili9488_driver_set_pixel(( page - CurX ), ( col + CurY ), color );
			status |= ili9488_driver_set_pixel(( page + CurY ), ( col + CurX ), color );
			status |= ili9488_driver_set_pixel(( page - CurY ), ( col + CurX ), color );

			if (D < 0)
			{
				D += (CurX << 2) + 6;
			}
			else
			{
				D += ((CurX - CurY) << 2) + 10;
				CurY--;
			}

			CurX++;
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set (draw) character
*
* @param[in] 	ch - Character to display
* @param[in] 	page - Start page
* @param[in] 	col - Start column
* @param[in] 	fg_color - Foreground color
* @param[in] 	bg_color - Background color
* @param[in] 	font_opt - Font of choise
* @return		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_set_char(const uint8_t ch, const uint16_t page, const uint16_t col, const ili9488_rgb_t fg_color, const ili9488_rgb_t  bg_color, const ili9488_font_opt_t font_opt)
{
	uint16_t bg_color_u16 = ili9488_driver_convert_to_rgb565(bg_color);
	uint16_t fg_color_u16 = ili9488_driver_convert_to_rgb565(fg_color);

	ili9488_status_t status = eILI9488_OK;
	uint32_t lut_offset;
	uint8_t i;
	uint8_t j;
	uint32_t line;
	const ili9488_font_t * p_font;
	uint8_t line_size_bit;
	uint8_t line_size_byte;
	uint8_t line_bit_offset;
	uint8_t char_lut_size;

	// Get font data
	p_font = ili9488_font_get( font_opt );

	// Old function takes 9.1ms
	// New function takes 250us

	// Clear buffer (memset for 16bit seems not to work)
	uint16_t *buf = g_charBuffer[!g_charBufferIdx];
	int16_t count = (p_font->width) * p_font->height;
    while(count--) *buf++ = bg_color_u16;
	
	// Check pinter
	if ( NULL != p_font )
	{
		// Calculate various font table info
		line_size_bit = ((( p_font -> width / 8U ) * 8U ) + 8U );
		line_size_byte = ( line_size_bit / 8U );
		line_bit_offset = ( line_size_bit - ( p_font -> width ));
		char_lut_size = ( p_font -> height * line_size_byte );

		// Calculate table offset
		lut_offset = char_lut_size * ( ch - 32U );

		// Every line
		for ( i = 0; i < p_font -> height; i ++ )
		{
			// Assemble line
			line = 0;
			for ( j = 0; j < line_size_byte; j++ )
			{
				line |= ( p_font -> p_font[ lut_offset + ( i * line_size_byte ) + j ] << ( line_size_bit - ( 8U * ( j + 1U ))));
			}

			// Every pixel
			for( j = line_bit_offset; j < line_size_bit; j++ )
			{
				
				if ( line & ( 1 << j ))
				{
					g_charBuffer[!g_charBufferIdx][ ( i ) + ( (line_size_bit - j) * p_font -> height) ] = fg_color_u16;
				}
			}
		}
		// Wait until previous operation is finished
		ili9488_if_wait_for_ready();
		// Swap buffer when previous operation is finished
		g_charBufferIdx = !g_charBufferIdx;

		// Set cursor
		status |= ili9488_driver_set_cursor( col, col + p_font->height - 1, page, page + p_font->width - 1);

		// Write to memory
		status |= ili9488_low_if_write_rgb_to_gram( (uint16_t * const) &g_charBuffer[g_charBufferIdx], (p_font->width) * p_font->height, true);

	}

	// No font
	else
	{
		status = eILI9488_ERROR;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Set string
*
* @param[in] 	str - Pointer to string
* @param[in] 	page - Start page
* @param[in] 	col - Start column
* @param[in] 	fg_color - Foreground color
* @param[in]	bg_color - Background color
* @param[in] 	font_opt - Font of choise
* @return		status - Either Ok or Error
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_driver_set_string(const char *str, const uint16_t page, const uint16_t col, const ili9488_rgb_t fg_color, const ili9488_rgb_t  bg_color, const ili9488_font_opt_t font_opt)
{
	ili9488_status_t status = eILI9488_OK;
	uint16_t page_walker = page;
	uint8_t font_width;

	// Get font width
	font_width = ili9488_font_get_width( font_opt );

	// Print chars
	while( *str )
	{
		ili9488_driver_set_char( *str, page_walker, col, fg_color, bg_color, font_opt );
		str++;
		page_walker += font_width;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
