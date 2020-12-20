/*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

These displays use SPI to communicate, 4 or 5 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

/*
 *  Modified by Neal Horman 7/14/2012 for use in mbed
 *  Modified by Greg Steiert 2/7/2017 for FeatherOLED
 */

#ifndef _ADAFRUIT_SSD1306_H_
#define _ADAFRUIT_SSD1306_H_

#include "mbed.h"
#include "Adafruit_GFX.h"

#include <vector>
#include <algorithm>

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

/** The pure base class for the SSD1306 display driver.
 *
 * You should derive from this for a new transport interface type,
 * such as the SPI and I2C drivers.
 */
class Adafruit_SSD1306 : public Adafruit_GFX
{
public:
	Adafruit_SSD1306(uint8_t rawHeight = 32, uint8_t rawWidth = 128)
		: Adafruit_GFX(rawWidth,rawHeight)
	{
		buffer.resize(rawHeight * rawWidth / 8);
	};

	void begin(uint8_t switchvcc = SSD1306_SWITCHCAPVCC);
	
	// These must be implemented in the derived transport driver
	virtual void command(uint8_t c) = 0;
	virtual void data(uint8_t c) = 0;
	virtual void drawPixel(int16_t x, int16_t y, uint16_t color);

	/// Clear the display buffer    
	void clearDisplay(void);
	virtual void invertDisplay(bool i);

	/// Cause the display to be updated with the buffer content.
	void display();
	/// Fill the buffer with the AdaFruit splash screen.
	virtual void splash();
    
protected:
	virtual void sendDisplayBuffer() = 0;

	// the memory buffer for the LCD
	std::vector<uint8_t> buffer;
};


/** This is the I2C SSD1306 display driver transport class
 *
 */
class Adafruit_SSD1306_I2c : public Adafruit_SSD1306
{
public:
	#define SSD_I2C_ADDRESS     0x78
	/** Create a SSD1306 I2C transport display driver instance with the specified RST pin name, the I2C address, as well as the display dimensions
	 *
	 * Required parameters
	 * @param i2c - A reference to an initialized I2C object
	 * @param RST - The Reset pin name
	 *
	 * Optional parameters
	 * @param i2cAddress - The i2c address of the display
	 * @param rawHeight - The vertical number of pixels for the display, defaults to 32
	 * @param rawWidth - The horizonal number of pixels for the display, defaults to 128
	 */
	Adafruit_SSD1306_I2c(I2C &i2c, uint8_t i2cAddress = SSD_I2C_ADDRESS, uint8_t rawHeight = 32, uint8_t rawWidth = 128)
	    : Adafruit_SSD1306(rawHeight, rawWidth)
	    , mi2c(i2c)
	    , mi2cAddress(i2cAddress)
	    {
		    begin();
		    splash();
		    display();
	    };

	virtual void command(uint8_t c)
	{
		char buff[2];
		buff[0] = 0; // Command Mode
		buff[1] = c;
		mi2c.write(mi2cAddress, buff, sizeof(buff));
	}

	virtual void data(uint8_t c)
	{
		char buff[2];
		buff[0] = 0x40; // Data Mode
		buff[1] = c;
		mi2c.write(mi2cAddress, buff, sizeof(buff));
	};

protected:
	virtual void sendDisplayBuffer()
	{
		char buff[17];
		buff[0] = 0x40; // Data Mode

		// send display buffer in 16 byte chunks
		for(uint16_t i=0, q=buffer.size(); i<q; i+=16 ) 
		{	uint8_t x ;

			// TODO - this will segfault if buffer.size() % 16 != 0
			for(x=1; x<sizeof(buff); x++) 
				buff[x] = buffer[i+x-1];
			mi2c.write(mi2cAddress, buff, sizeof(buff));
		}
	};

	I2C &mi2c;
	uint8_t mi2cAddress;
};

#endif