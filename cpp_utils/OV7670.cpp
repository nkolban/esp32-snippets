/*
 * OV7670.cpp
 *
 *  Created on: Jun 10, 2017
 *      Author: kolban
 */

#include "OV7670.h"
#include "I2S.h"
#include <driver/gpio.h>
#include <driver/ledc.h>
#include "GPIO.h"
#include "FreeRTOS.h"
#include "GeneralUtils.h"
#include "sdkconfig.h"
#include <esp_log.h>

#define OV7670_I2C_ADDR (0x21)

extern "C" {
	#include <soc/i2s_reg.h>
	#include <soc/i2s_struct.h>
}

static const char* LOG_TAG = "OV7670";

static bool getBit(uint8_t value, uint8_t bitNum) {
	return (value & (1 << bitNum)) != 0;
}

static uint8_t setBit(uint8_t value, uint8_t bitNum, bool bitValue) {
	value = value & (~(1 << bitNum));
	value = value | (bitValue << bitNum);
	return value;
}

static void toGrayscale(uint8_t* pData, uint32_t length) {
	uint32_t i = 0;
	uint8_t* pLine = new uint8_t[length / 2];
	uint8_t* pLineSave = pLine;
	// RGB 565
	// 76543210 76543210
	//  RRRRRGGG GGGBBBBB
	while (i < length) {
		uint8_t byte1 = *pData;
		pData++;
		uint8_t byte2 = *pData;
		pData++;
		uint8_t red   = ((byte1 & 0b11111000) >> 3) << 3;
		uint8_t green = (((byte1 & 0b111) << 3) | ((byte2 & 0b11100000) >> 5)) << 2;
		uint8_t blue  = (byte2 & 0b11111) << 3;
		*pLine = (red + green + blue) / 3;
		pLine++;
		i += 2;
	}
	pLine = pLineSave;
	GeneralUtils::hexDump(pLine, length / 2);
}

void OV7670::setFormat(uint8_t value) {
	uint8_t com7 = readRegister(OV7670_REG_COM7);
	com7 = setBit(com7, 2, getBit(value, 1));
	com7 = setBit(com7, 0, getBit(value, 0));
	writeRegister(OV7670_REG_COM7, com7);
}

void OV7670::resetCamera() {
	uint8_t com7 = readRegister(OV7670_REG_COM7);
	com7 = setBit(com7, 7, true);
	writeRegister(OV7670_REG_COM7, com7);
}

void OV7670::setRGBFormat(uint8_t value) {
	uint8_t com15 = readRegister(OV7670_REG_COM15);
	com15 = setBit(com15, 5, getBit(value, 1));
	com15 = setBit(com15, 4, getBit(value, 0));
	writeRegister(OV7670_REG_COM15, com15);
}

void OV7670::setTestPattern(uint8_t value) {
	uint8_t com7 = readRegister(OV7670_REG_COM7);
	if (value == OV7670_TESTPATTERN_NONE) {
		com7 = setBit(com7, 1, false);
	} else {
		com7 = setBit(com7, 1, true);
	}
	writeRegister(OV7670_REG_COM7, com7);

	uint8_t scaling_xsc = readRegister(OV7670_REG_SCALING_XSC);
	scaling_xsc = setBit(scaling_xsc, 7, getBit(value, 1));
	writeRegister(OV7670_REG_SCALING_XSC, scaling_xsc);
	uint8_t scaling_ysc = readRegister(OV7670_REG_SCALING_YSC);
	scaling_ysc = setBit(scaling_ysc, 7, getBit(value, 0));
	writeRegister(OV7670_REG_SCALING_YSC, scaling_ysc);
}
/*
 *
 * COM7
 * +---+
 * | 7 | SCCB Register Reset
 * |   | 0 - No change
 * |   | 1 - Reset
 * +---+
 * | 6 | Reserved
 * +---+
 * | 5 | Output format - CIF selection
 * +---+
 * | 4 | Output format - QVGA selection
 * +---+
 * | 3 | Output format - QCIF selection
 * +---+
 * | 2 | Output format - RGB selection
 * +---+
 * | 1 | Output format - Color Bar
 * +---+
 * | 0 | Output format - Raw RGB
 * |   |                          COM7[2]   COM7[0]
 * |   | YUV                        0         0
 * |   | RGB                        1         0
 * |   | Bayer RAW                  0         1
 * |   | Processed Bayer RAW        1         1
 * +---+
 *
 */

/*
static void IRAM_ATTR isr_vsync(void* arg) {
	ESP_EARLY_LOGD(LOG_TAG, "VSYNC");
}
*/

static uint32_t vsyncCounter = 0;
//static uint32_t hrefCounter = 0;
static uint32_t lastHref = 0;
static uint32_t pclkCounter = 0;
//static uint32_t lastPclk = 0;

