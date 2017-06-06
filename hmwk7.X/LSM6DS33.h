#ifndef LSM6DS33_H__
#define LSM6DS33_H__
// Header file for LSM6DS33.c
// implements high-level IMU functions using I2C

#define IMU_ADDR 0x6A // I2C hardware address of LSM6DS33
#define LSM_ARRAY_LEN 14 // data array has 14 values

void LSM6DS33_init(void); // initialize LSM6DS33 IMU
unsigned char WhoAmI(void); // use this to confirm communication with LSM6DS33 IMU
void I2C_read_multiple(unsigned char, unsigned char, unsigned char *, unsigned char); // perform I2C sequential read
signed short getTemp(unsigned char *); // convert temperature LSB and MSB to signed short
float convTemp(unsigned char *); // convert temperature signed short to float (Celsius)
signed short getxXL(unsigned char *); // convert x-acceleration LSB and MSB to signed short
float convxXL(unsigned char *); // convert x-acceleration signed short to float (g's)
signed short getyXL(unsigned char *); // convert y-acceleration LSB and MSB to signed short
float convyXL(unsigned char *); // convert y-acceleration signed short to float (g's)
signed short getzXL(unsigned char *); // convert z-acceleration LSB and MSB to signed short
float convzXL(unsigned char *); // convert z-acceleration signed short to float (g's)

void drawxXLvec(char, char, signed char, short); // draw x-vector
void drawyXLvec(char, char, signed char, short); // draw y-vector

#endif