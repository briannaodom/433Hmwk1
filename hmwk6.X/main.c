#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>   // needed to use sprintf()
#include"picConfig.h" // contains DEVCFGx commands
#include"ILI9163C.h"
#include"TFTLCD.h" // custom library

#define DELAYTIME 4000000 // 40000 yields 0.001 s delay time when using Core Timer
#define CS LATBbits.LATB7  // SPI chip select pin
#define STRLEN 19   // maximum number of characters per string

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
    
    char progress;
    char outbuf[STRLEN];
    float fps = 1.0;

    __builtin_enable_interrupts();

    _CP0_SET_COUNT(0);
    while(1) {
        for (progress=0; progress < 101; progress++) {
            _CP0_SET_COUNT(0);
            
            sprintf(outbuf,"Hello World %d!",progress);
            LCD_drawString(28, 32, outbuf, TEXTCOLOR);
            
            LCD_progressBar(15,70,progress,GREEN,RED);
            
            sprintf(outbuf,"FPS: %5.2f",fps);
            LCD_drawString(28,100,outbuf,TEXTCOLOR);
            
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