/*
static void IRAM_ATTR vsyncHandler(void* arg) {
	vsyncCounter++;
	lastHref = hrefCounter;
	hrefCounter = 0;
}
*/

/*
static void IRAM_ATTR hrefHandler(void* arg) {
	hrefCounter++;
}
*/

/*
static void IRAM_ATTR pclckHandler(void* arg) {
	pclkCounter++;
	portYIELD_FROM_ISR();
}
*/

/*
static inline void i2s_conf_reset()
{
    const uint32_t lc_conf_reset_flags = I2S_IN_RST_M | I2S_AHBM_RST_M | I2S_AHBM_FIFO_RST_M;
    I2S0.lc_conf.val |= lc_conf_reset_flags;
    I2S0.lc_conf.val &= ~lc_conf_reset_flags;

    const uint32_t conf_reset_flags = I2S_RX_RESET_M | I2S_RX_FIFO_RESET_M | I2S_TX_RESET_M | I2S_TX_FIFO_RESET_M;
    I2S0.conf.val |= conf_reset_flags;
    I2S0.conf.val &= ~conf_reset_flags;
    while (I2S0.state.rx_fifo_reset_back) {
        ;
    }
}
*/

OV7670::OV7670() {
	m_i2c = nullptr;
}


OV7670::~OV7670() {
	if (m_i2c != nullptr) {
		delete m_i2c;
		m_i2c = nullptr;
	}
}


static esp_err_t camera_enable_out_clock(camera_config_t* config) {
	ESP_LOGD(LOG_TAG, ">> camera_enable_out_clock: freq_hz=%d, pin=%d", config->xclk_freq_hz, config->pin_xclk);
	periph_module_enable(PERIPH_LEDC_MODULE);

	ledc_timer_config_t timer_conf;
	timer_conf.freq_hz    = config->xclk_freq_hz;
	timer_conf.duty_resolution = (ledc_timer_bit_t) 1;
	timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	timer_conf.timer_num  = config->ledc_timer;
	esp_err_t err = ledc_timer_config(&timer_conf);
	if (err != ESP_OK) {
		ESP_LOGE(LOG_TAG, "ledc_timer_config failed, rc=%x", err);
		return err;
	}

	ledc_channel_config_t ch_conf;
	ch_conf.channel    = config->ledc_channel;
	ch_conf.timer_sel  = config->ledc_timer;
	ch_conf.intr_type  = LEDC_INTR_DISABLE;
	ch_conf.duty       = 1;
	ch_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	ch_conf.gpio_num   = config->pin_xclk;

	err = ledc_channel_config(&ch_conf);
	if (err != ESP_OK) {
		ESP_LOGE(LOG_TAG, "ledc_channel_config failed, rc=%x", err);
		return err;
	}
	ESP_LOGD(LOG_TAG, "<< camera_enable_out_clock");
	return ESP_OK;
} // camera_enable_out_clock

/*
static void i2s_init() {
	// Enable and configure I2S peripheral
	periph_module_enable(PERIPH_I2S0_MODULE);
	// Toggle some reset bits in LC_CONF register
	// Toggle some reset bits in CONF register
	// Enable slave mode (sampling clock is external)

	// Switch on Slave mode.
	// I2S_CONF_REG -> I2S_RX_SLAVE_MOD
	I2S0.conf.rx_slave_mod = 1;

	// Enable parallel mode
	// I2S_CONF2_REG -> I2S_LCD_END
	I2S0.conf2.lcd_en = 1;

	// Use HSYNC/VSYNC/HREF to control sampling
	// I2S_CONF2_REG -> I2S_CAMERA_EN
	I2S0.conf2.camera_en = 1;


	// Configure clock divider
	I2S0.clkm_conf.clkm_div_a   = 1;
	I2S0.clkm_conf.clkm_div_b   = 0;
	I2S0.clkm_conf.clkm_div_num = 2;

	// FIFO will sink data to DMA
	I2S0.fifo_conf.dscr_en = 1;

	// FIFO configuration
	I2S0.fifo_conf.rx_fifo_mod          = 0; // 0-3???
	I2S0.fifo_conf.rx_fifo_mod_force_en = 1;
	I2S0.conf_chan.rx_chan_mod          = 1;

	// Clear flags which are used in I2S serial mode
	I2S0.sample_rate_conf.rx_bits_mod = 0;
	I2S0.conf.rx_right_first = 0;
	I2S0.conf.rx_msb_right   = 0;
	I2S0.conf.rx_msb_shift   = 0;
	I2S0.conf.rx_mono        = 0;
	I2S0.conf.rx_short_sync  = 0;
	I2S0.timing.val          = 0;

}
*/

/**
 * @brief Dump the settings.
 */
