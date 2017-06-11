#include <proc/p32mx250f128b.h>
#include"i2c_master_noint.h"
#include"ILI9163C.h"
#include<stdio.h>

void i2c_master_setup(void) {
  ANSELBbits.ANSB2 = 0;
  ANSELBbits.ANSB3 = 0;
  I2C2BRG = 233;            // I2CBRG = [1/(2*Fsck) - PGD]*Pblck - 2  //Fsck = 400MHz, PGD = 48MHz, Pblck=104 ns (53 for 400MHz, 233 for 100MHz)
  I2C2CONbits.ON = 1;               // turn on the I2C2 module
}


// Start a transmission on the I2C bus
void i2c_master_start(void) {
    I2C2CONbits.SEN = 1;            // send the start bit
    while(I2C2CONbits.SEN) { ; }    // wait for the start bit to be sent
}

void i2c_master_send(unsigned char byte) { // send a byte to slave
  I2C2TRN = byte;                   // if an address, bit 0 = 0 for write, 1 for read
  while(I2C2STATbits.TRSTAT) { ; }  // wait for the transmission to finish
  if(I2C2STATbits.ACKSTAT) {        // if this is high, slave has not acknowledged
    // ("I2C2 Master: failed to receive ACK\r\n");
  }
}

void i2c_master_stop(void) {          // send a STOP:
  I2C2CONbits.PEN = 1;                // comm is complete and master relinquishes bus
  while(I2C2CONbits.PEN) { ; }        // wait for STOP to complete
}

void i2c_master_restart(void) {     
    I2C2CONbits.RSEN = 1;           // send a restart 
    while(I2C2CONbits.RSEN) { ; }   // wait for the restart to clear
}


unsigned char i2c_master_recv(void) { // receive a byte from the slave
    I2C2CONbits.RCEN = 1; // start receiving data
    while (!I2C2STATbits.RBF) {
        ;
    } // wait to receive the data
    return I2C2RCV; // read and return the data
}

void i2c_master_ack(int val) {        // sends ACK = 0 (slave should send another byte)
                                      // or NACK = 1 (no more bytes requested from slave)
    I2C1CONbits.ACKDT = val;          // store ACK/NACK in ACKDT
    I2C1CONbits.ACKEN = 1;            // send ACKDT
    while(I2C1CONbits.ACKEN) { ; }    // wait for ACK/NACK to be sent
}


void setExpander(unsigned char address, unsigned char command){
    i2c_master_start();
    i2c_master_send((ADDRESS << 1)|0); //opcode = 0b01000000, write mode
    i2c_master_send(address);
    i2c_master_send(command);
    i2c_master_stop();      
}

void initIMU(void){
    i2c_master_setup();
//    setExpander(0x00, 0b11110000);
//    setExpander(0x01, 0x0); //Enable embedded functions register
    setExpander(CTRL1_XL, 0x82); //0b10000010 Turn on the accelerometer, Set the sample rate to 1.66 kHz, with 2g sensitivity, and 100 Hz filter.
    setExpander(CTRL2_G, 0x88); // 0b10001000 turn on the gyroscope, write to the CTRL2_G register. Set the sample rate to 1.66 kHz, with 1000 dps sensitivity.
    setExpander(CTRL3_C, 0x04); //Making the IF_INC bit 1 will enable the ability to read multiple registers in a row without specifying every register location, saving us time when reading out all of the data. 
}                                   

char getExpander(unsigned char address){
    i2c_master_start();
    i2c_master_send((ADDRESS << 1)|0);
    i2c_master_send(address); // the register to read from
    i2c_master_restart(); // make the restart bit
    i2c_master_send((ADDRESS << 1)|1); // write the address, shifted left by 1, or with a 1 to indicate reading
    unsigned char r = i2c_master_recv(); // save the value returned
    i2c_master_ack(1); // make the ack so the slave knows we got it
    i2c_master_stop(); // make the stop bit
    return r;
}

void I2C_read_multiple(unsigned char address, unsigned char reg, unsigned char * data, int length){
    // read one
    int i = 0;
    i2c_master_start();
    i2c_master_send(ADDRESS << 1|0);
    i2c_master_send(reg); // the register to read from
    i2c_master_restart(); // make the restart bit
    i2c_master_send(ADDRESS << 1|1); // write the address, shifted left by 1, or with a 1 to indicate reading     
    for (i = 0; i < length; i++){    
        data[i] = i2c_master_recv(); // save the value returned    
        if (i < length - 1) {                 
            i2c_master_ack(0); // make the ack so the slave knows we got it   
        }
        else {
            i2c_master_ack(1); // make the ack so the slave knows we got it            
        }
    }

    i2c_master_stop(); // make the stop bit
}

void I2C_read_multiple_hack(unsigned char address, unsigned char reg, unsigned char * data, int length){
    // read one
    int i = 0;
    for (i = 0; i < length; i++){
        data[i] = getExpander(reg + i);
    }
}

signed short get_x_accel(unsigned char * data){
    signed short x_xl = 0;
    x_xl = (data[1] << 8) | data[0];
    return x_xl;
}
