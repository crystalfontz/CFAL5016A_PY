//===========================================================================
//
//
//  CRYSTALFONTZ CFAL5016A-PY
//
//  This code communicates via 3-wire SPI
//
//	The module is a CFAL5016A-PY, a serial version of the CFAL5016A-Y
//	  https://www.crystalfontz.com/product/cfal5016apy
//
//	Non serial version (4-bit & 8-bit parallel) can be found here:
//	  https://www.crystalfontz.com/product/cfal5016ay
//
//  The controller is a Winstar WS0010
//    https://www.crystalfontz.com/controllers/WinstarDisplay/WS0010/
//
//  Powered using a Seeeduino v4.2, an open-source 3.3v & 5v capable Arduino clone.
//    https://www.seeedstudio.com/Seeeduino-V4.2-p-2517.html
//    https://github.com/SeeedDocument/SeeeduinoV4/raw/master/resources/Seeeduino_v4.2_sch.pdf
//============================================================================
//
//
//
//===========================================================================
//This is free and unencumbered software released into the public domain.
//
//Anyone is free to copy, modify, publish, use, compile, sell, or
//distribute this software, either in source code form or as a compiled
//binary, for any purpose, commercial or non-commercial, and by any
//means.
//
//In jurisdictions that recognize copyright laws, the author or authors
//of this software dedicate any and all copyright interest in the
//software to the public domain. We make this dedication for the benefit
//of the public at large and to the detriment of our heirs and
//successors. We intend this dedication to be an overt act of
//relinquishment in perpetuity of all present and future rights to this
//software under copyright law.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
//OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//OTHER DEALINGS IN THE SOFTWARE.
//
//For more information, please refer to <http://unlicense.org/>
//============================================================================



//============================================================================
// LCD & USD control lines
//   ARD      | Port | CFA5016A-PY |  Function					   | Wire
//------------+------+-------------+--------------------------------+------------
//  5V	      |		 | #2          |  POWER 5V					   | Red
//  GND	      |		 | #1          |  GROUND					   | Black
// -----------+------+-------------+--------------------------------+------------
// #10/D10    |  PB2 | #15         |  Slave Select			  (SS) | Purple
// #11/D11    |  PB3 | #14         |  Master Out Slave In	(MOSI) | Grey
// #12/D12    |  PB4 | #13         |  Master In Slave Out	(MISO) | Brown
// #13/D13    |  PB5 | #12         |  Clock					 (SCK) | Yellow
// -----------+------+-------------+--------------------------+------------
//============================================================================

#define SS		PORTB2
#define MOSI	PORTB3
#define MISO	PORTB4
#define CLK		PORTB5			

#define sendCommand(cmd)	soft_spi_send_byte(0,0,cmd)
#define sendData(data)		soft_spi_send_byte(1,0,data)


uint8_t pointer = 0;

const unsigned char crystalfontz_text[100] = { 
	0X00,0X00,0X58,0X94,0X94,0X64,0X00,0XFC,0X84,0X84,0XFD,0X00,0X88,0X70,0X88,0X00,
	0X00,0X08,0XFC,0X00,0X00,0X78,0X94,0X94,0X64,0X00,0X00,0X00,0X78,0X84,0X84,0X84,
	0X78,0X00,0X00,0XFC,0X80,0X80,0X00,0XFC,0X94,0X94,0X94,0X00,0XFC,0X84,0X84,0X78,
	0X00,0X00,0X3E,0X41,0X41,0X41,0X22,0X00,0X7C,0X04,0X04,0X04,0X38,0XC0,0X38,0X04,
	0X00,0X48,0X54,0X54,0X24,0X04,0X7E,0X44,0X00,0X64,0X54,0X54,0X7C,0X00,0X7F,0X00,
	0X04,0X7E,0X05,0X00,0X38,0X44,0X44,0X38,0X00,0X7C,0X04,0X04,0X78,0X04,0X7E,0X44,
	0X00,0X64,0X54,0X4C, };

