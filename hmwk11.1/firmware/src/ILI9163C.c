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
#include <stdio.h>
#include "ILI9163C.h"

void SPI1_init() {
//	SDI1Rbits.SDI1R = 0b0100; // B8 is SDI1 (Nick's)
    SDI1Rbits.SDI1R = 0b0001; // B5 is SDI1 
//    RPA1Rbits.RPA1R = 0b0011; // A1 is SDO1 (Nick's))
    RPB8Rbits.RPB8R = 0b0011; // RPB8 (pin 16) is SDO1
    TRISBbits.TRISB7 = 0; // SS is B7
    LATBbits.LATB7 = 1; // SS starts high

    // A0 / DAT pin
    ANSELBbits.ANSB15 = 0;
    TRISBbits.TRISB15 = 0;
    LATBbits.LATB15 = 0;
//    A0 = 0;
	
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

void draw_character(unsigned short x, unsigned short y, char ch, unsigned short t_color, unsigned short bg_color){
    // check boundary
    int i,j,k;
    char dummy = ch - 0x20;
//    int dummy = (int)(ch - 0x20);
		for (i = 0; i < 5; i++){
            if (ch < 128){
                for (j = 0; j < 8; j++){
                    if ((ASCII[dummy][i] >> j) & 1){
                        LCD_drawPixel(x+i,y+j,t_color);                       
                    } 
                    else{
                        LCD_drawPixel(x+i,y+j,bg_color);                           
                    }
                }
            }
        }
}

void draw_string(unsigned short x, unsigned short y, unsigned char *msg, unsigned short t_color, unsigned short bg_color){
    int i = 0, j = 0;
    while(msg[i] != 0){
        draw_character(x+j, y, msg[i], t_color, bg_color);
        i++;
        j = j + 5;
    }
    
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

//void draw_bar_x_accel(int x_accel, int bar_depth, unsigned short t_color, unsigned short bg_color){
//
//    int max_abs_accel = 10;
//    int x_org = (128/2)-50; // start column of progress bar
//    int y_org = (128/2)-50;    
//    int clmn_px = 0;
//    int k = 0;
//    if (x_accel >= 0){
//        clmn_px = x_org + (int)(x_accel*0.5);
//        for(k = 0; k < bar_depth; k++){
//            LCD_drawPixel(clmn_px, y_org-(bar_depth/2)+k, t_color);              
//        }        
//    }
//    else{
//        clmn_px = x_org - (int)(x_accel*0.5);  
//        for(k = 0; k < bar_depth; k++){
//            LCD_drawPixel(clmn_px, y_org-(bar_depth/2)+k, t_color);              
//        }  
//    }
//
//}

void draw_bar_accel(float x_l_acc, float y_l_acc, int bar_depth,unsigned short t_color, unsigned short bg_color){
    // draw x line from the origin of the screen
    int x_org = 128/2; // start column of progress bar
    int y_org = 128/2;
    int max_g = 10;
    int i,k;
    // 5 pixels per 1 m/s^2
    // x accel line
    if (x_l_acc >= 0) {
        for (i = x_org; i < 50 + x_org; i++){
            for (k = 0; k < bar_depth; k++){
                if (i < (int)x_l_acc*5 + x_org){
                    LCD_drawPixel(i , y_org + (bar_depth/2)+k, t_color);                        
                }
                else {
                    LCD_drawPixel(i , y_org + (bar_depth/2)+k, bg_color);                     
                }
            }
        }
    }
    else {
        for (i = x_org; i > x_org - 50; i--){
            for (k = 0; k < bar_depth; k++){
                if (i > x_org + (int)x_l_acc*5){
                    LCD_drawPixel(i , y_org + (bar_depth/2)+k, t_color);                        
                }
                else {
                    LCD_drawPixel(i , y_org + (bar_depth/2)+k, bg_color);                     
                }             
            }            
        }
    }
    // y accel line 
    if (y_l_acc >= 0) {
        for (i = y_org; i < 50 + y_org; i++){
            for (k = 0; k < bar_depth; k++){
                if (i < (int)y_l_acc*5 + y_org){
                    LCD_drawPixel(x_org + (bar_depth/2)+k, i, t_color);                        
                }
                else {
                    LCD_drawPixel(x_org + (bar_depth/2)+k, i, bg_color);                     
                }
            }
        }
    }
    else {
        for (i = y_org; i > y_org - 50; i--){
            for (k = 0; k < bar_depth; k++){
                if (i > (int)y_l_acc*5 + y_org){
                    LCD_drawPixel(x_org + (bar_depth/2)+k, i, t_color);                        
                }
                else {
                    LCD_drawPixel(x_org + (bar_depth/2)+k, i, bg_color);                     
                }             
            }            
        }
    }    
}

void draw_fps(unsigned short x, unsigned short y, int counter, unsigned short t_color, unsigned short bg_color){
    unsigned char msg[45];   
    sprintf(msg, "FPS: %.2f   ", 24000000.0/counter);
    draw_string(x, y + 70, msg, t_color, bg_color);
}

void LCD_clearScreen(unsigned short color) {
    int i;
    LCD_setAddr(0,0,_GRAMWIDTH,_GRAMHEIGH);
		for (i = 0;i < _GRAMSIZE; i++){
			LCD_data16(color);
		}
}