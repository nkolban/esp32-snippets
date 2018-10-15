/*
 * System.cpp
 *
 *  Created on: May 27, 2017
 *      Author: kolban
 */

#include "System.h"
#include <esp_system.h>
#include <soc/gpio_struct.h>
#include <stdio.h>

extern "C" {
#include <esp_heap_caps.h>
}

typedef volatile struct {
	union {
		struct {
			uint32_t mcu_oe:     1;
			uint32_t slp_sel:    1;
			uint32_t mcu_wpd:    1;
			uint32_t mcu_wpu:    1;
			uint32_t mcu_ie:     1;
			uint32_t mcu_drb:    2;
			uint32_t func_wpd:   1;
			uint32_t func_wpu:   1;
			uint32_t func_ie:    1;
			uint32_t func_drv:   2;
			uint32_t mcu_sel:    3;
			uint32_t reserved15: 17;
		};
		uint32_t val;
	};
} io_mux_reg_t;


typedef volatile struct {
	union {
		struct {
			uint32_t clk1: 4;
			uint32_t clk2: 4;
			uint32_t clk3: 4;
			uint32_t reserved12: 20;
		};
		uint32_t val;
	} pin_ctrl;

	// The 36 exposed pads.
	io_mux_reg_t pad_gpio36;     // GPIO36
	io_mux_reg_t pad_gpio37;     // GPIO37
	io_mux_reg_t pad_gpio38;     // GPIO38
	io_mux_reg_t pad_gpio39;     // GPIO39
	io_mux_reg_t pad_gpio34;     // GPIO34
	io_mux_reg_t pad_gpio35;     // GPIO35
	io_mux_reg_t pad_gpio32;     // GPIO32
	io_mux_reg_t pad_gpio33;     // GPIO33
	io_mux_reg_t pad_gpio25;     // GPIO25
	io_mux_reg_t pad_gpio26;     // GPIO26
	io_mux_reg_t pad_gpio27;     // GPIO27
	io_mux_reg_t pad_mtms;       // GPIO14
	io_mux_reg_t pad_mtdi;       // GPIO12
	io_mux_reg_t pad_mtck;       // GPIO13
	io_mux_reg_t pad_mtdo;       // GPIO15
	io_mux_reg_t pad_gpio2;      // GPIO2
	io_mux_reg_t pad_gpio0;      // GPIO0
	io_mux_reg_t pad_gpio4;      // GPIO4
	io_mux_reg_t pad_gpio16;     // GPIO16
	io_mux_reg_t pad_gpio17;     // GPIO17
	io_mux_reg_t pad_sd_data2;   // GPIO9
	io_mux_reg_t pad_sd_data3;   // GPIO10
	io_mux_reg_t pad_sd_cmd;     // GPIO11
	io_mux_reg_t pad_sd_clk;     // GPIO6
	io_mux_reg_t pad_sd_data0;   // GPIO7
	io_mux_reg_t pad_sd_data1;   // GPIO8
	io_mux_reg_t pad_gpio5;      // GPIO5
	io_mux_reg_t pad_gpio18;     // GPIO18
	io_mux_reg_t pad_gpio19;     // GPIO19
	io_mux_reg_t pad_gpio20;     // GPIO20
	io_mux_reg_t pad_gpio21;     // GPIO21
	io_mux_reg_t pad_gpio22;     // GPIO22
	io_mux_reg_t pad_u0rxd;      // GPIO3
	io_mux_reg_t pad_u0txd;      // GPIO1
	io_mux_reg_t pad_gpio23;     // GPIO23
	io_mux_reg_t pad_gpio24;     // GPIO24
} io_mux_dev_t;

static io_mux_dev_t* IO_MUX = (io_mux_dev_t*) 0x3ff49000;

