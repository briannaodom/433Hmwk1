#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <math.h>

#pragma config DEBUG = 0b11 // no debugging (disabled)
#pragma config JTAGEN = 0 // no jtag (disabled)
#pragma config ICESEL = 0b11 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect ()
#pragma config BWP = 1 // no boot write protect (writeable)
#pragma config CP = 1 // no code protect

// DEVCFG1
#pragma config FNOSC = 0b011  // use primary oscillator with pll
#pragma config FSOSCEN = 0  // turn off secondary oscillator (disabled)
#pragma config IESO =  0 // no switching clocks (disabled)
#pragma config POSCMOD = 0b10 // high speed crystal mode
#pragma config OSCIOFNC = 1  // disable secondary osc
#pragma config FPBDIV = 0b00 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM =  0b11 // do not enable clock switch
#pragma config WDTPS = 0b10100 // use slowest wdt
#pragma config WINDIS = 1 // wdt no window mode
#pragma config FWDTEN = 0 // wdt disabled
#pragma config FWDTWINSZ = 0b11 // wdt window at 25% 

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = 0b001 // divide input clock to be in range 4-5MHz; resonater 8MHZ so divide by 2
#pragma config FPLLMUL = 0b111 // multiply clock after FPLLIDIV; used highest mult
#pragma config FPLLODIV = 0b001 // divide clock after FPLLMUL to get 48MHz
#pragma config UPLLIDIV = 0b001  // divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
#pragma config UPLLEN = 0 // USB clock on (enabled)

// DEVCFG3
#pragma config USERID = 0xFFFF // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = 0 // allow multiple reconfigurations
#pragma config IOL1WAY = 0 // allow multiple reconfigurations
#pragma config FUSBIDIO = 1 // USB pins controlled by USB module
#pragma config FVBUSONIO = 1 // USB BUSON controlled by USB module 

#define CS LATAbits.LATA4 //define CS pin
#define DELAYTIME 48000000*0.5*0.0005 //same as 480000000/2/2000

unsigned char triWave[100];  //square wave
unsigned char sinWave[100];  //sin wave

unsigned char SPI_IO(unsigned char write) {
    SPI1BUF = write;
    while(!SPI1STATbits.SPIRBF) {;}
    return SPI1BUF;
}

void INIT_SPI(void){
    
    TRISAbits.TRISA4 = 0;     //set SD1 to pin A4
    CS = 1;                   //chip no selected
    SPI1CONbits.ON = 0;       //turn off SPI module and reset it
    SPI1BUF;                  //Clear the rx buffer by reading from it
    SPI1BRG = 0x1000;         //SP1 clock set to 12 MHz; 0x1000 allows it to be viewed on NSCOPE
    SPI1STATbits.SPIROV = 0;  //Clear the overflow bit
    SPI1CONbits.CKE = 1;      //when clock goes from high to low data changes
    SPI1CONbits.MSTEN = 1;    //master operation
    SPI1CONbits.ON = 1;       //after setting parameters, turn on SP1 on 
    RPA1Rbits.RPA1R = 0b0011; //set A1 on MC pic to SD01/A1 pin on Pic32
}

void createWaves(void){
    int counter = 0;
    for (counter = 0;counter < 100; counter++) {
    sinWave[counter] = 255.0*0.5 + 255.0*0.5*sin(counter*3.14159*2.0*0.01); //same as 255.0/2 and 3.14159*2.0/100
    triWave[counter] = counter*255.0*0.01;
    }
}

void setVoltage(int channel, unsigned char voltage) {
    
    CS = 0; //chip is selected 
    
    if (channel == 1) { 
        unsigned int volt1 = 0b01110000; //send to first input 
        unsigned int volt2 = voltage >> 4;
        unsigned int result1 = volt1 | volt2;
        unsigned int result2 = voltage << 4;
        unsigned char data = SPI_IO(result1);
        unsigned char data1 = SPI_IO(result2);
    }
    else {
        unsigned int volt1 = 0b11110000; //send to second input
        unsigned int volt2 = voltage >> 4;
        unsigned int result1 = volt1 | volt2;
        unsigned int result2 = voltage << 4;
        unsigned char data = SPI_IO(result1);
        unsigned char data1 = SPI_IO(result2);  
    }
    CS = 1; //chip not selected
}


int main() {

    __builtin_disable_interrupts();
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583); // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    
    BMXCONbits.BMXWSDRM = 0x0; // 0 data RAM access wait states
    INTCONbits.MVEC = 0x1; // enable multi vector interrupts
    DDPCONbits.JTAGEN = 0;  // disable JTAG to get pins back
    
    INIT_SPI();
    createWaves();
    __builtin_enable_interrupts();
    
   
    int counter = 0;          
    while(1) {
        for (counter = 0; counter < 100;counter++){            
            setVoltage(0,sinWave[counter]); //sin wave on NScope
            _CP0_SET_COUNT(0);
            while(_CP0_GET_COUNT() < DELAYTIME){
                ;
            }
            setVoltage(1,triWave[counter]); //tri wave on NSCope        
            _CP0_SET_COUNT(0);
            while(_CP0_GET_COUNT() < DELAYTIME){
                ;
            }
        }             
    }
}

