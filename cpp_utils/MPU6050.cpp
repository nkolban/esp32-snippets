/**
 * Encapsulate the MPU6050.
 */

#include "MPU6050.h"
#define PIN_SDA 25
#define PIN_CLK 26
#define I2C_ADDRESS 0x68 // I2C address of MPU6050

#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_GYRO_XOUT_H  0x43
#define MPU6050_PWR_MGMT_1   0x6B

/**
 * Construct an MPU6050 handler.
 */
MPU6050::MPU6050() {
	i2c = new I2C(PIN_SDA, PIN_CLK);
	// Dummy call
	i2c->setAddress(I2C_ADDRESS);
	i2c->beginTransaction();
	i2c->write(MPU6050_ACCEL_XOUT_H);
	i2c->endTransaction();

	i2c->beginTransaction();
	i2c->write(MPU6050_PWR_MGMT_1);
	i2c->write(0);
	i2c->endTransaction();
	accel_x = accel_y = accel_z = 0;
	gyro_x  = gyro_y  = gyro_z  = 0;
}


MPU6050::~MPU6050() {
	delete i2c;
}


/**
 * Read the acceleration value from the device.
 */
void MPU6050::readAccel() {
	i2c->beginTransaction();
	i2c->write(MPU6050_ACCEL_XOUT_H);
	i2c->endTransaction();

	uint8_t data[6];
	i2c->beginTransaction();
	i2c->read(data, 5, 1);
	i2c->readByte(data+5, 0);
	i2c->endTransaction();

	accel_x = (data[0] << 8) | data[1];
	accel_y = (data[2] << 8) | data[3];
	accel_z = (data[4] << 8) | data[5];
} // readAccel

/**
 * Read the gyroscopic values from the device.
 */
void MPU6050::readGyro() {
	i2c->beginTransaction();
	i2c->write(MPU6050_GYRO_XOUT_H);
	i2c->endTransaction();

	uint8_t data[6];
	i2c->beginTransaction();
	i2c->read(data, 5, 1);
	i2c->readByte(data+5, 0);
	i2c->endTransaction();

	gyro_x = (data[0] << 8) | data[1];
	gyro_y = (data[2] << 8) | data[3];
	gyro_z = (data[4] << 8) | data[5];
} // readGyro
