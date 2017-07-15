/*
 * OV7670.cpp
 *
 *  Created on: Jun 10, 2017
 *      Author: kolban
 */

#include "OV7670.h"
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <GPIO.h>
#include <FreeRTOS.h>
#include "sdkconfig.h"
#include <esp_log.h>
extern "C" {
	#include <soc/i2s_reg.h>
	#include <soc/i2s_struct.h>
}

static char tag[]="OV7670";

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

#define OV7670_I2C_ADDR (0x21)

static void IRAM_ATTR isr_vsync(void* arg) {
	ESP_EARLY_LOGD(tag, "VSYNC");
}


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

OV7670::OV7670() {
	m_i2c = nullptr;
}


OV7670::~OV7670() {
	if (m_i2c != nullptr) {
		delete m_i2c;
		m_i2c = nullptr;
	}
}


static esp_err_t camera_enable_out_clock(camera_config_t* config)
{
    periph_module_enable(PERIPH_LEDC_MODULE);

    ledc_timer_config_t timer_conf;
    timer_conf.bit_num = (ledc_timer_bit_t)1;
    timer_conf.freq_hz = config->xclk_freq_hz;
    timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    timer_conf.timer_num = config->ledc_timer;
    esp_err_t err = ledc_timer_config(&timer_conf);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "ledc_timer_config failed, rc=%x", err);
        return err;
    }

    ledc_channel_config_t ch_conf;
    ch_conf.channel = config->ledc_channel;
    ch_conf.timer_sel = config->ledc_timer;
    ch_conf.intr_type = LEDC_INTR_DISABLE;
    ch_conf.duty = 1;
    ch_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    ch_conf.gpio_num = config->pin_xclk;
    err = ledc_channel_config(&ch_conf);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "ledc_channel_config failed, rc=%x", err);
        return err;
    }
    return ESP_OK;
} // camera_enable_out_clock

/*
static void i2s_init() {
	// Enable and configure I2S peripheral
	periph_module_enable(PERIPH_I2S0_MODULE);
	// Toggle some reset bits in LC_CONF register
	// Toggle some reset bits in CONF register
	// Enable slave mode (sampling clock is external)

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
	ESP_LOGD(tag, "PID: 0x%.2x, VER: 0x%.2x, MID: 0x%.4x",
		readRegister(OV7670_REG_PID),
		readRegister(OV7670_REG_VER),
		readRegister(OV7670_REG_MIDH) << 8 | readRegister(OV7670_REG_MIDL));
} // dump


/**
 * @brief Initialize the camera.
 */
void OV7670::init(camera_config_t cameraConfig) {
	m_cameraConfig = cameraConfig;

	// Define the GPIO pin directions.
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d0);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d1);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d2);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d3);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d4);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d5);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d6);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_d7);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_xclk);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_vsync);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_href);
	ESP32CPP::GPIO::setInput(m_cameraConfig.pin_pclk);
	ESP32CPP::GPIO::setOutput(m_cameraConfig.pin_reset);

	// Reset the camera.
	reset();

	// provide a 20MHz clock on the pin_xclck
	camera_enable_out_clock(&m_cameraConfig);

	// Create the I2C interface.
	m_i2c = new I2C();
	m_i2c->init(OV7670_I2C_ADDR, (gpio_num_t)m_cameraConfig.pin_sscb_sda, (gpio_num_t)m_cameraConfig.pin_sscb_scl);
	m_i2c->scan();
	ESP_LOGD(tag, "Do you see 0x21 listed?");
	dump();

	// Setup the VSYNC interrupt handler
  ESP32CPP::GPIO::setInterruptType(m_cameraConfig.pin_vsync, GPIO_INTR_NEGEDGE);
  ESP32CPP::GPIO::interruptEnable(m_cameraConfig.pin_vsync);
	gpio_isr_handle_t handle;
	esp_err_t err = ::gpio_isr_register(isr_vsync, nullptr, ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_IRAM, &handle);
	if (err != ESP_OK) {
		ESP_LOGD(tag, "gpio_isr_register: %d", err);
	}

  ESP_LOGD(tag, "Waiting for positive edge on VSYNC");
  while (gpio_get_level(m_cameraConfig.pin_vsync) == 0) {
      ;
  }
  while (gpio_get_level(m_cameraConfig.pin_vsync) != 0) {
      ;
  }
  ESP_LOGD(tag, "Got VSYNC");

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
	ESP32CPP::GPIO::low(m_cameraConfig.pin_reset);
	FreeRTOS::sleep(10);
	ESP32CPP::GPIO::high(m_cameraConfig.pin_reset);
	FreeRTOS::sleep(10);
}