const unsigned char gImage_50x16_logo[100] = { 
	0X00,0X00,0X00,0X00,0X0F,0X0F,0X2F,0X6F,0XCF,0X8F,0XCF,0X40,0X1F,0X3F,0X7F,0XC0,
	0X8A,0XCA,0X6A,0X2A,0X0A,0X0A,0X0A,0X00,0X00,0X00,0X00,0X0F,0X0F,0X2F,0X6F,0XCF,
	0X8F,0XCF,0X40,0X1F,0X3F,0X7F,0XC0,0X8A,0XCA,0X6A,0X2A,0X0A,0X0A,0X0A,0X00,0X00,
	0X00,0X00,0X00,0X00,0X00,0X00,0XE0,0XE0,0XE8,0XEC,0XE6,0XE2,0XE6,0X04,0XF0,0XF8,
	0XFC,0X06,0XA2,0XA6,0XAC,0XA8,0XA0,0XA0,0XA0,0X00,0X00,0X00,0X00,0XE0,0XE0,0XE8,
	0XEC,0XE6,0XE2,0XE6,0X04,0XF0,0XF8,0XFC,0X06,0XA2,0XA6,0XAC,0XA8,0XA0,0XA0,0XA0,
	0X00,0X00,0X00,0X00, };


//----------------------------------------------------------------------------
void writeString(char* myString)
{
	uint8_t i = 0;
	//Send data until there is no more data to be sent
	do
	{
		sendData((uint8_t)myString[i]);
		i++;
		setPointer(pointer, pointer + 1);
	} while (myString[i] != NULL);
}

//----------------------------------------------------------------------------
void setCursor(uint8_t column, uint8_t row)
{
	//Since the driver can go up to 64 columns, we need to calculate
	//where the cursor should go based on our 10x2 character display
	uint8_t	newPoint = (row * 64) + column;
	//Use the checkPointer function to calculate where to put the cursor
	setPointer(50, newPoint);
}

//----------------------------------------------------------------------------
void setPointer(uint8_t oldPoint, uint8_t newPoint)
{
	//Lets first check to see if the new pointer is in an allowable position
	if (newPoint < 10 || (newPoint > 63 && newPoint < 74))
	{
		pointer = newPoint;
		//if it is on the screen, check to see if we're just doing a standard increment
		//if it is not a standard increment, send the command to set the cursor (DDRAM Address)
		//to the new location
		if (newPoint != oldPoint + 1)
		{
			sendCommand(0x80 | newPoint);
		}
		//if it is, we preset the driver to autoincrement so no command needs to be sent
	}
	//check to see if the new point is outside the bounds of the first line
	else if (newPoint > 9 && newPoint < 64)
	{
		//if it is, send it to the beginning of the second line
		sendCommand(0xC0);
		pointer = 64;
	}
	//check to see if the cursor is outside the bounds of the second line
	else if (newPoint > 73)
	{
		//if it is, send it to the beginning of the first line
		sendCommand(0x80);
		pointer = 0;
	}
}

//----------------------------------------------------------------------------
void soft_spi_send_byte(char cmd, char write, char data)
{
	// The SPI.transfer function sends a byte at a time and since we need to send 10 bits we need a work around
	// In order to send 10 bits of data instead of just a byte, we need to bitbang the controller
	// We are going to hard code the pins to be high or low using assembler instructions, the asm function

	// enable chip_sel
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (SS));		// set chip select
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (SS));		// clear chip select

	// Send the first bit, the data/command flag bit
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
	asm("sbrc %0, 0" : : "a" (cmd));								//	check if the data should be set or not
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

	// Send the second bit, the write flag bit
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
	asm("sbrc %0, 0" : : "a" (write));								//	check if the data should be set or not
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock


	// Now send all the data bits
	// Send Bit 7 of data
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
	asm("sbrc %0, 7" : : "a" (data));								//	check if the data should be set or not
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

	// Send Bit 6 of data
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
	asm("sbrc %0, 6" : : "a" (data));								//	check if the data should be set or not
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

	// Send bit 5 of data
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
	asm("sbrc %0, 5" : : "a" (data));								//	check if the data should be set or not
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

	// Send bit 4 of data
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
	asm("sbrc %0, 4" : : "a" (data));								//	check if the data should be set or not
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

	// Send bit 3 of data
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
	asm("sbrc %0, 3" : : "a" (data));								//	check if the data should be set or not
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

	// Send bit 2 of data
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
	asm("sbrc %0, 2" : : "a" (data));								//	check if the data should be set or not
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

	// Send bit 1 of data
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
	asm("sbrc %0, 1" : : "a" (data));								//	check if the data should be set or not
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

	// Send bit 0 of data
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
	asm("sbrc %0, 0" : : "a" (data));								//	check if the data should be set or not
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock


	// disable chip_sel
	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (SS));
	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));	// clear the clock

	//certain commands need some time to think so lets give it some time
	//this can be removed for just data sending and some commands
	delayMicroseconds(350);
}



