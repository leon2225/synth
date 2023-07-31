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

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////

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
 * @param       none
 * @return      XPT2046_TouchData_t
 */
////////////////////////////////////////////////////////////////////////////////
XPT2046_TouchData_t XPT2046_Read(void)
{
    XPT2046_TouchData_t touchData;
    uint32_t displayBaudrate = spi_get_baudrate(XPT2046_SPI);
    touchData.x = 0;
    touchData.y = 0;
    touchData.pressed = false;

    if (gpio_get(XPT2046_IRQ) == 0)
    {
        // pause display-DMA 
        ili9488_if_pause_dma();
        ili9488_if_set_cs(1);
        gpio_put(eGPIO_D_CS, 1);
        gpio_xor_mask(1 << DEBUG1_PIN);

        gpio_put(XPT2046_CS, 0);
        spi_set_baudrate(XPT2046_SPI, XPT2046_FREQ);

        uint16_t txBuffer[5] = {0x9000, 0xD000, 0x0000, 0x0000, 0x00};
        uint16_t rxBuffer[5] = {0x00};

        spi_write16_read16_blocking(XPT2046_SPI, txBuffer, rxBuffer, 5);

        gpio_put(XPT2046_CS, 1);


        gpio_xor_mask(1 << DEBUG1_PIN);

        // Resume display-DMA
        spi_set_baudrate(XPT2046_SPI, displayBaudrate);
        gpio_put(DEBUG3_PIN, 1);
        ili9488_if_set_cs(0);
        ili9488_if_resume_dma();

        uint16_t x = (rxBuffer[0] << 8) | (rxBuffer[1]>>(8));
        uint16_t y = (rxBuffer[1] << 8) | (rxBuffer[2]>>(8));

        touchData.x = x;
        touchData.y = y;
        touchData.pressed = true;
    }

    return touchData;
}