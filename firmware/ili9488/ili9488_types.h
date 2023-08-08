#pragma once


////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// Typedefs
////////////////////////////////////////////////////////////////////////////////

// General status
typedef enum
{
	eILI9488_OK 		= 0x00,		/**<Normal operation */
	eILI9488_ERROR 		= 0x01,		/**<General error */
	eILI9488_ERROR_SPI	= 0x02,		/**<SPI interface error */
} ili9488_status_t;

// Standard rbg
struct RGB_Color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
#ifdef __cplusplus
	bool operator==(const RGB_Color& other) const
	{
		return (r == other.r && g == other.g && b == other.b);
	}
	bool operator!=(const RGB_Color& other) const
	{
		return !(*this == other);
	}
#endif
};
typedef struct RGB_Color ili9488_rgb_t;


// Font options
typedef enum
{
	eILI9488_FONT_8 = 0,
	eILI9488_FONT_12,
	eILI9488_FONT_16,
	eILI9488_FONT_20,
	eILI9488_FONT_24,

	eILI9488_FONT_NUM_OF,
} ili9488_font_opt_t;

// Rectangle attributes
typedef struct
{
	// Position of rectangle
	struct
	{
		uint16_t x;
		uint16_t y;
		uint16_t width;
		uint16_t height;
	} position;

	// Rounded corners
	struct
	{
		bool	enable;
		uint8_t	radius;
	} rounded;

	// Filled rectangle
	struct
	{
		bool			enable;
		ili9488_rgb_t color;
	} fill;

	// Rectangle with border
	struct
	{
		bool			enable;
		ili9488_rgb_t color;
		uint8_t			width;
	} border;

} ili9488_rect_attr_t;

// Circle attributes
typedef struct
{
	// Position & size of circle
	struct
	{
		uint16_t x;
		uint16_t y;
		uint8_t radius;
	} position;

	// Filled circle
	struct
	{
		bool			enable;
		ili9488_rgb_t color;
	} fill;

	// Border
	struct
	{
		bool 			enable;
		ili9488_rgb_t	color;
		uint8_t			width;
	} border;

} ili9488_circ_attr_t;

#ifdef __cplusplus
}
#endif