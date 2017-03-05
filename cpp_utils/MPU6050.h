#ifndef MAIN_MPU6050_H_
#define MAIN_MPU6050_H_
#include "I2C.h"

/**
 * @brief Driver for the MPU6050 accelerometer and gyroscope.
 *
 * The MPU6050 uses I2C as the communication between the master and the slave.  The class stores
 * the last read values as class instance local data.  The data can be retrieved from the class
 * using the appropriate getters.  The readAccel() and readGyro() methods cause communication
 * with the device to retrieve and locally hold the values from the device.
 */
class MPU6050 {
private:
	I2C *i2c;
	short accel_x, accel_y, accel_z;
	short gyro_x, gyro_y, gyro_z;
public:
	MPU6050();
	virtual ~MPU6050();

	void readAccel();

	void readGyro();

	/**
	 * @brief Get the X acceleration value.
	 */
	short getAccelX() const
	{
		return accel_x;
	}

	/**
	 * @brief Get the Y acceleration value.
	 */
	short getAccelY() const
	{
		return accel_y;
	}

	/**
	 * @brief Get the Z acceleration value.
	 */
	short getAccelZ() const
	{
		return accel_z;
	}

	/**
	 * @brief Get the X gyroscopic value.
	 */
	short getGyroX() const
	{
		return gyro_x;
	}

	/**
	 * @brief Get the Y gyroscopic value.
	 */
	short getGyroY() const
	{
		return gyro_y;
	}

	/**
	 * @brief Get the Z gyroscopic value.
	 */
	short getGyroZ() const
	{
		return gyro_z;
	}
};

#endif /* MAIN_MPU6050_H_ */
