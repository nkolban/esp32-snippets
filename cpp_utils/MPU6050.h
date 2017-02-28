/**
 * Encapsulate the MPU6050.
 *
 * This class encapsulates the MPU6050 accelerometer and gyroscope.
 * The MPU6050 uses I2C as the communication between the master and the slave.
 */


#ifndef MAIN_MPU6050_H_
#define MAIN_MPU6050_H_
#include "I2C.h"

class MPU6050 {
private:
	I2C *i2c;
	short accel_x, accel_y, accel_z;
	short gyro_x, gyro_y, gyro_z;
public:
	MPU6050();
	virtual ~MPU6050();

	/**
	 * Read the acceleration values from the MPU-6050.
	 */
	void readAccel();

	/**
	 * Read the gyor values from the MPU-6050.
	 */
	void readGyro();

	/**
	 * Retrieve the X acceleration value.
	 */
	short getAccelX() const
	{
		return accel_x;
	}

	/**
	 * Retrieve the Y acceleration value.
	 */
	short getAccelY() const
	{
		return accel_y;
	}

	/**
	 * Retrieve the Z acceleration value.
	 */
	short getAccelZ() const
	{
		return accel_z;
	}

	/**
	 * Retrieve X gyro value.
	 */
	short getGyroX() const
	{
		return gyro_x;
	}


	/**
	 * Retrieve Y gyro value.
	 */
	short getGyroY() const
	{
		return gyro_y;
	}


	/**
	 * Retrieve Z gyro value.
	 */
	short getGyroZ() const
	{
		return gyro_z;
	}
};

#endif /* MAIN_MPU6050_H_ */
