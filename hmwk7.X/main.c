
#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<math.h>
#include<stdio.h>   // needed to use sprintf()
#include"configPic.h" // contains DEVCFGx commands
#include"ILI9163C.h" // low-level LCD library
#include"TFTLCD.h" // high-level LCD library
#include"i2c_master_noint.h" // I2C library
#include"LSM6DS33.h" // high-level IMU library

#define DELAYTIME 4000000 // 40000 yields 0.001 s delay time when using Core Timer
#define CS LATBbits.LATB7  // SPI chip select pin
#define STRLEN 19   // maximum number of characters per string

void drawxXLvec(char x, char y, signed char magnitude, short doneColor) { // draw x-vector
    int barX, barY;
    if (magnitude > 0) { // if magnitude is positive
        for (barX=0; barX<50; barX++) {
            if (barX<magnitude) {
                for (barY=0; barY<4; barY++) {
                    LCD_drawPixel(x+barX,y+barY,doneColor);
                }
            }
            else {
                for (barY=0; barY<4; barY++) {
                    LCD_drawPixel(x+barX,y+barY,BACKGROUND);
                }
            }
            for (barY=0; barY<4; barY++) {
                    LCD_drawPixel(x-barX,y+barY,BACKGROUND); // erase the bar in the opposite direction
            }
        }
    }
    else { // if magnitude is 0 or negative
        for (barX=0; barX<50; barX++) {
            if (barX<(-magnitude)) {
                for (barY=0; barY<4; barY++) {
                    LCD_drawPixel(x-barX,y+barY,doneColor);
                }
            }
            else {
                for (barY=0; barY<4; barY++) {
                    LCD_drawPixel(x-barX,y+barY,BACKGROUND);
                }
            }
            for (barY=0; barY<4; barY++) {
                    LCD_drawPixel(x+barX,y+barY,BACKGROUND); // erase the bar in the opposite direction
            }
        }
    }
}

void drawyXLvec(char x, char y, signed char magnitude, short doneColor) { // draw y-vector
    int barX, barY;
    if (magnitude > 0) { // if magnitude is positive
        for (barY=0; barY<50; barY++) {
            if (barY<magnitude) {
                for (barX=0; barX<4; barX++) {
                    LCD_drawPixel(x+barX,y+barY,doneColor);
                }
            }
            else {
                for (barX=0; barX<4; barX++) {
                    LCD_drawPixel(x+barX,y+barY,BACKGROUND);
                }
            }
            for (barX=0; barX<4; barX++) {
                    LCD_drawPixel(x+barX,y-barY,BACKGROUND); // erase the bar in the opposite direction
            }
        }
    }
    else { // if magnitude is 0 or negative
        for (barY=0; barY<50; barY++) {
            if (barY<(-magnitude)) {
                for (barX=0; barX<4; barX++) {
                    LCD_drawPixel(x+barX,y-barY,doneColor);
                }
            }
            else {
                for (barX=0; barX<4; barX++) {
                    LCD_drawPixel(x+barX,y-barY,BACKGROUND);
                }
            }
            for (barX=0; barX<4; barX++) {
                    LCD_drawPixel(x+barX,y+barY,BACKGROUND); // erase the bar in the opposite direction
            }
        }
    }
}

int main() {
    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    // Turn off AN2 and AN3 pins (make B2 and B3 available for I2C)
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA0 = 0; // pin 0 of Port A is CS (chip select) (output)
    TRISAbits.TRISA1 = 1; // pin 1 of Port A is SDO1 (output)
    TRISAbits.TRISA4 = 0; // Pin 4 of Port A is LED1 (output)
    TRISBbits.TRISB4 = 1; // Pin 4 of Port B is USER button (input)
    LATAbits.LATA4 = 1; // Turn LED2 ON
    
    SPI1_init();
    LCD_init();
    LCD_clearScreen(BACKGROUND);
    
    LSM6DS33_init();
    
    unsigned char progress;
    unsigned char outbuf[STRLEN];
    float fps = 1.0;
    unsigned char boxptx, boxpty;
    
    unsigned char iamwhatiam = 0;
    iamwhatiam = WhoAmI();
    
    unsigned char LSMRead[LSM_ARRAY_LEN];

    __builtin_enable_interrupts();

    _CP0_SET_COUNT(0);
    while(1) {
        for (progress=0; progress < 101; progress++) {
            I2C_read_multiple(IMU_ADDR, 0x20, LSMRead, LSM_ARRAY_LEN); // use length = LSM_ARRAY_LEN
            _CP0_SET_COUNT(0);
            
            LCD_drawString(5,61,"X",TEXTCOLOR);
            LCD_drawString(61,5,"Y",TEXTCOLOR);
            
            drawxXLvec(62,62,((signed char) (convxXL(LSMRead)*50)),GREEN);
            drawyXLvec(62,62,((signed char) (convyXL(LSMRead)*50)),RED);
            
            for(boxptx=0;boxptx<4;boxptx++) { // draw a 4x4 white box at the center of the screen
                for(boxpty=0;boxpty<4;boxpty++) {
                    LCD_drawPixel(62+boxptx,62+boxpty,WHITE);
                }
            }
            
            fps = 24000000.0/_CP0_GET_COUNT();

            LATAbits.LATA4 = 1; // turn on LED1; USER button is low (FALSE) if pressed.
            while(!PORTBbits.RB4) { // button is on GP7, so shift 7 bits to the right.
                LATAbits.LATA4 = 0;
            }
            
            while(_CP0_GET_COUNT() < (48000000/2/5)) {} // 0.2 ms delay = 5 Hz timer
        }
        LCD_clearScreen(BACKGROUND); // use this function sparingly!
    }
    return 0;
}