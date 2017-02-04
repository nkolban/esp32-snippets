#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"

#define BMP180_ADDRESS 0x77

enum
{
	BMP085_REGISTER_CAL_AC1            = 0xAA,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_AC2            = 0xAC,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_AC3            = 0xAE,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_AC4            = 0xB0,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_AC5            = 0xB2,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_AC6            = 0xB4,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_B1             = 0xB6,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_B2             = 0xB8,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_MB             = 0xBA,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_MC             = 0xBC,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_MD             = 0xBE,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CHIPID             = 0xD0,
	BMP085_REGISTER_VERSION            = 0xD1,
	BMP085_REGISTER_SOFTRESET          = 0xE0,
	BMP085_REGISTER_CONTROL            = 0xF4,
	BMP085_REGISTER_TEMPDATA           = 0xF6,
	BMP085_REGISTER_PRESSUREDATA       = 0xF6,
	BMP085_REGISTER_READTEMPCMD        = 0x2E,
	BMP085_REGISTER_READPRESSURECMD    = 0x34 // 0011 0100
};
typedef enum
{
  BMP085_MODE_ULTRALOWPOWER          = 0,
  BMP085_MODE_STANDARD               = 1,
  BMP085_MODE_HIGHRES                = 2,
  BMP085_MODE_ULTRAHIGHRES           = 3
} bmp085_mode_t;

static char tag[] = "bmp180";

static short readRegister16(uint8_t reg) {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (BMP180_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
	i2c_master_write_byte(cmd, reg, 1);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	uint8_t msb;
	uint8_t lsb;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (BMP180_ADDRESS << 1) | I2C_MASTER_READ, 1 /* expect ack */);
	i2c_master_read_byte(cmd, &msb, 1);
	i2c_master_read_byte(cmd, &lsb, 0);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	short ret = (short)((msb << 8) | lsb);
	return ret;
}

static int readRegister24(uint8_t reg) {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (BMP180_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
	i2c_master_write_byte(cmd, reg, 1);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	uint8_t msb;
	uint8_t lsb;
	uint8_t xlsb;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (BMP180_ADDRESS << 1) | I2C_MASTER_READ, 1 /* expect ack */);
	i2c_master_read_byte(cmd, &msb, 1);
	i2c_master_read_byte(cmd, &lsb, 1);
	i2c_master_read_byte(cmd, &xlsb, 0);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	int ret = (int)((msb << 16) | (lsb << 8) | xlsb);
	return ret;
}

static uint32_t readUncompensatedTemp() {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (BMP180_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
	i2c_master_write_byte(cmd, BMP085_REGISTER_CONTROL, 1);
	i2c_master_write_byte(cmd, BMP085_REGISTER_READTEMPCMD, 1);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	vTaskDelay(5/portTICK_PERIOD_MS);
	long ret = (unsigned short)readRegister16(BMP085_REGISTER_TEMPDATA);
	return ret;
}

static uint32_t readUncompensatedPressure(uint32_t mode) {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (BMP180_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
	i2c_master_write_byte(cmd, BMP085_REGISTER_CONTROL, 1);
	i2c_master_write_byte(cmd, BMP085_REGISTER_READPRESSURECMD + (mode << 6), 1);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	switch(mode) {
	case BMP085_MODE_ULTRALOWPOWER:
		vTaskDelay(5/portTICK_PERIOD_MS);
		break;
	case BMP085_MODE_STANDARD:
		vTaskDelay(8/portTICK_PERIOD_MS);
		break;
	case BMP085_MODE_HIGHRES:
		vTaskDelay(14/portTICK_PERIOD_MS);
		break;
	case BMP085_MODE_ULTRAHIGHRES:
	default:
		vTaskDelay(26/portTICK_PERIOD_MS);
		break;
	}
	long ret;
	if (mode != BMP085_MODE_ULTRAHIGHRES) {
		ret = readRegister24(BMP085_REGISTER_PRESSUREDATA);
	} else {
		ret = readRegister24(BMP085_REGISTER_PRESSUREDATA);
	}
	return ret >> (8-mode);
}

double centigrade_to_fahrenheit(double centigrade) {
	return centigrade * 9 / 5 + 32.0;
}

double pascals_to_inHg(double pressure) {
	return pressure * 0.00029530;
}

void x() {


	short AC1 = readRegister16(BMP085_REGISTER_CAL_AC1);
	short AC2 = readRegister16(BMP085_REGISTER_CAL_AC2);
	short AC3 = readRegister16(BMP085_REGISTER_CAL_AC3);
	unsigned short AC4 = readRegister16(BMP085_REGISTER_CAL_AC4);
	unsigned short AC5 = readRegister16(BMP085_REGISTER_CAL_AC5);
	unsigned short AC6 = readRegister16(BMP085_REGISTER_CAL_AC6);
	short B1 = readRegister16(BMP085_REGISTER_CAL_B1);
	short B2 = readRegister16(BMP085_REGISTER_CAL_B2);
//	short MB = readRegister16(BMP085_REGISTER_CAL_MB);
	short MC = readRegister16(BMP085_REGISTER_CAL_MC);
	short MD = readRegister16(BMP085_REGISTER_CAL_MD);


	//ESP_LOGD(tag, "Params: AC1: %d, AC2: %d, AC3: %d, AC4:%d, AC5: %d, AC6: %d, B1: %d, B2: %d, MB: %d, MC: %d, MD: %d",
	//	AC1, AC2, AC3, AC4, AC5, AC6, B1, B2, MB, MC, MD);


	int32_t bmpMode = BMP085_MODE_STANDARD;
	int32_t UT = readUncompensatedTemp();
	int32_t UP = readUncompensatedPressure(bmpMode);

  int32_t X1 = (UT - (int32_t)AC6) * ((int32_t)AC5) >> 15;
  int32_t X2 = ((int32_t)MC<<11) / (X1+(int32_t)MD);
  int32_t B5 =  X1 + X2;
  double temp = ((B5 + 8)>>4)/10.0;

  ESP_LOGD(tag, "Temp = %.1foC = %.1foF", temp, centigrade_to_fahrenheit(temp));

  int32_t B6 = B5 - 4000;
  X1 = (B2 * (B6 * B6) >> 12) >> 11;
  X2 = (AC2 * B6) >> 11;
  int32_t X3 = X1 + X2;
  int32_t B3 = (((((int32_t)AC1)*4 + X3) << bmpMode) + 2) >> 2;
  X1 = (AC3 * B6) >> 13;
  X2 = (B1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  uint32_t B4 = ((uint32_t)AC4 * (uint32_t)(X3 + 32768)) >> 15;
  uint32_t B7 = ((((uint32_t)UP) - B3) * (50000 >> bmpMode));
  int32_t P;
  if (B7 < 0x80000000) {
  	P = (B7 << 1) / B4;
  } else {
  	P = (B7 / B4) << 1;
  }
  X1 = (P >> 8) * (P >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * P) >> 16;
  int32_t pressure = P + ((X1 + X2 + 3791) >> 4);
  ESP_LOGD(tag, "pressure = %d pa, = %f inHg, UP=%d", pressure, pascals_to_inHg(pressure), UP);
}



void task_bmp180(void *ignore) {
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = 18;
	conf.scl_io_num = 19;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;
	i2c_param_config(I2C_NUM_0, &conf);

	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
	while(1) {
		x();
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}
