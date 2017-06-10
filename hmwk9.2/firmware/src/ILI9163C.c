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
#include "ILI9163C.h"
#include <math.h>

void SPI1_init() {
    SDI1Rbits.SDI1R = 0b0100; // B8 is SDI1
    ANSELBbits.ANSB13 = 0;
    RPB13Rbits.RPB13R = 0b0011; //Set up Pin B13 to be SDO1
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
    while (!SPI1STATbits.SPIRBF) { // wait to receive the byte
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
    spi_io(dat >> 8);
    spi_io(dat);
    LATBbits.LATB7 = 1; // CS
}

void LCD_init() {
    int time = 0;
    LCD_command(CMD_SWRESET); //software reset
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000 / 2 / 2) {
    } //delay(500);

    LCD_command(CMD_SLPOUT); //exit sleep
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000 / 2 / 200) {
    } //delay(5);

    LCD_command(CMD_PIXFMT); //Set Color Format 16bit
    LCD_data(0x05);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000 / 2 / 200) {
    } //delay(5);

    LCD_command(CMD_GAMMASET); //default gamma curve 3
    LCD_data(0x04); //0x04
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000 / 2 / 1000) {
    } //delay(1);

    LCD_command(CMD_GAMRSEL); //Enable Gamma adj
    LCD_data(0x01);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000 / 2 / 1000) {
    } //delay(1);

    LCD_command(CMD_NORML);

    LCD_command(CMD_DFUNCTR);
    LCD_data(0b11111111);
    LCD_data(0b00000110);

    int i = 0;
    LCD_command(CMD_PGAMMAC); //Positive Gamma Correction Setting
    for (i = 0; i < 15; i++) {
        LCD_data(pGammaSet[i]);
    }

    LCD_command(CMD_NGAMMAC); //Negative Gamma Correction Setting
    for (i = 0; i < 15; i++) {
        LCD_data(nGammaSet[i]);
    }

    LCD_command(CMD_FRMCTR1); //Frame Rate Control (In normal mode/Full colors)
    LCD_data(0x08); //0x0C//0x08
    LCD_data(0x02); //0x14//0x08
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000 / 2 / 1000) {
    } //delay(1);

    LCD_command(CMD_DINVCTR); //display inversion
    LCD_data(0x07);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000 / 2 / 1000) {
    } //delay(1);

    LCD_command(CMD_PWCTR1); //Set VRH1[4:0] & VC[2:0] for VCI1 & GVDD
    LCD_data(0x0A); //4.30 - 0x0A
    LCD_data(0x02); //0x05
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000 / 2 / 1000) {
    } //delay(1);

    LCD_command(CMD_PWCTR2); //Set BT[2:0] for AVDD & VCL & VGH & VGL
    LCD_data(0x02);
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000 / 2 / 1000) {
    } //delay(1);

    LCD_command(CMD_VCOMCTR1); //Set VMH[6:0] & VML[6:0] for VOMH & VCOML
    LCD_data(0x50); //0x50
    LCD_data(99); //0x5b
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000 / 2 / 1000) {
    } //delay(1);

    LCD_command(CMD_VCOMOFFS);
    LCD_data(0); //0x40
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000 / 2 / 1000) {
    } //delay(1);

    LCD_command(CMD_CLMADRS); //Set Column Address
    LCD_data16(0x00);
    LCD_data16(_GRAMWIDTH);

    LCD_command(CMD_PGEADRS); //Set Page Address
    LCD_data16(0x00);
    LCD_data16(_GRAMHEIGH);

    LCD_command(CMD_VSCLLDEF);
    LCD_data16(0); // __OFFSET
    LCD_data16(_GRAMHEIGH); // _GRAMHEIGH - __OFFSET
    LCD_data16(0);

    LCD_command(CMD_MADCTL); // rotation
    LCD_data(0b00001000); // bit 3 0 for RGB, 1 for GBR, rotation: 0b00001000, 0b01101000, 0b11001000, 0b10101000

    LCD_command(CMD_DISPON); //display ON
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000 / 2 / 1000) {
    } //delay(1);

    LCD_command(CMD_RAMWR); //Memory Write
}

