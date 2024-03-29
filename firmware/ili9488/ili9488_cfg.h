// Copyright (c) 2021 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      ili9488_cfg.h
*@brief     Configuration file for ILI9488 driver
*@author    Ziga Miklosic
*@date      31.04.2021
*@version	V1.0.1
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup ILI9488_CFG
* @{ <!-- BEGIN GROUP -->
*
* 	Configurations for ILI9488 driver.
*/
////////////////////////////////////////////////////////////////////////////////

#ifndef _ILI9488_CFG_H_
#define _ILI9488_CFG_H_

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

// Enable debug mode (0/1)
#define ILI9488_DEBUG_EN				( 0 )

// **********************************************************
// 	ORIENTATION
// **********************************************************

// Flip display landscape orientation (0/1)
#define ILI9488_DISPLAY_FLIP			( 0 )


// **********************************************************
// 	FONTS
// **********************************************************
//
//	Here unused fonts can be disabled in order to save
// 	flash memory space

// Enable usage of font 8 (0/1)
// Size: 760 B
#define ILI9488_FONT_8_USE_EN			( 1 )

// Enable usage of font 12 (0/1)
// Size: 1,11 kB
#define ILI9488_FONT_12_USE_EN			( 1 )

// Enable usage of font 16 (0/1)
// Size: 2,97 kB
#define ILI9488_FONT_16_USE_EN			( 1 )

// Enable usage of font 20 (0/1)
// Size: 3,71 kB
#define ILI9488_FONT_20_USE_EN			( 1 )

// Enable usage of font 24 (0/1)
// Size: 6,68 kB
#define ILI9488_FONT_24_USE_EN			( 1 )


// **********************************************************
// 	BACKLIGHT
// **********************************************************

// Start-up brightness (0.0f - 1.0f)
#define ILI9488_LED_STARTUP_VAL			( 1.0f )

// **********************************************************
// 	DEBUG COM PORT
// **********************************************************
#if ( ILI9488_DEBUG_EN )
	#define ILI9488_DBG_PRINT( ... )					( com_dbg_print( eCOM_DBG_CH_INFO, (const char*) __VA_ARGS__ ))
	#define ILI9488_ASSERT(x)							if ( 0 == x ) { ILI9488_DBG_PRINT("ASSERT ERROR! file: %s, line: %d", __FILE__, __LINE__ ); while(1){} }
#else
	#define ILI9488_DBG_PRINT( ... )					{ ; }
	#define ILI9488_ASSERT(x)							{ ; }
#endif

#endif // _ILI9488_CFG_H_
