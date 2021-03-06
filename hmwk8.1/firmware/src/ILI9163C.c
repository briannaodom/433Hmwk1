// functions to operate the ILI9163C on the PIC32
// adapted from https://github.com/sumotoy/TFT_ILI9163C/blob/master/TFT_ILI9163C.cpp

// pin connections:
// VCC - 3.3V
// GND - GND
// CS - B7
// RESET - 3.3V
// A0 - B15
// SDA - A1
// SCK - B14
// LED - 3.3V

// B8 is turned into SDI1 but is not used or connected to anything

#include <xc.h>
#include "ILI9163C2.h"

void SPI1_init() {
	SDI1Rbits.SDI1R = 0b0100; // B8 is SDI1
    ANSELAbits.ANSA1 = 0;
    RPA1Rbits.RPA1R = 0b0011; // A1 is SDO1
    TRISBbits.TRISB7 = 0; // SS is B7
    LATBbits.LATB7 = 1; // SS starts high

    // A0 / DAT pin
    ANSELBbits.ANSB15 = 0;
    TRISBbits.TRISB15 = 0;
    LATBbits.LATB15 = 0;
	
	SPI1CON = 0; // turn off the spi module and reset it
    SPI1BUF; // clear the rx buffer by reading from it
    SPI1BRG = 1; // baud rate to 12 MHz [SPI1BRG = (48000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0; // clear the overflow bit
    SPI1CONbits.CKE = 1; // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1; // master operation
    SPI1CONbits.ON = 1; // turn on spi1
}

unsigned char spi_io(unsigned char o) {
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}

void LCD_command(unsigned char com) {
    LATBbits.LATB15 = 0; // DAT
    LATBbits.LATB7 = 0; // CS
    spi_io(com);
    LATBbits.LATB7 = 1; // CS
}

void LCD_data(unsigned char dat) {
    LATBbits.LATB15 = 1; // DAT
    LATBbits.LATB7 = 0; // CS
    spi_io(dat);
    LATBbits.LATB7 = 1; // CS
}

void LCD_data16(unsigned short dat) {
    LATBbits.LATB15 = 1; // DAT
    LATBbits.LATB7 = 0; // CS
    spi_io(dat>>8);
    spi_io(dat);
    LATBbits.LATB7 = 1; // CS
}

void LCD_init() {
    int time = 0;
    LCD_command(CMD_SWRESET);//software reset
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000/2/2) {} //delay(500);

	LCD_command(CMD_SLPOUT);//exit sleep
    time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/200) {} //delay(5);

	LCD_command(CMD_PIXFMT);//Set Color Format 16bit
	LCD_data(0x05);
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/200) {} //delay(5);

	LCD_command(CMD_GAMMASET);//default gamma curve 3
	LCD_data(0x04);//0x04
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_GAMRSEL);//Enable Gamma adj
	LCD_data(0x01);
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_NORML);

	LCD_command(CMD_DFUNCTR);
	LCD_data(0b11111111);
	LCD_data(0b00000110);

    int i = 0;
	LCD_command(CMD_PGAMMAC);//Positive Gamma Correction Setting
	for (i=0;i<15;i++){
		LCD_data(pGammaSet[i]);
	}

	LCD_command(CMD_NGAMMAC);//Negative Gamma Correction Setting
	for (i=0;i<15;i++){
		LCD_data(nGammaSet[i]);
	}

	LCD_command(CMD_FRMCTR1);//Frame Rate Control (In normal mode/Full colors)
	LCD_data(0x08);//0x0C//0x08
	LCD_data(0x02);//0x14//0x08
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_DINVCTR);//display inversion
	LCD_data(0x07);
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_PWCTR1);//Set VRH1[4:0] & VC[2:0] for VCI1 & GVDD
	LCD_data(0x0A);//4.30 - 0x0A
	LCD_data(0x02);//0x05
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_PWCTR2);//Set BT[2:0] for AVDD & VCL & VGH & VGL
	LCD_data(0x02);
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_VCOMCTR1);//Set VMH[6:0] & VML[6:0] for VOMH & VCOML
	LCD_data(0x50);//0x50
	LCD_data(99);//0x5b
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_VCOMOFFS);
	LCD_data(0);//0x40
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_CLMADRS);//Set Column Address
	LCD_data16(0x00);
    LCD_data16(_GRAMWIDTH);

	LCD_command(CMD_PGEADRS);//Set Page Address
	LCD_data16(0x00);
    LCD_data16(_GRAMHEIGH);

	LCD_command(CMD_VSCLLDEF);
	LCD_data16(0); // __OFFSET
	LCD_data16(_GRAMHEIGH); // _GRAMHEIGH - __OFFSET
	LCD_data16(0);

	LCD_command(CMD_MADCTL); // rotation
    LCD_data(0b00001000); // bit 3 0 for RGB, 1 for GBR, rotation: 0b00001000, 0b01101000, 0b11001000, 0b10101000

	LCD_command(CMD_DISPON);//display ON
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_RAMWR);//Memory Write
}

