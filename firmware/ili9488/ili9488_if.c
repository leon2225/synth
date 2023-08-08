// Copyright (c) 2021 Ziga Miklosic
// All Rights Reserved
// This software is under MIT licence (https://opensource.org/licenses/MIT)
////////////////////////////////////////////////////////////////////////////////
/**
*@file      ili9488_if.c
*@brief     Application level interafce with ILI9488 chip
*@author    Ziga Miklosic
*@date      31.04.2021
*@version	V1.0.1
*/
////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup ILI9488_IF
* @{ <!-- BEGIN GROUP -->
*
* 	Application level interface driver for ILI9488 chip
*
*
* 	Put code that is platform depended inside code block start with
* 	"USER_CODE_BEGIN" and with end of "USER_CODE_END".
*
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "ili9488_if.h"
#include "pico/stdlib.h"

// USER INCLUDES BEGIN...

#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"

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
uint pwm_slice_num;
uint pwm_chan;
uint32_t g_dmaChannel = 0;
uint32_t g_dmaSrc = 0;
dma_channel_config g_dmaConfig;
uint32_t g_constData = 0; //Used for non-incrementing DMA transfers

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_if_dma_init(void);

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*		Initialise interface (peirphery)
*
* @note	User shall put here all hardware dependent initilisation. Those are
* 		GPIO, SPI, DMA & TIMER.
*
* 		Not needed to be initilized here but must be initiliezd before calling
* 		"ili9488_init()" function!
*
* @return		status 	- Status of initilization
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_if_init(void)
{
	ili9488_status_t status = eILI9488_OK;

	// USER CODE BEGIN...

	// Init GPIO
	gpio_init( eGPIO_D_CS );
	gpio_init( eGPIO_D_DC );
	gpio_init( eGPIO_D_RESET );

	gpio_put( eGPIO_D_CS, eGPIO_LOW );
	gpio_put( eGPIO_D_DC, eGPIO_HIGH );
	gpio_put( eGPIO_D_RESET, eGPIO_HIGH );

	gpio_set_dir( eGPIO_D_CS, GPIO_OUT );
	gpio_set_dir( eGPIO_D_DC, GPIO_OUT );
	gpio_set_dir( eGPIO_D_RESET, GPIO_OUT );

	// Init PWM
	gpio_set_function( eGPIO_D_LED, GPIO_FUNC_PWM );
	pwm_slice_num = pwm_gpio_to_slice_num( eGPIO_D_LED );
	pwm_chan = pwm_gpio_to_channel( eGPIO_D_LED );
	pwm_set_wrap( pwm_slice_num, 255 );
	pwm_set_chan_level( pwm_slice_num, pwm_chan, 0 );
	pwm_set_enabled( pwm_slice_num, true );

	// Init SPI
	gpio_set_function( eGPIO_D_CLK, GPIO_FUNC_SPI );
	gpio_set_function( eGPIO_D_MOSI, GPIO_FUNC_SPI );
	gpio_set_function( eGPIO_D_MISO, GPIO_FUNC_SPI );

	spi_init( eGPIO_SPI, 60 * 1000 * 1000 );
	spi_set_format( eGPIO_SPI, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST );

	// Init DMA
	ili9488_if_dma_init();

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Control SPI chip select line
*
* @note	User shall provide definition of that function based on used platform!
*
* @param[in]	state	- Logic state of CS line
* @return		void
*/
////////////////////////////////////////////////////////////////////////////////
void ili9488_if_set_cs(const bool state)
{
	// USER CODE BEGIN...

	if ( true == state )
	{
		gpio_put( eGPIO_D_CS, eGPIO_HIGH );
	}
	else
	{
		gpio_put( eGPIO_D_CS, eGPIO_LOW );
	}

	// USER CODE END...
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Control display DC line
*
* @note	User shall provide definition of that function based on used platform!
*
* @param[in]	state	- Logic state of DC line
* @return		void
*/
////////////////////////////////////////////////////////////////////////////////
void ili9488_if_set_dc(const bool state)
{
	// USER CODE BEGIN...

	if ( true == state )
	{
		gpio_put( eGPIO_D_DC, eGPIO_HIGH );
	}
	else
	{
		gpio_put( eGPIO_D_DC, eGPIO_LOW );
	}

	// USER CODE END...
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Control display reset line
*
* @note	User shall provide definition of that function based on used platform!
*
* @param[in]	state	- Logic state of reset line
* @return		void
*/
////////////////////////////////////////////////////////////////////////////////
void ili9488_if_set_reset(const bool state)
{
	// USER CODE BEGIN...

	if ( true == state )
	{
		gpio_put( eGPIO_D_RESET, eGPIO_LOW );
	}
	else
	{
		gpio_put( eGPIO_D_RESET, eGPIO_HIGH );
	}

	// USER CODE END...
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Control of LED dimming timer
*
* @note	User shall provide definition of that function based on used platform!
*
* @param[in]	brightness	- Brightness of backlight display LED
* @return		void
*/
////////////////////////////////////////////////////////////////////////////////
void ili9488_if_set_led(const float brigthness)
{
	// USER CODE BEGIN...
	float duty = 255.0f * brigthness;
	pwm_set_chan_level( pwm_slice_num, pwm_chan, duty );

	// USER CODE END...
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Transmit bytes via SPI
*
* @param[in]	p_data	- Pointer to transmit data
* @param[in]	size 	- Number of data to transmit
* @return		status 	- Status of transmittion
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_if_spi_transmit(const uint16_t * p_data, const uint32_t size, const bool incrementSrc, const bool blocking)
{
	ili9488_status_t status = eILI9488_OK;
	g_constData = *p_data;

	// USER CODE BEGIN...

	// wait if previous transfer is not finished
	ili9488_if_wait_for_ready();

	// start DMA transfer
	channel_config_set_read_increment(&g_dmaConfig, incrementSrc);
	channel_config_set_transfer_data_size(&g_dmaConfig, DMA_SIZE_16);
	dma_channel_configure(g_dmaChannel, &g_dmaConfig,
                          &spi_get_hw(eGPIO_SPI)->dr,		// write address
                          incrementSrc?p_data:(const uint16_t*)&g_constData,	// read address
                          size, 							// element count
                          true                   			// start immediately
    );

	ili9488_if_pause_dma();
	ili9488_if_set_cs( true );
	ili9488_if_set_cs( false );
	ili9488_if_resume_dma();
	

	// wait if transfer is not finished for blocking mode
	if ( true == blocking )
	{
		ili9488_if_wait_for_ready();
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Transmit bytes via SPI - 8bit mode
*
* @param[in]	p_data	- Pointer to transmit data
* @param[in]	size 	- Number of data to transmit
* @return		status 	- Status of transmittion
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_if_spi_transmit_8b(const uint8_t * p_data, const uint32_t size)
{
	ili9488_status_t status = eILI9488_OK;

	// USER CODE BEGIN...

	if ( size != spi_write_blocking(eGPIO_SPI, (uint8_t*) p_data, size ))
	{
		status = eILI9488_ERROR_SPI;
	}

	// USER CODE END...

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Receive bytes via SPI
*
* @param[in]	p_data	- Pointer to received data
* @param[in]	size 	- Number of data to receive
* @return		status 	- Status of reception
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_if_spi_receive(uint16_t * p_data, const uint32_t size)
{
	ili9488_status_t status = eILI9488_OK;

	// USER CODE BEGIN...

	if ( size != spi_read16_blocking( eGPIO_SPI, 0, p_data, size ))
	{
		status = eILI9488_ERROR_SPI;
	}

	// USER CODE END...

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		Receive bytes via SPI - 8bit mode
*
* @param[in]	p_data	- Pointer to received data
* @param[in]	size 	- Number of data to receive
* @return		status 	- Status of reception
*/
////////////////////////////////////////////////////////////////////////////////
ili9488_status_t ili9488_if_spi_receive_8b(uint8_t * p_data, const uint32_t size)
{
	ili9488_status_t status = eILI9488_OK;

	// USER CODE BEGIN...

	if ( size != spi_read_blocking( eGPIO_SPI, 0, p_data, size ))
	{
		status = eILI9488_ERROR_SPI;
	}

	// USER CODE END...

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*		DMA initialization
*
*/

ili9488_status_t ili9488_if_dma_init(void)
{
	ili9488_status_t status = eILI9488_OK;

	// USER CODE BEGIN...
	// Set up the DMA channels
    g_dmaChannel = dma_claim_unused_channel(true);

    // Set up the DMA control block
    g_dmaConfig = dma_channel_get_default_config(g_dmaChannel);
    channel_config_set_transfer_data_size(&g_dmaConfig, DMA_SIZE_16);
    channel_config_set_read_increment(&g_dmaConfig, false);
    channel_config_set_write_increment(&g_dmaConfig, false);
    channel_config_set_dreq(&g_dmaConfig, spi_get_dreq(eGPIO_SPI, true));

	return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  	  Checks if transaction is in progress
 * 
 * @return  true  - Transaction in progress
 * @return  false - Transaction not in progress
 */
////////////////////////////////////////////////////////////////////////////////
bool ili9488_if_dma_busy( void )
{
	return dma_channel_is_busy(g_dmaChannel) || spi_is_busy(eGPIO_SPI);
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  	  Waits until no transaction is in progress
 */
////////////////////////////////////////////////////////////////////////////////
void ili9488_if_wait_for_ready( void )
{
	while (ili9488_if_dma_busy())
	{
		tight_loop_contents();
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * 	  Pauses DMA transfer
 */
////////////////////////////////////////////////////////////////////////////////
void ili9488_if_pause_dma( void )
{
	volatile uint32_t dummy;
	dma_hw->ch[g_dmaChannel].ctrl_trig &= ~0x1;
	while(spi_is_busy(eGPIO_SPI)){};
	while(spi_is_readable(eGPIO_SPI))
	{
		dummy = spi_get_hw(eGPIO_SPI)->dr;
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * 	  Resumes DMA transfer
 * 		But only when DMA isn't finished, otherwise it would restart 
 * 		the previous transfer
 */
////////////////////////////////////////////////////////////////////////////////
void ili9488_if_resume_dma( void )
{
	if(dma_channel_is_busy(g_dmaChannel))
	{
		dma_hw->ch[g_dmaChannel].ctrl_trig |= 0x1;
	}
	return;
}


////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