void LCD_drawPixel(unsigned short x, unsigned short y, unsigned short color) {
    // check boundary
    LCD_setAddr(x, y, x + 1, y + 1);
    LCD_data16(color);
}

void LCD_setAddr(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1) {
    LCD_command(CMD_CLMADRS); // Column
    LCD_data16(x0); //Set Start Column
    LCD_data16(x1); //Set End Column

    LCD_command(CMD_PGEADRS); // Page or Row
    LCD_data16(y0); //Set Start Row
    LCD_data16(y1); //Set End Row

    LCD_command(CMD_RAMWR); //Into RAM
}

void LCD_clearScreen(unsigned short color) {
    int i;
    LCD_setAddr(0, 0, _GRAMWIDTH, _GRAMHEIGH);
    for (i = 0; i < _GRAMSIZE; i++) {
        LCD_data16(color);
    }
}

//c1 color is the color of the text
//c2 color is the color of the background
//Screen is slightly smaller. Try to keep text between 2-120
void LCD_char(char character, unsigned short x0, unsigned short y0, unsigned short c1, unsigned short c2) {
    
    int i;
    for (i = 0; i < 5; i++)
    {
        if ((x0 + i) < 128 && x0 > 0)
        {
            LCD_drawByte(ASCII[character - 0x20][i], x0 + i, y0, c1, c2);
        }
    }
}

void LCD_drawByte(unsigned int byte, unsigned short x0, unsigned short y0, unsigned short c1, unsigned short c2) {
    int i;
    for(i = 0; i < 8; i++)
    {
        if(y0 + i < 128 && y0 > 0)
        {
            char fill = (byte >> i)&1;
            if(fill == 0) // Bit location is 0, therefore draw background color.
            {
                LCD_drawPixel(x0, y0 + i, c2);
            }
            else // Bit location is 1, therefore draw text color.
            {
                LCD_drawPixel(x0, y0 + i, c1);
            }            
        }
    }
}

void LCD_string(char* string, unsigned short x0, unsigned short y0, unsigned short c1, unsigned short c2)
{
    int i = 0;
    int position = x0;
    while(string[i] != 0)
    {
        LCD_char(string[i], position, y0, c1, c2);
        i++;
        position+=5;
    }
    
}
/*
 * length can be positive or negative, a negative value would be in the negative x direction
 * width can be positive or negative, a negative value would be in the positive y direction
 */

void LCD_barX(unsigned short x0, unsigned short y0, int length, int width, int max, unsigned short c1, unsigned short c2)
{
    int xDir = (length >= 0) ? 1 : -1;
    int yDir = (width >= 0) ? 1 : -1;
    int i, j;
    unsigned short x , y;
    if(length*xDir > max)
    {
        length = max * xDir;
    }
    int absLength = length * xDir;
    int absWidth = width * yDir;
    for(i = 0; i < absLength; i++)
    {
        x = x0 + i*xDir;
        for(j = 0; j<absWidth; j++)
        {
            y = y0 + j*yDir;
            LCD_drawPixel(x, y, c1);
        }
    }
    
    for(i = absLength; i < max; i++)
    {
        x = x0 + i*xDir;
        for(j = 0; j<absWidth; j++)
        {
            y = y0 + j*yDir;
            LCD_drawPixel(x, y, c2);
        }
    }
}

void LCD_barY(unsigned short x0, unsigned short y0, int length, int width, int max, unsigned short c1, unsigned short c2)
{
    int xDir = (length >= 0) ? 1 : -1;
    int yDir = (width >= 0) ? 1 : -1;
    int i, j;
    unsigned short x , y;
    if(length*xDir > max)
    {
        length = max * xDir;
    }
    int absLength = length * xDir;
    int absWidth = width * yDir;
    for(i = 0; i < absLength; i++)
    {
        x = x0 + i*xDir;
        for(j = 0; j<absWidth; j++)
        {
            y = y0 + j*yDir;
            LCD_drawPixel(y, x, c1);
        }
    }
    
    for(i = absLength; i < max; i++)
    {
        x = x0 + i*xDir;
        for(j = 0; j<absWidth; j++)
        {
            y = y0 + j*yDir;
            LCD_drawPixel(y, x, c2);
        }
    }
}