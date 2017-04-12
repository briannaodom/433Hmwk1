#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <math.h>

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // free up secondary osc pins
#pragma config FPBDIV = DIV_1 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1 // slowest wdt
#pragma config WINDIS = OFF // no wdt window
#pragma config FWDTEN = OFF // wdt off by default
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 48MHz
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiply by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 0 // some 16bit userubmit the link to your repid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

#define CS LATBbits.LATB7
#define DELAY 48000000/2/2000

unsigned int sin_wave[100];
unsigned int tri_wave[200];

char SPI1_IO(unsigned char write){
    SPI1BUF = write;
    while(!SPI1STATbits.SPIRBF){;
  }
 return SPI1BUF;   
}

void setVoltage(unsigned int channel, unsigned int voltage)
{
    unsigned int result1;
    unsigned int result2;
    CS = 0;
    result1 = voltage << 4;
    result2 = (( 0b111<< 4)|(channel << 7))|(voltage << 4); 
    SPI1_IO(result1);
    SPI1_IO(result2);
    CS = 1;
}

void init_SPI1()
{
    CS = 1;                  //chip not selected
    SPI1CONbits.ON=0;        //SP1 off before setting params
    SPI1BUF;                 //clear rx buffer to read
    SPI1BRG = 0x1000; 
    SPI1CONbits.MSTEN = 1;   //master op
    SPI1STATbits.SPIROV = 0; //clear overflow
    SPI1CONbits.CKP = 0;
    SPI1CONbits.CKE = 1;     //data change based on high to low
    SPI1CONbits.ON = 1;
    
    TRISBbits.TRISB7 = 0;
    RPB17Rbits.RPB17R = 0b0011; //SDO1 set to pin 17
    SDI1Rbits.SDI1R = 0b0100; // SDI1 set to pin 17
     // Set up Pin B7 to be CS (Slave select Digital Output pin)   
}

void DAC_waves(){
    int i;
    int j;
    double tempSin;
    double tempTri;
    
    for(i=0; i < 100; i++){
        tempSin= (255.0/2.0)+(255.0/2.0)*sin(2*3.14159*(i/100.0));
        sin_wave[i] = tempSin;        
    }
    for(j=0; j< 200; j++){
        tempTri = (255.0/200.0)*j;
        tri_wave[j] = tempTri;
    }
}

int main() {
    __builtin_disable_interrupts();
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583); // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    BMXCONbits.BMXWSDRM = 0x0; // 0 data RAM access wait states
    INTCONbits.MVEC = 0x1; // enable multi vector interrupts
    DDPCONbits.JTAGEN = 0;  // disable JTAG to get pins back

    init_SPI1();
    DAC_waves();
    __builtin_enable_interrupts();
    int i = 0;
    int j = 0;
    while(1) {
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < DELAY){//1KHz
            ;
        }
        setVoltage(1,sin_wave[i]);
        _CP0_SET_COUNT(0);
         while(_CP0_GET_COUNT() < DELAY){//1KHz
            ;
        }
        setVoltage(2,tri_wave[j]);
        i++, j++;
        if(i == 100){
            i = 0;
        }
        if(j == 200){
            j= 0;
        }
    }
}