static const io_mux_reg_t* io_mux_translate[] = {
		&IO_MUX->pad_gpio0,      // 0
		&IO_MUX->pad_u0txd,      // 1
		&IO_MUX->pad_gpio2,      // 2
		&IO_MUX->pad_u0rxd,      // 3
		&IO_MUX->pad_gpio4,      // 4
		&IO_MUX->pad_gpio5,      // 5
		&IO_MUX->pad_sd_clk,     // 6
		&IO_MUX->pad_sd_data0,   // 7
		&IO_MUX->pad_sd_data1,   // 8
		&IO_MUX->pad_sd_data2,   // 9
		&IO_MUX->pad_sd_data3,   // 10
		&IO_MUX->pad_sd_cmd,     // 11
		&IO_MUX->pad_mtdi,       // 12
		&IO_MUX->pad_mtck,       // 13
		&IO_MUX->pad_mtms,       // 14
		&IO_MUX->pad_mtdo,       // 15
		&IO_MUX->pad_gpio16,     // 16
		&IO_MUX->pad_gpio17,     // 17
		&IO_MUX->pad_gpio18,     // 18
		&IO_MUX->pad_gpio19,     // 19
		&IO_MUX->pad_gpio20,     // 20
		&IO_MUX->pad_gpio21,     // 21
		nullptr,                 // 28
		nullptr,                 // 29
		nullptr,                 // 30
		nullptr,                 // 31
		&IO_MUX->pad_gpio32,     // 32
		&IO_MUX->pad_gpio33,     // 33
		&IO_MUX->pad_gpio34,     // 34
		&IO_MUX->pad_gpio35,     // 35
		&IO_MUX->pad_gpio36,     // 36
		&IO_MUX->pad_gpio37,     // 37
		&IO_MUX->pad_gpio38,     // 38
		&IO_MUX->pad_gpio39     // 39
};

static const io_mux_reg_t* gpioToIoMux(int gpio) {
	return io_mux_translate[gpio];
}

System::System() {
	// TODO Auto-generated constructor stub

}

System::~System() {
	// TODO Auto-generated destructor stub
}