void init_1602b()
{
	delay(500);


	//The following block is the power on sequence. Each time the module is power
	//cycled, this is the code that is initiated internally.
	//This is for reference but since we are switching between graphic and character
	//mode, we do need to cycle the display power on and off for a refresh

	//------------------------------------------------------------------------
	//Start Power On Sequence
	//Display clear
	sendCommand(0x01);

	//Function Set
	sendCommand(0x30);	//00110000
		//	0011XXXX
		//	    ||||---	Font Table Selection
		// 	    ||		>>00: English_Japenese character font table
		//		||		  01: Western European character font table - I
		//		||		  10: English_Russian character font table
		//		||		  11: Western European character font table - II
		//		||-----	Character Font Set
		//		|		>>0: 5x8 dot character font
		//		|		  1: 5x10 dot character font
		//		|------	Interface Data Length Control Bit	
		//				>>0: data is sent or received in 4-bit length DB4-DB7
		//				  1: data is sent or received in 8-bit length DB4-DB7


	//Cursor / Display Shift Instruction (2 parts)
	//only sending second part
	sendCommand(0x13);	//00010011
		//	0001XX11
		//		||----	Enable / Disable Internal Power
		//		|		>>0: Internal Power OFF
		//		|	 	  1: Internal Power ON
		//		|-----	Graphic Mode / Character Mode Selection
		//				>>0: Character Mode
		//				  1: Graphic Mode

	//Display ON/OFF Control
	sendCommand(0x08);	//00001000	
		//	00001XXX
		//		 |||--	Blinking Control Bit
		//		 ||		  1: Cursor Blinking ON
		//		 ||		>>0: Cursor Blinking OFF
		//		 ||---	Cursor Display Control
		//		 |		>>0: Cursor OFF
		//		 |		  1: Cursor ON
		//		 |----	Display ON/OFF
		//		  		>>0: Display OFF
		//		  		  1: Display ON

	//Entry Mode Set
	sendCommand(0x04); //00000100
	   //	000001XX
	   //		  ||--	Shift Entire Display Control Bit
	   //		  |		>>0: Decrement DDRAM Address by 1 when a character
	   //		  |			 code is written into or read from DDRAM
	   //		  |		  1: Increment DDRAM Address by 1 when a character
	   //		  |			 code is written into or read from DDRAM
	   //		  |---	Increment/Decrement bit
	   //		  		>>0: when writing to DDRAM, each
	   //		  			 entry moves the cursor to the left
	   //			 	  1: when writing to DDRAM, each 
	   //		  			 entry moves the cursor to the right

	//End Power On Sequence
	//------------------------------------------------------------------------



	//Cursor / Display Shift Instruction (2 parts)
	sendCommand(0x14);
		//	0001XX00
		//		||----	Shift Function
		//				  00: Shifts cursor position to the left
		//					  (AC is decremented by 1)
		//				  01: Shifts cuursor position to the right
		//					  (AC is incremented by 1)
		//				>>10: Shifts entire display to the left
		//					  The cursor follows the display shift
		//				  11: Shifts entire display to the right
		//					  The cursor follows the display shift
	sendCommand(0x17);
		//	0001XX11
		//		||----	Enable / Disable Internal Power
		//		|		  0: Internal Power OFF
		//		|	 	>>1: Internal Power ON
		//		|-----	Graphic Mode / Character Mode Selection
		//				>>0: Character Mode
		//				  1: Graphic Mode

	//Function Set
	sendCommand(0x3B);	//00111011
		//	0011XXXX
		//	    ||||---	Font Table Selection
		// 	    ||		  00: English_Japenese character font table
		//		||		  01: Western European character font table - I
		//		||		  10: English_Russian character font table
		//		||		>>11: Western European character font table - II
		//		||-----	Character Font Set
		//		|		>>0: 5x8 dot character font
		//		|		  1: 5x10 dot character font
		//		|------	Interface Data Length Control Bit	
		//				  0: data is sent or received in 4-bit length DB4-DB7
		//				>>1: data is sent or received in 8-bit length DB4-DB7

	//Display ON/OFF Control
	sendCommand(0x0C);	//00001100
		//	00001XXX
		//		 |||--	Blinking Control Bit
		//		 ||		  1: Cursor Blinking ON
		//		 ||		>>0: Cursor Blinking OFF
		//		 ||---	Cursor Display Control
		//		 |		>>0: Cursor OFF
		//		 |		  1: Cursor ON
		//		 |----	Display ON/OFF
		//		  		  0: Display OFF
		//		  		>>1: Display ON

		
	//Entry Mode Set
	sendCommand(0x06); //00000110
		//	000001XX
		//		  ||--	Shift Entire Display Control Bit
		//		  |		>>0: Decrement DDRAM Address by 1 when a character
		//		  |			 code is written into or read from DDRAM
		//		  |		  1: Increment DDRAM Address by 1 when a character
		//		  |			 code is written into or read from DDRAM
		//		  |---	Increment/Decrement bit
		//		  		  0: when writing to DDRAM, each
		//		  			 entry moves the cursor to the left
		//			 	>>1: when writing to DDRAM, each 
		//		  			 entry moves the cursor to the right


	//Return Home 00000010
	sendCommand(0x02);

	//Display Clear 00000001
	sendCommand(0x01);
}