void LCD_drawPixel(unsigned short x, unsigned short y, unsigned short color) {
    // check boundary
    LCD_setAddr(x,y,x+1,y+1);
    LCD_data16(color);
}

void LCD_setAddr(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1) {
    LCD_command(CMD_CLMADRS); // Column
    LCD_data16(x0);
	LCD_data16(x1);

	LCD_command(CMD_PGEADRS); // Page
	LCD_data16(y0);
	LCD_data16(y1);

	LCD_command(CMD_RAMWR); //Into RAM
}

void LCD_clearScreen(unsigned short color) {
    int i;
    LCD_setAddr(0,0,_GRAMWIDTH,_GRAMHEIGH);
		for (i = 0;i < _GRAMSIZE; i++){
			LCD_data16(color);
		}
}

void LCD_writeByte(unsigned short byte, unsigned short x, unsigned short y, unsigned short color) {
    int i;
    for (i = 0;i < 8; i++) {
        if (y < 128 && y > 8) {
            //first we need to grab the bit 
            unsigned short pixel = (byte >> i) & 1;
            if (!pixel) {//then the location is empty
                LCD_drawPixel(x,y - i,BACKGROUND);           
            }
            else {
                LCD_drawPixel(x,y - i,color);
            }
        }
        
    }
}

void LCD_writeChar(char letter, unsigned short x, unsigned short y, unsigned short color) {
    //each letter takes up 5 spaces
    int i;
    for (i = 0;i < 5; i++) {
        if (x > 5 && x < 128) {
            LCD_writeByte(ASCII[letter - 0x20][i],x - i,y,color);            
        }        
    }
    
}

void LCD_writeString(char *inputStr, unsigned short x, unsigned short y, unsigned short color) {
    int counter = 0;
    while(inputStr[counter] != 0) {
        LCD_writeChar(inputStr[counter], (x - 5*counter - counter), y, color);
        counter ++;
    }
}

void LCD_drawBar(int number, unsigned short x, unsigned short y, unsigned short color,int dir) {
    //Let's make the bar height 4 pixels
    unsigned short height = 0x0F;
    unsigned short ydir1 = 1;
    unsigned short ydir2 = 0x80;
    int i, j;    
    int magnitude = ((int) (number/2));   
    if (dir == XDIR) { //direction being called is the X direction
        if (magnitude > 0) {
            for (i = 0;i <= 100; i++) {
                if (i <= magnitude) {
                    LCD_writeByte(height,x + i, y, color);
                }
                else 
                {
                    LCD_writeByte(0x00, x + i, y, color);
                }
            }
        }
        else {
            for (i = 0;i <= 100; i++) {
                if (i <= -magnitude) {
                    LCD_writeByte(height,x - i, y, color);
                }
                else 
                {
                    LCD_writeByte(0x00, x - i, y, color);
                }
            }
            
        }
    }
    else {
        if (magnitude > 0) {
        for (i = 0;i <= 100; i++) {
            if (i <= magnitude) {
                    for (j = 0; j < 4; j++) {
                        LCD_writeByte(ydir2,x - j, y + i, color);
                    }
                }
                else 
                {   
                    for(j = 0; j < 4; j++) {
                        LCD_writeByte(0x00, x - j, y + i, color);
                    }
                    
                } 
            }
        }
        else {
            for (i = 0;i <= 100; i++) 
            {
                if (i <= -magnitude) 
                {
                    for (j = 0; j < 4; j++) {
                        LCD_writeByte(ydir1,x - j, y - i, color);
                    }
                }
                else 
                {   
                    for(j = 0; j < 4; j++) {
                        LCD_writeByte(0x00, x - j, y - i, color);
                    }
                    
                } 
            }
        }                    
    }
}