const static char* outSignalStrings[] = {
		"SPICLK_out", // 0
		"SPIQ_out", // 1
		"SPID_out", // 2
		"SPIHD_out", //3
		"SPIWP_out", // 4
		"SPICS0_out", // 5
		"SPICS1_out", // 6
		"SPICS2_out", // 7
		"HSPICLK_out",  // 8
		"HSPIQ_out", // 9
		"HSPID_out", // 10
		"HSPICS0_out", // 11
		"HSPIHD_out", // 12
		"HSPIWP_out", // 13
		"U0TXD_out", // 14
		"U0RTS_out", // 15
		"U0DTR_out", // 16
		"U1TXD_out", // 17
		"U1RTS_out", // 18
		"", // 19
		"", // 20
		"", // 21
		"", // 22
		"I2S0O_BCK_out", // 23
		"I2S1O_BCK_out", // 24
		"I2S0O_WS_out", // 25
		"I2S1O_WS_out", // 26
		"I2S0I_BCK_out", // 27
		"I2S0I_WS_out", // 28
		"I2CEXT0_SCL_out", // 29
		"I2CEXT0_SDA_out", // 30
		"sdio_tohost_int_out", // 31
		"pwm0_out0a", // 32
		"pwm0_out0b", // 33
		"pwm0_out1a", // 34
		"pwm0_out1b", // 35
		"pwm0_out2a", // 36
		"pwm0_out2b", //37
		"", // 38
		"", // 39
		"", // 40
		"", // 41
		"", // 42
		"", // 43
		"", // 44
		"", // 45
		"", // 46
		"", // 47
		"", // 48
		"", // 49
		"", // 50
		"", // 51
		"", // 52
		"", // 53
		"", // 54
		"", // 55
		"", // 56
		"", // 57
		"", // 58
		"", // 59
		"", // 60
		"HSPICS1_out", // 61
		"HSPICS2_out", // 62
		"VSPICLK_out_mux", // 63
		"VSPIQ_out", // 64
		"VSPID_out", // 65
		"VSPIHD_out", // 66
		"VSPIWP_out", // 67
		"VSPICS0_out", // 68
		"VSPICS1_out", // 69
		"VSPICS2_out", // 70
		"ledc_hs_sig_out0", // 71
		"ledc_hs_sig_out1", // 72
		"ledc_hs_sig_out2", // 73
		"ledc_hs_sig_out3", // 74
		"ledc_hs_sig_out4", // 75
		"ledc_hs_sig_out5", // 76
		"ledc_hs_sig_out6", // 77
		"ledc_hs_sig_out7", // 78
		"edc_ls_sig_out0", // 79
		"ledc_ls_sig_out1", // 80
		"ledc_ls_sig_out2", // 81
		"ledc_ls_sig_out3", // 82
		"ledc_ls_sig_out4", // 83
		"ledc_ls_sig_out5", // 84
		"ledc_ls_sig_out6", // 85
		"ledc_ls_sig_out7", // 86
		"rmt_sig_out0", // 87
		"rmt_sig_out1", // 88
		"rmt_sig_out2", // 89
		"rmt_sig_out3", // 90
		"rmt_sig_out4", // 91
		"rmt_sig_out5", // 92
		"rmt_sig_out6", // 93
		"rmt_sig_out7", // 94
		"I2CEXT1_SCL_out", // 95
		"I2CEXT1_SCL_out", // 96
		"host_ccmd_od_pullup_en_n", // 97
		"host_rst_n_1", // 98
		"host_rst_n_2", // 99
		"gpio_sd0_out", // 100
		"gpio_sd1_out", // 101
		"gpio_sd2_out", // 102
		"gpio_sd3_out", // 103
		"gpio_sd4_out", // 104
		"gpio_sd5_out", // 105
		"gpio_sd6_out", // 106
		"gpio_sd7_out", // 107
		"pwm1_out0a", // 108
		"pwm1_out0b", // 109
		"pwm1_out1a", // 110
		"pwm1_out1b", // 111
		"pwm1_out2a", // 112
		"pwm1_out2b", // 113
		"pwm2_out1h", // 114
		"pwm2_out1l", // 115
		"pwm2_out2h", // 116
		"pwm2_out2l", // 117
		"pwm2_out3h", // 118
		"pwm2_out3l", // 119
		"pwm2_out4h", // 120
		"pwm2_out4l", // 121
		"", // 122
		"", // 123
		"", // 124
		"", // 125
		"", // 126
		"", // 127
		"", // 128
		"", // 129
		"", // 130
		"", // 131
		"", // 132
		"", // 133
		"", // 134
		"", // 135
		"", // 136
		"", // 137
		"", // 138
		"", // 139
		"I2S0O_DATA_out0", // 140
		"I2S0O_DATA_out1", // 141
		"I2S0O_DATA_out2", // 142
		"I2S0O_DATA_out3", // 143
		"I2S0O_DATA_out4", // 144
		"I2S0O_DATA_out5", // 145
		"I2S0O_DATA_out6", // 146
		"I2S0O_DATA_out7", // 147
		"I2S0O_DATA_out8", // 148
		"I2S0O_DATA_out9", // 149
		"I2S0O_DATA_out10", // 150
		"I2S0O_DATA_out11", // 151
		"I2S0O_DATA_out12", // 152
		"I2S0O_DATA_out13", // 153
		"I2S0O_DATA_out14", // 154
		"I2S0O_DATA_out15", // 155
		"I2S0O_DATA_out16", // 156
		"I2S0O_DATA_out17", // 157
		"I2S0O_DATA_out18", // 158
		"I2S0O_DATA_out19", // 159
		"I2S0O_DATA_out20", // 160
		"I2S0O_DATA_out21", // 161
		"I2S0O_DATA_out22", // 162
		"I2S0O_DATA_out23", // 163
		"I2S1I_BCK_out", // 164
		"I2S1I_WS_out", // 165
		"I2S1O_DATA_out0", // 166
		"I2S1O_DATA_out1", // 167
		"I2S1O_DATA_out2", // 168
		"I2S1O_DATA_out3", // 169
		"I2S1O_DATA_out4", // 170
		"I2S1O_DATA_out5", // 171
		"I2S1O_DATA_out6", // 172
		"I2S1O_DATA_out7", // 173
		"I2S1O_DATA_out8", // 174
		"I2S1O_DATA_out9", // 175
		"I2S1O_DATA_out10", // 176
		"I2S1O_DATA_out11", // 177
		"I2S1O_DATA_out12", // 178
		"I2S1O_DATA_out13", // 179
		"I2S1O_DATA_out14", // 180
		"I2S1O_DATA_out15", // 181
		"I2S1O_DATA_out16", // 182
		"I2S1O_DATA_out17", // 183
		"I2S1O_DATA_out18", // 184
		"I2S1O_DATA_out19", // 185
		"I2S1O_DATA_out20", // 186
		"I2S1O_DATA_out21", // 187
		"I2S1O_DATA_out22", // 188
		"I2S1O_DATA_out23", // 189
		"pwm3_out1h", // 190
		"pwm3_out1l", // 191
		"pwm3_out2h", // 192
		"pwm3_out2l", // 193
		"pwm3_out3h", // 194
		"pwm3_out3l", // 195
		"pwm3_out4h", // 196
		"pwm3_out4l", // 197
		"U2TXD_out", // 198
		"U2RTS_out", // 199
		"emac_mdc_o", // 200
		"emac_mdo_o", // 201
		"emac_crs_o", // 202
		"emac_col_o", // 203
		"bt_audio0_irq", // 204
		"bt_audio1_irq", // 205
		"bt_audio2_irq", // 206
		"ble_audio0_irq", // 207
		"ble_audio1_irq", // 208
		"ble_audio2_irq", // 209
		"pcmfsync_out", // 210
		"pcmclk_out", // 211
		"pcmdout", // 212
		"ble_audio_sync0_p", // 213
		"ble_audio_sync1_p", // 214
		"ble_audio_sync2_p", // 215
		"", // 216
		"", // 217
		"", // 218
		"", // 219
		"", // 220
		"", // 221
		"", // 222
		"", // 223
		"sig_in_func224", // 224
		"sig_in_func225", // 225
		"sig_in_func226", // 226
		"sig_in_func227", // 227
		"sig_in_func228", // 228
		"", // 229
		"", // 230
		"", // 231
		"", // 232
		"", // 233
		"", // 234
		"", // 235
		"", // 236
		"", // 237
		"", // 238
		"", // 239
		"", // 240
		"", // 241
		"", // 242
		"", // 243
		"", // 244
		"", // 245
		"", // 246
		"", // 247
		"", // 248
		"", // 249
		"", // 250
		"", // 251
		"", // 252
		"", // 253
		"", // 254
		"", // 255
};