void setup()
{
	delay(500);
	DDRB = 0x2C; //Set pins 10, 11, and 13 as output pins 0010 1100

	init_1602b();
}

void loop()
{
	//set to graphic mode
	sendCommand(0x1F);
	//Start with left-most column (column 0)
	//Set DDRAM to ADD = 0 for X-axis: 10000000
	sendCommand(0x80);
	//Start with the bottom row (row 1)
	//Set CGRAM to ACG = 1 for Y-axis: 01000001
	sendCommand(0x41);
	//Loop through the first 50 bytes in the array
	for (int row = 0; row<50; row++)
	{
		sendData(0xFF);
	}

	//continue with left-most column (column 0)
	//Set DDRAM to ADD = 0 for X-axis: 10000000
	sendCommand(0x80);
	//Continue on the top row (row 0)
	//Set CGRAM to ACG = 0 for Y-axis: 01000000
	sendCommand(0x40);
	//Loop through the second 50 bytes in the array
	for (int row = 50; row<100; row++)
	{
		sendData(0xFF);
	}
	//Pause to see the display
	delay(2000);	



	sendCommand(0x80);
	sendCommand(0x41);
	for (int row = 0; row<50; row++)
	{
		sendData(crystalfontz_text[row]);
	}
	sendCommand(0x80);
	sendCommand(0x40);
	for (int row = 50; row<100; row++)
	{
		sendData(crystalfontz_text[row]);
	}
	delay(2000);



	sendCommand(0x80);
	sendCommand(0x41);
	for (int row = 0; row<50; row++)
	{
		sendData(gImage_50x16_logo[row]);
	}
	sendCommand(0x80);
	sendCommand(0x40);
	for (int row = 50; row<100; row++)
	{
		sendData(gImage_50x16_logo[row]);
	}
	delay(2000);



	//Re-initialize to return to character mode
	init_1602b();
	setCursor(0, 0);
	char* firstLine = "Character ";
	char* secondLine = "+ Graphic ";

	writeString(firstLine);
	writeString(secondLine);


	delay(2000);
}
