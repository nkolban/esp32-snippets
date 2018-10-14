/*
 * OV7670.h
 *
 *  Created on: Jun 10, 2017
 *      Author: kolban
 */

#ifndef CPP_UTILS_OV7670_H_
#define CPP_UTILS_OV7670_H_
#include <I2C.h>
#include <driver/ledc.h>

#define OV7670_REG_GAIN   (0x00)
#define OV7670_REG_BLUE   (0x01)
#define OV7670_REG_RED    (0x02)
#define OV7670_REG_VREF   (0x03)
#define OV7670_REG_COM1   (0x04)
#define OV7670_REG_BAVE   (0x05)
#define OV7670_REG_GbAVE  (0x06)
#define OV7670_REG_AECHH  (0x07)
#define OV7670_REG_RAVE   (0x08)
#define OV7670_REG_COM2   (0x09)
#define OV7670_REG_PID    (0x0A)
#define OV7670_REG_VER    (0x0B)
#define OV7670_REG_COM3   (0x0C)
#define OV7670_REG_COM4   (0x0D)
#define OV7670_REG_COM5   (0x0E)
#define OV7670_REG_COM6   (0x0F)
#define OV7670_REG_AECH   (0x10)
#define OV7670_REG_CLKRC  (0x11)
#define OV7670_REG_COM7   (0x12)
#define OV7670_REG_COM8   (0x13)
#define OV7670_REG_COM9   (0x14)
#define OV7670_REG_COM10  (0x15)
#define OV7670_REG_HSTART (0x17)
#define OV7670_REG_HSTOP  (0x18)
#define OV7670_REG_VSTRT  (0x19)
#define OV7670_REG_VSTOP  (0x1a)
#define OV7670_REG_PSHFT  (0x1b)
#define OV7670_REG_MIDH   (0x1c)
#define OV7670_REG_MIDL   (0x1d)
#define OV7670_REG_MFVP   (0x1e)
#define OV7670_REG_LAEC   (0x1f)
#define OV7670_REG_ADCCTR0 (0x20)
#define OV7670_REG_ADCCTR1 (0x21)
#define OV7670_REG_ADCCTR2 (0x22)
#define OV7670_REG_ADCCTR3 (0x23)
#define OV7670_REG_AEW    (0x24)
#define OV7670_REG_AEB    (0x25)
#define OV7670_REG_VPT    (0x26)
#define OV7670_REG_BBIAS  (0x27)
#define OV7670_REG_GbBIAS (0x28)
#define OV7670_REG_RSVD   (0x00)
#define OV7670_REG_EXHCH  (0x2a)
#define OV7670_REG_EXHCL  (0x2b)
#define OV7670_REG_RBIAS  (0x2c)
#define OV7670_REG_ADVFL  (0x2d)
#define OV7670_REG_ADVFH  (0x2e)
#define OV7670_REG_YAVE   (0x2f)
#define OV7670_REG_HSYST  (0x30)
#define OV7670_REG_HSYEN  (0x31)
#define OV7670_REG_HREF   (0x32)
#define OV7670_REG_CHLF   (0x33)
#define OV7670_REG_ARBLM  (0x34)
#define OV7670_REG_ADC    (0x37)
#define OV7670_REG_ACOM   (0x38)
#define OV7670_REG_OFON   (0x39)
#define OV7670_REG_TSLB   (0x3a)
#define OV7670_REG_COM11  (0x3b)
#define OV7670_REG_COM12  (0x3c)
#define OV7670_REG_COM13  (0x3d)
#define OV7670_REG_COM14  (0x3e)
#define OV7670_REG_EDGE   (0x3f)
#define OV7670_REG_COM15  (0x40)
#define OV7670_REG_COM16  (0x41)
#define OV7670_REG_COM17  (0x42)
#define OV7670_REG_AWBC1  (0x43)
#define OV7670_REG_AWBC2  (0x44)
#define OV7670_REG_AWBC3  (0x45)
#define OV7670_REG_AWBC4  (0x46)
#define OV7670_REG_AWBC5  (0x47)
#define OV7670_REG_AWBC6  (0x48)
#define OV7670_REG_RSVD   (0x00)
#define OV7670_REG_REG4B  (0x4b)
#define OV7670_REG_DNSTH  (0x4c)
#define OV7670_REG_RSVD   (0x00)
#define OV7670_REG_MTX1   (0x4f)
#define OV7670_REG_MTX2   (0x50)
#define OV7670_REG_MTX3   (0x51)
#define OV7670_REG_MTX4   (0x52)
#define OV7670_REG_MTX5   (0x53)
#define OV7670_REG_MTX6   (0x54)
#define OV7670_REG_BRIGHT (0x55)
#define OV7670_REG_CONTRAS (0x56)
#define OV7670_REG_CONTRAS_CENTER (0x57)
#define OV7670_REG_MTXS  (0x58)
#define OV7670_REG_RSVD  (0x00)
#define OV7670_REG_LCC1  (0x62)
#define OV7670_REG_LCC2  (0x63)
#define OV7670_REG_LCC3  (0x64)
#define OV7670_REG_LCC4  (0x65)
#define OV7670_REG_LCC5  (0x66)
#define OV7670_REG_MANU  (0x67)
#define OV7670_REG_MANV  (0x68)
#define OV7670_REG_GFIX  (0x69)
#define OV7670_REG_GGAIN (0x6a)
#define OV7670_REG_DBLV  (0x6b)
#define OV7670_REG_AWBCTR3 (0x6c)
#define OV7670_REG_AWBCTR2 (0x6d)
#define OV7670_REG_AWBCTR1 (0x6e)
#define OV7670_REG_AWBCTR0 (0x6f)
#define OV7670_REG_SCALING_XSC      (0x70)
#define OV7670_REG_SCALING_YSC      (0x71)
#define OV7670_REG_SCALING_DCWCTR   (0x72)
#define OV7670_REG_SCALING_PCLK_DIV (0x73)
#define OV7670_REG_REG74   (0x74)
#define OV7670_REG_REG75   (0x75)
#define OV7670_REG_REG76   (0x76)
#define OV7670_REG_REG77   (0x77)
#define OV7670_REG_GAM1    (0x7a)
#define OV7670_REG_GAM2    (0x7b)
#define OV7670_REG_GAM3    (0x7c)
#define OV7670_REG_GAM4    (0x7d)
#define OV7670_REG_GAM5    (0x7e)
#define OV7670_REG_GAM6    (0x7f)
#define OV7670_REG_GAM7    (0x80)
#define OV7670_REG_GAM8    (0x81)
#define OV7670_REG_GAM9    (0x82)
#define OV7670_REG_GAM10   (0x83)
#define OV7670_REG_GAM11   (0x84)
#define OV7670_REG_GAM12   (0x85)
#define OV7670_REG_GAM13   (0x86)
#define OV7670_REG_GAM14   (0x87)
#define OV7670_REG_GAM15   (0x88)
#define OV7670_REG_RSVD    (0x00)
#define OV7670_REG_DM_LNL  (0x92)
#define OV7670_REG_DML_LNH (0x93)
#define OV7670_REG_LCC6    (0x94)
#define OV7670_REG_LCC7    (0x95)
#define OV7670_REG_BD50ST  (0x9d)
#define OV7670_REG_BD60ST  (0x9e)
#define OV7670_REG_STR_OPT (0xAC)
#define OV7670_REG_STR_R   (0xAD)
#define OV7670_REG_STR_G   (0xAE)
#define OV7670_REG_STR_B   (0xAF)
#define OV7670_REG_THL_ST  (0xB3)
#define OV7670_REG_THL_DLT (0xB5)
#define OV7670_REG_AD_CHB  (0xBE)
#define OV7670_REG_AD_CHR  (0xBF)
#define OV7670_REG_AD_CHGb (0xC0)
#define OV7670_REG_AD_CHGr (0xC1)
#define OV7670_REG_SATCTR  (0xC9)