void OV7670::dump() {
	ESP_LOGD(LOG_TAG, "PID: 0x%.2x, VER: 0x%.2x, MID: 0x%.4x",
		readRegister(OV7670_REG_PID),
		readRegister(OV7670_REG_VER),
		readRegister(OV7670_REG_MIDH) << 8 | readRegister(OV7670_REG_MIDL));
	uint8_t com7 = readRegister(OV7670_REG_COM7);
	uint32_t outputFormat = getBit(com7, 2) << 1 | getBit(com7, 0);
	//uint32_t outputFormat = (com7 & (1<<2)) >> 1 | (com7 & (1<<0));
	std::string outputFormatString;
	switch (outputFormat) {
		case 0b00:
			outputFormatString = "YUV";
			break;
		case 0b10:
			outputFormatString = "RGB";
			break;
		case 0b01:
			outputFormatString = "Raw Bayer RGB";
			break;
		case 0b11:
			outputFormatString = "Process Bayer RGB";
			break;
		default:
			outputFormatString = "Unknown";
			break;
	}
	ESP_LOGD(LOG_TAG, "Output format: %s", outputFormatString.c_str());
	if (outputFormat == 0b10) {
		uint8_t com15 = readRegister(OV7670_REG_COM15);
		uint8_t rgbType = getBit(com15, 5) << 1 | getBit(com15, 4);
		char* rgbTypeString;
		switch (rgbType) {
			case 0b00:
			case 0b10:
				rgbTypeString = (char*) "Normal RGB Output";
				break;
			case 0b01:
				rgbTypeString = (char*) "RGB 565";
				break;
			case 0b11:
				rgbTypeString = (char*) "RGB 555";
				break;
			default:
				rgbTypeString = (char*) "Unknown";
				break;
		}
		ESP_LOGD(LOG_TAG, "Rgb Type: %s", rgbTypeString);
	}
	ESP_LOGD(LOG_TAG, "Color bar: %s", getBit(com7, 1) ? "Enabled" : "Disabled");

	uint8_t scaling_xsc = readRegister(OV7670_REG_SCALING_XSC);
	uint8_t scaling_ysc = readRegister(OV7670_REG_SCALING_YSC);
	uint32_t testPattern = getBit(scaling_xsc, 7) << 1 | getBit(scaling_ysc, 7);
	char* testPatternString;
	switch (testPattern) {
		case 0b00:
			testPatternString = (char*) "No test output";
			break;
		case 0b01:
			testPatternString = (char*) "Shifting 1";
			break;
		case 0b10:
			testPatternString = (char*) "8-bar color bar";
			break;
		case 0b11:
			testPatternString = (char*) "Fade to gray color bar";
			break;
		default:
			testPatternString = (char*) "Unknown";
			break;
	}
	ESP_LOGD(LOG_TAG, "Test pattern: %s", testPatternString);
	ESP_LOGD(LOG_TAG, "Horizontal scale factor: %d", scaling_xsc & 0x3f);
	ESP_LOGD(LOG_TAG, "Vertical scale factor: %d", scaling_ysc & 0x3f);
	uint8_t com15 = readRegister(OV7670_REG_COM15);
	switch ((com15 & 0b11000000) >> 6) {
		case 0b00:
		case 0b01:
			ESP_LOGD(LOG_TAG, "Output range: 0x10 to 0xf0");
			break;
		case 0b10:
			ESP_LOGD(LOG_TAG, "Output range: 0x01 to 0xfe");
			break;
		case 0b11:
			ESP_LOGD(LOG_TAG, "Output range: 0x00 to 0xff");
			break;
		default:
			break;
	}
} // dump

/*
static void log(char* marker) {
	ESP_LOGD(LOG_TAG, "%s", marker);
	FreeRTOS::sleep(100);
}
*/

/**
 * @brief Initialize the camera.
 */
void OV7670::init(camera_config_t cameraConfig) {
	ESP_LOGD(LOG_TAG, ">> init");
	m_cameraConfig = cameraConfig;

	// Define the GPIO pin directions.
	/*
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d0);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d1);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d2);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d3);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d4);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d5);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d6);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d7);

	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_vsync);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_href);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_pclk);
		*/
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_xclk);
	ESP32CPP::GPIO::setOutput(m_cameraConfig.pin_reset);


	// Reset the camera.
	reset();
	// provide a 20MHz clock on the pin_xclck
	camera_enable_out_clock(&m_cameraConfig);

	// Create the I2C interface.
	m_i2c = new I2C();
	m_i2c->init(OV7670_I2C_ADDR, (gpio_num_t) m_cameraConfig.pin_sscb_sda, (gpio_num_t) m_cameraConfig.pin_sscb_scl);
	m_i2c->scan();
	ESP_LOGD(LOG_TAG, "Do you see 0x21 listed?");
	resetCamera();
	FreeRTOS::sleep(100);
	resetCamera();
	setFormat(OV7670_FORMAT_RGB);
	setRGBFormat(OV7670_FORMAT_RGB_RGB_565);
	setTestPattern(OV7670_TESTPATTERN_GRAY_FADE);
	dump();

	// Setup the VSYNC interrupt handler
