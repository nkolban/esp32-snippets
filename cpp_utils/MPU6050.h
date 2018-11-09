#ifndef MAIN_MPU6050_H_
#define MAIN_MPU6050_H_

#include <math.h>
#include "I2C.h"
#include <driver/gpio.h>

/**
 * @brief Driver for the %MPU6050 accelerometer and gyroscope.
 *
 * The %MPU6050 uses %I2C as the communication between the master and the slave.  The class stores
 * the last read values as class instance local data.  The data can be retrieved from the class
 * using the appropriate getters.  The readAccel() and readGyro() methods cause communication
 * with the device to retrieve and locally hold the values from the device.
 *
 * A call to init() must precede all other API calls.
 */
class MPU6050 {
public:
	MPU6050();
	virtual ~MPU6050();

	/**
	 * @brief Get the X acceleration value.
	 */
	short getAccelX() const {
		return accel_x;
	}

	/**
	 * @brief Get the Y acceleration value.
	 */
	short getAccelY() const {
		return accel_y;
	}

	/**
	 * @brief Get the Z acceleration value.
	 */
	short getAccelZ() const {
		return accel_z;
	}

	/**
	 * @brief Get the X gyroscopic value.
	 */
	short getGyroX() const {
		return gyro_x;
	}

	/**
	 * @brief Get the Y gyroscopic value.
	 */
	short getGyroY() const {
		return gyro_y;
	}

	/**
	 * @brief Get the Z gyroscopic value.
	 */
	short getGyroZ() const {
		return gyro_z;
	}

	/**
	 * @brief Get the magnitude of the acceleration.
	 *
	 * Since acceleration is a vector quantity, it has both direction and magnitude.  This
	 * method returns the currently known magnitude of the last read data.
	 *
	 * @return The magnitude of the acceleration.
	 */
	uint32_t getMagnitude() {
		return sqrt(accel_x * accel_x + accel_y * accel_y + accel_z * accel_z);
	}

	void init(gpio_num_t sdaPin = I2C::DEFAULT_SDA_PIN, gpio_num_t clkPin = I2C::DEFAULT_CLK_PIN);
	void readAccel();
	void readGyro();

private:
	I2C* i2c;
	short accel_x, accel_y, accel_z;
	short gyro_x, gyro_y, gyro_z;
	bool inited;

};

#endif /* MAIN_MPU6050_H_ */
