/*
 * I2S.cpp
 *
 *  Created on: Jul 23, 2017
 *      Author: kolban
 */
#include <driver/periph_ctrl.h>
#include <string>
#include <sstream>
#include <iomanip>
#include "I2S.h"
#include "GPIO.h"
#include "FreeRTOS.h"
#include "GeneralUtils.h"

extern "C" {
	#include <soc/i2s_reg.h>
	#include <soc/i2s_struct.h>
}

#include <rom/lldesc.h>
#include <esp_intr_alloc.h>
#include <esp_log.h>

static const char* LOG_TAG = "I2S";

static intr_handle_t i2s_intr_handle;


/**
 * A representation of a DMA buffer.
 */
class DMABuffer {
public:
	DMABuffer();
	~DMABuffer();

	void       dump();
	uint32_t   getData(uint8_t* pData, uint32_t length);
	lldesc_t*  getDesc();
	uint32_t   getLength();
	DMABuffer* getNext();
	bool       isEoF();
	void       setNext(DMABuffer* pNext);

private:
	lldesc_t   m_desc;
	DMABuffer* m_pNext;
};


DMABuffer::DMABuffer() {
	m_desc.length = 0;
	m_desc.size   = 4092;
	m_desc.owner  = 1;
	m_desc.sosf   = 1;
	m_desc.offset = 0;
	m_desc.eof    = 0;
	m_desc.empty  = 0;
	m_desc.buf    = new uint8_t[m_desc.size];
	m_pNext       = nullptr;
} // DMABuffer


DMABuffer::~DMABuffer() {
	delete[] m_desc.buf;
} // ~DMABuffer


/**
 * @brief Dump the state of the buffer.
 * @return N/A
 */
void DMABuffer::dump() {
	std::ostringstream ss;
	ss << "size: "     << m_desc.size;
	ss << ", buf: 0x"  << std::hex << (uint32_t) m_desc.buf << std::dec;
	ss << ", length: " << m_desc.length;
	ss << ", offset: " << m_desc.offset;
	ss << ", sosf: "   << m_desc.sosf;
	ss << ", eof: "    << m_desc.eof;
	ss << ", owner: "  << m_desc.owner;
	ESP_LOGD(LOG_TAG, "Desc: %s", ss.str().c_str());
	int length = 100;
	if (length > m_desc.length) length = m_desc.length;
	GeneralUtils::hexDump((uint8_t*) m_desc.buf, length);
}

/**
 * @brief Populate a buffer of data with the DMA data.
 * @param [in] pData A pointer to data to be populated with the DMA data.
 * @param [in] length The size in bytes of the pData buffer.
 * @return The number of bytes actually copied.
 */
uint32_t DMABuffer::getData(uint8_t* pData, uint32_t length) {
	uint8_t* pBuf = (uint8_t*) m_desc.buf;
	if (length > getLength()) length = getLength();
	uint32_t i;
	// The descriptor buffer is filled with data that contains:
	// b1 00 b0 00  b3 00 b2 00  b5 00 b4 00  b7 00 b6 00
	// Our goal is to populate the passed in buffer with data of the form:
	// b0 b1 b2 b3  b4 b5 b6 b7 ...
	// The following alogrithm does that.
	for (i = 0; i < length; i += 2) {
		*pData = pBuf[2];
		pData++;
		*pData = pBuf[0];
		pData++;
		pBuf += 4;
	}
	return i;
} // getData


/**
 * @brief Get the underlying linked list descriptor.
 * @return The underlying linked list descriptor.
 */
lldesc_t* DMABuffer::getDesc() {
	return &m_desc;
} // getDesc


/**
 * @brief Get the number of populated bytes.
 * @return The number of populated bytes.
 */
uint32_t DMABuffer::getLength() {
	return m_desc.length / 4;
} // getLength


/**
 * @brief Get the next DMA Buffer.
 * @return The next DMA Buffer;
 */
DMABuffer* DMABuffer::getNext() {
	return m_pNext;
} // getNext


/**
 * @brief Have we received all the data we expected?
 * @return True if we have received all the data we expected.
 */
bool DMABuffer::isEoF() {
	return m_desc.eof != 0;
} // isEoF


/**
 * @brief Set the next DMA buffer in the chain.
 * @param [in] pNext The next DMA buffer in the chain.
 */
void DMABuffer::setNext(DMABuffer* pNext) {
	m_pNext = pNext;
	m_desc.empty = 0;
	m_desc.qe.stqe_next = pNext->getDesc();
} // setNext


I2S::I2S() {
	// TODO Auto-generated constructor stub
}

I2S::~I2S() {
	// TODO Auto-generated destructor stub
}

DMABuffer* pCurrentDMABuffer;
DMABuffer* pLastDMABuffer;