// COM7[2] and COM7[0]
#define OV7670_FORMAT_YUV (0b00)
#define OV7670_FORMAT_RGB (0b10)

#define OV7670_FORMAT_RGB_RGB_NORMAL (0b00)
#define OV7670_FORMAT_RGB_RGB_565    (0b01)
#define OV7670_FORMAT_RGB_RGB_555    (0b11)

// SCALING_XSC[7] and SCALINT_YSC[7]
#define OV7670_TESTPATTERN_NONE      (0b00)
#define OV7670_TESTPATTERN_SHIFT_1   (0b01)
#define OV7670_TESTPATTERN_BAR_8     (0b10)
#define OV7670_TESTPATTERN_GRAY_FADE (0b11)

typedef struct {
    gpio_num_t pin_reset;          /*!< GPIO pin for camera reset line - OUT */
    gpio_num_t pin_xclk;           /*!< GPIO pin for camera XCLK line  - IN  */
    gpio_num_t pin_sscb_sda;       /*!< GPIO pin for camera SDA line   - OUT */
    gpio_num_t pin_sscb_scl;       /*!< GPIO pin for camera SCL line   - OUT */
    gpio_num_t pin_d7;             /*!< GPIO pin for camera D7 line    - IN  */
    gpio_num_t pin_d6;             /*!< GPIO pin for camera D6 line    - IN  */
    gpio_num_t pin_d5;             /*!< GPIO pin for camera D5 line    - IN  */
    gpio_num_t pin_d4;             /*!< GPIO pin for camera D4 line    - IN  */
    gpio_num_t pin_d3;             /*!< GPIO pin for camera D3 line    - IN  */
    gpio_num_t pin_d2;             /*!< GPIO pin for camera D2 line    - IN  */
    gpio_num_t pin_d1;             /*!< GPIO pin for camera D1 line    - IN  */
    gpio_num_t pin_d0;             /*!< GPIO pin for camera D0 line    - IN  */
    gpio_num_t pin_vsync;          /*!< GPIO pin for camera VSYNC line - IN  */
    gpio_num_t pin_href;           /*!< GPIO pin for camera HREF line  - IN  */
    gpio_num_t pin_pclk;           /*!< GPIO pin for camera PCLK line  - IN  */

    int xclk_freq_hz;       /*!< Frequency of XCLK signal, in Hz */
    ledc_timer_t ledc_timer;        /*!< LEDC timer to be used for generating XCLK  */
    ledc_channel_t ledc_channel;    /*!< LEDC channel to be used for generating XCLK  */
} camera_config_t;

class OV7670 {
public:
	OV7670();
	virtual ~OV7670();
	void init(camera_config_t cameraConfig);
	void dump();
	void reset();
	void setFormat(uint8_t value);
	void setRGBFormat(uint8_t value);
	void setTestPattern(uint8_t value);
	void resetCamera();

private:
	uint8_t readRegister(uint8_t reg);
	void writeRegister(uint8_t reg, uint8_t value);
	camera_config_t m_cameraConfig;
	I2C* m_i2c;

};

#endif /* CPP_UTILS_OV7670_H_ */