/*
  ESP32CPP::GPIO::addISRHandler(m_cameraConfig.pin_vsync, vsyncHandler, nullptr);
  ESP32CPP::GPIO::setInterruptType(m_cameraConfig.pin_vsync, GPIO_INTR_NEGEDGE);
  ESP32CPP::GPIO::interruptEnable(m_cameraConfig.pin_vsync);


  ESP32CPP::GPIO::addISRHandler(m_cameraConfig.pin_href, hrefHandler, nullptr);
  ESP32CPP::GPIO::setInterruptType(m_cameraConfig.pin_href, GPIO_INTR_NEGEDGE);
  ESP32CPP::GPIO::interruptEnable(m_cameraConfig.pin_href);
*/


  //ESP32CPP::GPIO::addISRHandler(m_cameraConfig.pin_pclk, pclckHandler, nullptr);
  //ESP32CPP::GPIO::setInterruptType(m_cameraConfig.pin_pclk, GPIO_INTR_NEGEDGE);
  //ESP32CPP::GPIO::interruptEnable(m_cameraConfig.pin_pclk);

  /*
	gpio_isr_handle_t handle;
	esp_err_t err = ::gpio_isr_register(isr_vsync, nullptr, ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_IRAM, &handle);
	if (err != ESP_OK) {
		ESP_LOGD(LOG_TAG, "gpio_isr_register: %d", err);
	}
	*/

	I2S i2s;
	dma_config_t dmaConfig;
	dmaConfig.pin_d0	= cameraConfig.pin_d0;
	dmaConfig.pin_d1	= cameraConfig.pin_d1;
	dmaConfig.pin_d2	= cameraConfig.pin_d2;
	dmaConfig.pin_d3	= cameraConfig.pin_d3;
	dmaConfig.pin_d4	= cameraConfig.pin_d4;
	dmaConfig.pin_d5	= cameraConfig.pin_d5;
	dmaConfig.pin_d6	= cameraConfig.pin_d6;
	dmaConfig.pin_d7	= cameraConfig.pin_d7;
	dmaConfig.pin_href  = cameraConfig.pin_href;
	dmaConfig.pin_pclk  = cameraConfig.pin_pclk;
	dmaConfig.pin_vsync = cameraConfig.pin_vsync;
	i2s.cameraMode(dmaConfig, 50, 360 * 2);
	ESP_LOGD(LOG_TAG, "Waiting for data!");
	while (true) {
	  DMAData dmaData = i2s.waitForData();
//	  GeneralUtils::hexDump(dmaData.getData(), dmaData.getLength());
	  toGrayscale(dmaData.getData(), dmaData.getLength());
	  dmaData.free();
	}

	ESP_LOGD(LOG_TAG, "Waiting for positive edge on VSYNC");
	while (gpio_get_level(m_cameraConfig.pin_vsync) == 0) {
	  ;
	}
	while (gpio_get_level(m_cameraConfig.pin_vsync) != 0) {
	  ;
	}

	ESP_LOGD(LOG_TAG, "Got VSYNC");

	while (true) {
	  FreeRTOS::sleep(1000);
	  ESP_LOGD(LOG_TAG, "VSYNC Counter: %d, lastHref=%d, pclk=%d", vsyncCounter, lastHref, pclkCounter);
	}

	ESP_LOGD(LOG_TAG, "<< init");
} // init


/**
 * Read a register from the camera.
 * @param [in] reg The register to read.
 * @return The value of the register.
 */
uint8_t OV7670::readRegister(uint8_t reg) {
	uint8_t val;
	m_i2c->beginTransaction();
	m_i2c->write(reg);
	m_i2c->endTransaction();
	m_i2c->beginTransaction();
	m_i2c->read(&val, false);
	m_i2c->endTransaction();
	return val;
} // readRegister


/**
 * @brief Write a value to a camera register.
 * @param [in] reg The register to write.
 * @param [in] value The value to write into the register.
 * @return N/A.
 */
void OV7670::writeRegister(uint8_t reg, uint8_t value) {
	m_i2c->beginTransaction();
	m_i2c->write(reg);
	m_i2c->write(value);
	m_i2c->endTransaction();
} // writeRegister


/**
 * @brief Reset the camera.
 * Toggle the reset pin on the camera.
 */
void OV7670::reset() {
	// Reset the camera
	ESP_LOGD(LOG_TAG, "x1");
	ESP32CPP::GPIO::low(m_cameraConfig.pin_reset);
	FreeRTOS::sleep(10);
	ESP32CPP::GPIO::high(m_cameraConfig.pin_reset);
	FreeRTOS::sleep(10);
} // reset