static void i2s_conf_reset() {
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

/*
static void IRAM_ATTR logI2SIntr() {
	std::ostringstream ss;
	ss << "IN_DONE: "         << I2S0.int_raw.in_done;
	ss << ", IN_DSCR_EMPTY: " << I2S0.int_raw.in_dscr_empty;
	ss << ", IN_DSCR_ERR: "   << I2S0.int_raw.in_dscr_err;
	ss << ", IN_ERR_EOF: "    << I2S0.int_raw.in_err_eof;
	ss << ", IN_SUC_EOF: "    << I2S0.int_raw.in_suc_eof;
	ESP_EARLY_LOGV(LOG_TAG, "I2S Intr: %s", ss.str().c_str());
}
*/

/*
static void IRAM_ATTR logDesc(lldesc_t* pDesc) {
	std::ostringstream ss;
	ss << "size: "     << pDesc->size;
	ss << ", buf: 0x" << std::hex << (uint32_t)pDesc->buf << std::dec;
	ss << ", length: " << pDesc->length;
	ss << ", offset: " << pDesc->offset;
	ss << ", sosf: "   << pDesc->sosf;
	ss << ", eof: "    << pDesc->eof;
	ss << ", owner: "  << pDesc->owner;
	ESP_EARLY_LOGV(LOG_TAG, "Desc: %s", ss.str().c_str());
}
*/


/**
 * @brief I2S DMA Interrupt handler
 */
static void IRAM_ATTR i2s_isr(void* arg) {
	I2S* pI2S = (I2S*) arg;
	//ESP_EARLY_LOGV(LOG_TAG, "I2S isr");
	pLastDMABuffer = pCurrentDMABuffer;
	//logI2SIntr();
	//logDesc(pCurrentDMABuffer->getDesc());
	pCurrentDMABuffer = pCurrentDMABuffer->getNext();
	I2S0.int_clr.val = I2S0.int_raw.val;
	pI2S->m_dmaSemaphore.giveFromISR();
}

/**
 * @brief EXPERIMENTAL
 */
void I2S::cameraMode(dma_config_t config, int desc_count, int sample_count) {
	ESP_LOGD(LOG_TAG, ">> cameraMode");
	ESP32CPP::GPIO::setInput(config.pin_d0);
	ESP32CPP::GPIO::setInput(config.pin_d1);
	ESP32CPP::GPIO::setInput(config.pin_d2);
	ESP32CPP::GPIO::setInput(config.pin_d3);
	ESP32CPP::GPIO::setInput(config.pin_d4);
	ESP32CPP::GPIO::setInput(config.pin_d5);
	ESP32CPP::GPIO::setInput(config.pin_d6);
	ESP32CPP::GPIO::setInput(config.pin_d7);
	//ESP32CPP::GPIO::setInput(config.pin_xclk);
	ESP32CPP::GPIO::setInput(config.pin_vsync);
	ESP32CPP::GPIO::setInput(config.pin_href);
	ESP32CPP::GPIO::setInput(config.pin_pclk);
	//ESP32CPP::GPIO::setOutput(config.pin_reset);

	const uint32_t const_high = 0x38;

	gpio_matrix_in(config.pin_d0,	   I2S0I_DATA_IN0_IDX, false);
	gpio_matrix_in(config.pin_d1,	   I2S0I_DATA_IN1_IDX, false);
	gpio_matrix_in(config.pin_d2,	   I2S0I_DATA_IN2_IDX, false);
	gpio_matrix_in(config.pin_d3,	   I2S0I_DATA_IN3_IDX, false);
	gpio_matrix_in(config.pin_d4,	   I2S0I_DATA_IN4_IDX, false);
	gpio_matrix_in(config.pin_d5,	   I2S0I_DATA_IN5_IDX, false);
	gpio_matrix_in(config.pin_d6,	   I2S0I_DATA_IN6_IDX, false);
	gpio_matrix_in(config.pin_d7,	   I2S0I_DATA_IN7_IDX, false);
	gpio_matrix_in(config.pin_vsync,	I2S0I_V_SYNC_IDX,   true);
	gpio_matrix_in(config.pin_href,	 I2S0I_H_SYNC_IDX,   false);
//	gpio_matrix_in(const_high,		  I2S0I_V_SYNC_IDX,   false);
//	gpio_matrix_in(const_high,		  I2S0I_H_SYNC_IDX,   false);
	gpio_matrix_in(const_high,		  I2S0I_H_ENABLE_IDX, false);
	gpio_matrix_in(config.pin_pclk,	 I2S0I_WS_IN_IDX,	false);

	// Enable and configure I2S peripheral
	periph_module_enable(PERIPH_I2S0_MODULE);
	// Toggle some reset bits in LC_CONF register
	// Toggle some reset bits in CONF register
	// Enable slave mode (sampling clock is external)

	i2s_conf_reset();

	// Switch on Slave mode.
	// I2S_CONF_REG -> I2S_RX_SLAVE_MOD
	// Set to 1 to enable slave mode.
	I2S0.conf.rx_slave_mod = 1;

	// Enable parallel mode
	// I2S_CONF2_REG -> I2S_LCD_END
	// Set to 1 to enable LCD mode.
	I2S0.conf2.lcd_en = 1;

	// Use HSYNC/VSYNC/HREF to control sampling
	// I2S_CONF2_REG -> I2S_CAMERA_EN
	// Set to 1 to enable camera mode.
	I2S0.conf2.camera_en = 1;


	// Configure clock divider
	I2S0.clkm_conf.clkm_div_a   = 1;
	I2S0.clkm_conf.clkm_div_b   = 0;
	I2S0.clkm_conf.clkm_div_num = 2;

	// I2S_FIFO_CONF_REG -> I2S_DSCR_EN
	// FIFO will sink data to DMA
	I2S0.fifo_conf.dscr_en = 1;

	// FIFO configuration
	// I2S_FIFO_CONF_REG -> RX_FIFO_MOD
	I2S0.fifo_conf.rx_fifo_mod          = 1; // 0-3???

	// I2S_FIFO_CONF_REG -> RX_FIFO_MOD_FORCE_EN
	I2S0.fifo_conf.rx_fifo_mod_force_en = 1;

	// I2S_CONF_CHAN_REG -> I2S_RX_CHAN_MOD
	I2S0.conf_chan.rx_chan_mod          = 1;

	// Clear flags which are used in I2S serial mode
	// I2S_SAMPLE_RATE_CONF_REG -> I2S_RX_BITS_MOD
	I2S0.sample_rate_conf.rx_bits_mod = 0;

	// I2S_CONF_REG -> I2S_RX_RIGHT_FIRST
	I2S0.conf.rx_right_first = 0;
	//I2S0.conf.rx_right_first = 0;

	// I2S_CONF_REG -> I2S_RX_MSB_RIGHT
	I2S0.conf.rx_msb_right   = 0;
	//I2S0.conf.rx_msb_right   = 1;

	// I2S_CONF_REG -> I2S_RX_MSB_SHIFT
	I2S0.conf.rx_msb_shift   = 0;
	//I2S0.conf.rx_msb_shift   = 1;

	// I2S_CONF_REG -> I2S_RX_MSB_MONO
	I2S0.conf.rx_mono        = 0;

	// I2S_CONF_REG -> I2S_RX_SHORT_SYNC
	I2S0.conf.rx_short_sync  = 0;

	I2S0.timing.val          = 0;


	ESP_LOGD(LOG_TAG, "Initializing %d descriptors", desc_count);
	DMABuffer* pFirst = new DMABuffer(); // TODO: POTENTIAL LEAK
	DMABuffer* pLast = pFirst;
	for (int i = 1; i < desc_count; i++) {
		DMABuffer* pNewDMABuffer = new DMABuffer();  // TODO: POTENTIAL LEAK
		pLast->setNext(pNewDMABuffer);
		pLast = pNewDMABuffer;
	}
	pLast->setNext(pFirst);
	pCurrentDMABuffer = pFirst;

	// I2S_RX_EOF_NUM_REG
	I2S0.rx_eof_num	  = sample_count;

	// I2S_IN_LINK_REG -> I2S_INLINK_ADDR
	I2S0.in_link.addr	= (uint32_t) pFirst;

	// I2S_IN_LINK_REG -> I2S_INLINK_START
	I2S0.in_link.start   = 1;

	I2S0.int_clr.val	 = I2S0.int_raw.val;
	I2S0.int_ena.val	 = 0;
	I2S0.int_ena.in_done = 1;

	// Register the interrupt handler.
	esp_intr_alloc(
			ETS_I2S0_INTR_SOURCE,
			ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_IRAM,
			&i2s_isr,
			this,
			&i2s_intr_handle);

	m_dmaSemaphore.take();
	// Start the interrupt handler
	esp_intr_enable(i2s_intr_handle);

	I2S0.conf.rx_start = 1;

	/*
	while(1) {
		m_dmaSemaphore.wait();
		uint32_t dataLength = pLastDMABuffer->getLength();
		ESP_LOGD(LOG_TAG, "Got a DMA buffer; length=%d", dataLength);
		//pLastDMABuffer->dump();
		uint8_t *pData = new uint8_t[dataLength];
		pLastDMABuffer->getData(pData, dataLength);
		GeneralUtils::hexDump(pData, dataLength);
		delete[] pData;
		m_dmaSemaphore.take();
	}
	*/

	ESP_LOGD(LOG_TAG, "<< cameraMode");
}


DMAData I2S::waitForData() {
	DMAData dmaData;
	m_dmaSemaphore.wait();
	dmaData.m_length = pLastDMABuffer->getLength();
	ESP_LOGD(LOG_TAG, "Got a DMA buffer; length=%d", dmaData.m_length);
	//pLastDMABuffer->dump();
	dmaData.m_pData = new uint8_t[dmaData.m_length];
	pLastDMABuffer->getData(dmaData.m_pData, dmaData.m_length);
	return dmaData;
}
