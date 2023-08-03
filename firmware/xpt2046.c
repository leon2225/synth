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
#include "xpt2046.h"
#include "pico/stdlib.h"
#include <stdio.h>

// USER INCLUDES BEGIN...

#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "ili9488_if.h"

// USER INCLUDES END...

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
extern const uint DEBUG1_PIN;
extern const uint DEBUG2_PIN;
extern const uint DEBUG3_PIN;
extern const uint DEBUG4_PIN;

const uint32_t max_x = 3970;
const uint32_t min_x = 245;
const uint32_t max_y = 310;
const uint32_t min_y = 3870;

#define Z_THRESHOLD     300
#define Z_THRESHOLD_INT	75

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
int32_t xraw=0, yraw=0, zraw=0;
uint32_t usraw=0x00000000;

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////
int32_t constrain(int32_t x, int32_t min, int32_t max);
int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);
static int16_t besttwoavg( int16_t x , int16_t y , int16_t z );
////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*		@brief		Initializes the XPT2046 touch controller
*		@param		none
*		@return		none
*/
////////////////////////////////////////////////////////////////////////////////
void XPT2046_Init(void)
{
    gpio_init(XPT2046_CS);
    gpio_init(XPT2046_IRQ);
    gpio_set_dir(XPT2046_CS, GPIO_OUT);
    gpio_set_dir(XPT2046_IRQ, GPIO_IN);
    gpio_put(XPT2046_CS, 1);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief       Reads the current touch position
 * @return      XPT2046_TouchData_t
 */
////////////////////////////////////////////////////////////////////////////////
XPT2046_TouchData_t XPT2046_getTouch()
{
    XPT2046_TouchData_t touchData;
    int32_t x = 0;
    int32_t y = 0;

    // convert raw resistance-Values to display coordinates
    x = map(xraw, min_x, max_x, 0, 480);
    y = map(yraw, min_y, max_y, 0, 320);
    touchData.x = constrain(x, 0, 480);
    touchData.y = constrain(y, 0, 320);
    touchData.pressure = zraw;
    return touchData;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief       Cyclic handler for the touch controller, 
 *                  should be called as often as possible
 * @param       none
 * @return      XPT2046_TouchData_t
 */
////////////////////////////////////////////////////////////////////////////////
void XPT2046_update()
{
	int16_t data[6];
	int z;
    uint32_t displayBaudrate = spi_get_baudrate(XPT2046_SPI);

    // Pause display-DMA
    ili9488_if_pause_dma();
    ili9488_if_set_cs(1);
    gpio_put(eGPIO_D_CS, 1);

    //  and config SPI for touch
    gpio_put(XPT2046_CS, 0);
    spi_set_baudrate(XPT2046_SPI, XPT2046_FREQ);

    // read pressure (Z1&z2) + dummy X measure + 3 X+Y measures + 1 dummy
    uint16_t txBuffer[10] = {0x00B1 /* Z1 */, 0xC1 /* Z2 */, 0x91 /* X */,
                            0x91 /* X */, 0xD1 /* Y */, 0x91 /* X */,
                            0xD1 /* Y */, 0x91 /* X */, 0xD0 /* Y */, 0x00};
    uint16_t rxBuffer[10] = {0x00};

    // calculate pressure
    spi_write16_read16_blocking(XPT2046_SPI, txBuffer, rxBuffer, 3);
    z = (int16_t)(rxBuffer[1] >> 3) + 4095;     // Z1 + 4095
    z -= (int16_t)(rxBuffer[2] >> 3);           // - Z2

    if (z >= Z_THRESHOLD) {
        spi_write16_read16_blocking(XPT2046_SPI, txBuffer + 3, rxBuffer + 3, 5);
    }
    else 
    {
        // Compiler warns these values may be used unset on early exit.
        rxBuffer[3] = rxBuffer[4] = rxBuffer[5] = rxBuffer[6] = 0;	
    }
    spi_write16_read16_blocking(XPT2046_SPI, txBuffer + 8, rxBuffer + 8, 2);
    
    // End Touch transmisson
    gpio_put(XPT2046_CS, 1);

    // And resume display-DMA
    spi_set_baudrate(XPT2046_SPI, displayBaudrate);
    ili9488_if_set_cs(0);
    ili9488_if_resume_dma();
    

    z = z<0?0:z;
	if (z < Z_THRESHOLD) { //	if ( !touched ) {
		zraw = 0;
		return;
	}
	zraw = z;
	
	// Average pair with least distance between each measured x then y
	int16_t x = besttwoavg( rxBuffer[4] >> 3, rxBuffer[6] >> 3, rxBuffer[8] >> 3);
	int16_t y = besttwoavg( rxBuffer[5] >> 3, rxBuffer[7] >> 3, rxBuffer[9] >> 3);

    xraw = x;
    yraw = y;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief       Maps a value from one range to another
 * @param[in]   x:      Value to map
 * @param[in]   in_min: Minimum value of the input range
 * @param[in]   in_max: Maximum value of the input range
 * @param[in]   out_min: Minimum value of the output range
 * @param[in]   out_max: Maximum value of the output range
 * @return      uint32_t
 */
////////////////////////////////////////////////////////////////////////////////
int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief       Constrains a value to a range
 * @param[in]   x:      Value to constrain
 * @param[in]   min:    Minimum value of the range
 * @param[in]   max:    Maximum value of the range
 * @return      uint32_t
 */
////////////////////////////////////////////////////////////////////////////////
int32_t constrain(int32_t x, int32_t min, int32_t max) {
    if (x < min) {
        return min;
    } else if (x > max) {
        return max;
    }
    return x;
}


////////////////////////////////////////////////////////////////////////////////
/**
 * @brief       Returns the avg of the best fitting pair of values and discrads the third
 * @return      avg of the best fitting pair of values
 */
////////////////////////////////////////////////////////////////////////////////
static int16_t besttwoavg( int16_t x , int16_t y , int16_t z ) {
  int16_t da, db, dc;
  int16_t reta = 0;
  if ( x > y ) da = x - y; else da = y - x;
  if ( x > z ) db = x - z; else db = z - x;
  if ( z > y ) dc = z - y; else dc = y - z;

  if ( da <= db && da <= dc ) reta = (x + y) >> 1;
  else if ( db <= da && db <= dc ) reta = (x + z) >> 1;
  else reta = (y + z) >> 1;   //    else if ( dc <= da && dc <= db ) reta = (x + y) >> 1;

  return (reta);
}