/**
 * Dump the mappings for GPIO pins.
 */
/* static */ void System::dumpPinMapping() {
	const int numPins = 40;
	printf("GPIO_FUNCn_OUT_SEL_CFG_REG\n");
	printf("--------------------------\n");
	printf("%3s %4s\n", "Pin", "Func");
	for (uint8_t i = 0; i < numPins; i++) {
		const char *signal;
		if (GPIO.func_out_sel_cfg[i].func_sel == 256) {
			signal = (char*) "[GPIO]";
		} else if (GPIO.func_out_sel_cfg[i].func_sel == 257) {
			signal = (char*) "N/A";
		} else {
			signal = outSignalStrings[GPIO.func_out_sel_cfg[i].func_sel];
		}
		printf("%2d %4d %s\n", i, GPIO.func_out_sel_cfg[i].func_sel, signal);
		const io_mux_reg_t* io_mux = gpioToIoMux(i);
		if (GPIO.func_out_sel_cfg[i].func_sel == 256 && io_mux != nullptr) {
			printf("0x%x - function: %d, ie: %d\n", (uint32_t) io_mux, io_mux->mcu_sel + 1, io_mux->func_ie);
		}
	}

} // System#dumpPinMapping


/**
 * Dump the storage stats for the heap.
 */
/* static */ void System::dumpHeapInfo() {
	multi_heap_info_t heapInfo;

	printf("         %10s %10s %10s %10s %13s %11s %12s\n", "Free", "Allocated", "Largest", "Minimum", "Alloc Blocks", "Free Blocks", "Total Blocks");
	heap_caps_get_info(&heapInfo, MALLOC_CAP_EXEC);
	printf("EXEC     %10d %10d %10d %10d %13d %11d %12d\n", heapInfo.total_free_bytes, heapInfo.total_allocated_bytes, heapInfo.largest_free_block, heapInfo.minimum_free_bytes, heapInfo.allocated_blocks, heapInfo.free_blocks, heapInfo.total_blocks);
	heap_caps_get_info(&heapInfo, MALLOC_CAP_32BIT);
	printf("32BIT    %10d %10d %10d %10d %13d %11d %12d\n", heapInfo.total_free_bytes, heapInfo.total_allocated_bytes, heapInfo.largest_free_block, heapInfo.minimum_free_bytes, heapInfo.allocated_blocks, heapInfo.free_blocks, heapInfo.total_blocks);
	heap_caps_get_info(&heapInfo, MALLOC_CAP_8BIT);
	printf("8BIT     %10d %10d %10d %10d %13d %11d %12d\n", heapInfo.total_free_bytes, heapInfo.total_allocated_bytes, heapInfo.largest_free_block, heapInfo.minimum_free_bytes, heapInfo.allocated_blocks, heapInfo.free_blocks, heapInfo.total_blocks);
	heap_caps_get_info(&heapInfo, MALLOC_CAP_DMA);
	printf("DMA      %10d %10d %10d %10d %13d %11d %12d\n", heapInfo.total_free_bytes, heapInfo.total_allocated_bytes, heapInfo.largest_free_block, heapInfo.minimum_free_bytes, heapInfo.allocated_blocks, heapInfo.free_blocks, heapInfo.total_blocks);
	heap_caps_get_info(&heapInfo, MALLOC_CAP_SPIRAM);
	printf("SPISRAM  %10d %10d %10d %10d %13d %11d %12d\n", heapInfo.total_free_bytes, heapInfo.total_allocated_bytes, heapInfo.largest_free_block, heapInfo.minimum_free_bytes, heapInfo.allocated_blocks, heapInfo.free_blocks, heapInfo.total_blocks);
	heap_caps_get_info(&heapInfo, MALLOC_CAP_INTERNAL);
	printf("INTERNAL %10d %10d %10d %10d %13d %11d %12d\n", heapInfo.total_free_bytes, heapInfo.total_allocated_bytes, heapInfo.largest_free_block, heapInfo.minimum_free_bytes, heapInfo.allocated_blocks, heapInfo.free_blocks, heapInfo.total_blocks);
	heap_caps_get_info(&heapInfo, MALLOC_CAP_DEFAULT);
	printf("DEFAULT  %10d %10d %10d %10d %13d %11d %12d\n", heapInfo.total_free_bytes, heapInfo.total_allocated_bytes, heapInfo.largest_free_block, heapInfo.minimum_free_bytes, heapInfo.allocated_blocks, heapInfo.free_blocks, heapInfo.total_blocks);
} // System#dumpHeapInfo


/**
 * @brief Get the information about the device.
 * @param [out] info The structure to be populated on return.
 * @return N/A.
 */
void System::getChipInfo(esp_chip_info_t* info) {
	::esp_chip_info(info);
} // getChipInfo


/**
 * @brief Retrieve the system wide free heap size.
 * @return The system wide free heap size.
 */
size_t System::getFreeHeapSize() {
	return heap_caps_get_free_size(MALLOC_CAP_8BIT);
} // getFreeHeapSize


/**
 * @brief Retrieve the version of the ESP-IDF.
 * When an application is compiled, it is compiled against a version of the ESP-IDF.
 * This function returns that version.
 */
std::string System::getIDFVersion() {
	return std::string(::esp_get_idf_version());
} // getIDFVersion


/**
 * @brief Get the smallest heap size seen.
 * @return The smallest heap size seen.
 */
size_t System::getMinimumFreeHeapSize() {
	return heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
} // getMinimumFreeHeapSize


/**
 * @brief Restart the ESP32.
 */
void System::restart() {
	esp_restart();
} // restart
