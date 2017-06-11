/* 
 */

#ifndef I2C_MASTER_NOINT_H
#define	I2C_MASTER_NOINT_H

#define ADDRESS 0b1101011   
// Slave address
#define OUT_TEMP_L 0x20 //Temperature 
#define CTRL1_XL 0x10
#define CTRL2_G  0x11
#define CTRL3_C  0x12
#define OUTX_L_XL 0x28 //Linear acceleration of x_axis
#define OUTX_H_XL 0x29 //Linear acceleration of x_axis

void i2c_master_setup();
void i2c_master_start();
void i2c_master_send(unsigned char byte);
void i2c_master_stop();
void i2c_master_restart();
unsigned char i2c_master_recv(); 
void i2c_master_ack(int val);
void I2C_read_multiple(unsigned char address, unsigned char register, unsigned char * data, int length);
void I2C_read_multiple_hack(unsigned char address, unsigned char reg, unsigned char * data, int length);

void initIMU();
void setExpander(unsigned char address, unsigned char command);
signed short get_x_accel(unsigned char * data);

#endif	