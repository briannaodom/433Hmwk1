#include <xc.h>
#include"i2c_master_noint.h" // I2C library
#include"LSM6DS33.h" // high-level IMU library
#include"TFTLCD.h"

void LSM6DS33_init(void) { // initialize LSM6DS33 IMU
    i2c_master_setup(); // setup I2C2 at 100 kHz

    // initialize XL (accelerometer)
    i2c_master_start(); // START bit
    i2c_master_send(IMU_ADDR << 1);  // hardware address; RW (lsb) = 0, indicates write
    i2c_master_send(0x10);  // specify address to write to: 0x10 = CTRL1_XL (turn on the acceleromter)
    i2c_master_send(0x82);  // send value byte to address specified above
                        // ODR_XLx = 0b1000 -> 1.66 kHz sample rate
                        // FS_XL = 0b00 -> 2g sensitivity
                        // BW_XL = 0b10 -> 100 Hz low-pass filter
    i2c_master_stop();  // STOP bit

    // initialize G (gyroscope)
    i2c_master_start(); // START bit
    i2c_master_send(IMU_ADDR << 1);  // hardware address; RW (lsb) = 0, indicates write
    i2c_master_send(0x11);  // specify address to write to: 0x11 = CTRL2_G (turn on the gyroscope)
    i2c_master_send(0x88);  // send value byte to address specified above
                        // ODR_Gx = 0b1000 -> 1.66 kHz sample rate
                        // FS_Gx = 0b10 -> 2g sensitivity
                        // FS_125 = 0b0 -> disable full-scale @ 125 dps
                        // empty bit
    i2c_master_stop();  // STOP bit

    // configure for sequential reading
    i2c_master_start(); // START bit
    i2c_master_send(IMU_ADDR << 1);  // hardware address; RW (lsb) = 0, indicates write
    i2c_master_send(0x12);  // specify address to write to: 0x12 = CTRL3_C (control register 3)
    i2c_master_send(0x04);  // send value byte to address specified above
                            // 0x04 -> IF_INC = 1 -> able to read from multiple registers sequentially
    i2c_master_stop();  // STOP bit
}

unsigned char WhoAmI(void) { // use this to confirm communication with LSM6DS33 IMU
    unsigned char answer;
    i2c_master_start();
    i2c_master_send((IMU_ADDR << 1)); // hardware address and write bit
    i2c_master_send(0x0F);  // WHO_AM_I register: 0x0F
    i2c_master_restart(); // this line is REALLY important!
    i2c_master_send((IMU_ADDR << 1) | 1); // hardware address and read bit
    answer = i2c_master_recv(); // receive a byte from the slave. Should be 0x69 = 105d
    i2c_master_ack(1); // send NACK to slave
    i2c_master_stop();
    return answer;
}

void I2C_read_multiple(unsigned char address, unsigned char registerrr, unsigned char * data, unsigned char length) { // perform I2C sequential read
    char i;
        i2c_master_start();
        i2c_master_send((address << 1)); // hardware address and write bit
        i2c_master_send(registerrr);  // starting register. "register" has some other meaning in C, so I called it "registerrr" instead.
        i2c_master_restart(); // this line is REALLY important!
        i2c_master_send((address << 1) | 1); // hardware address and read bit
        for (i = 0; i < length; i++) {
            data[i] = i2c_master_recv(); // receive a byte from the slave. Should be 0x69 = 105d
            if (i==(length-1)) {
                i2c_master_ack(1); // send NACK to slave to stop reading
            }
            else {
                i2c_master_ack(0); // send ACK to slave to continue reading
            }
        }
        i2c_master_stop();
}

signed short getTemp(unsigned char * data) { // convert temperature LSB and MSB to signed short
    return data[1]<<8 | data[0];
}

signed short getxG(unsigned char * data) { // convert x-gyro LSB and MSB to signed short
    return data[3]<<8 | data[2];
}

signed short getyG(unsigned char * data) { // convert y-gyro LSB and MSB to signed short
    return data[5]<<8 | data[4];
}

signed short getzG(unsigned char * data) { // convert z-gyro LSB and MSB to signed short
    return data[7]<<8 | data[6];
}

signed short getxXL(unsigned char * data) { // convert x-acceleration LSB and MSB to signed short
    return data[9]<<8 | data[8];
}

signed short getyXL(unsigned char * data) { // convert y-acceleration LSB and MSB to signed short
    return data[11]<<8 | data[10];
}

signed short getzXL(unsigned char * data) { // convert z-acceleration LSB and MSB to signed short
    return data[13]<<8 | data[12];
}

float convxXL(unsigned char * data) { // convert x-acceleration signed short to float (g's)
    return (getxXL(data))*0.000061;
}

float convyXL(unsigned char * data) { // convert y-acceleration signed short to float (g's)
    return (getyXL(data))*0.000061;
}

float convzXL(unsigned char * data) { // convert z-acceleration signed short to float (g's)
    return (getzXL(data))*0.000061;
}

float convTemp(unsigned char * data) { // convert temperature signed short to float (Celsius)
    return (getTemp(data)+400)/16.0;
}

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
