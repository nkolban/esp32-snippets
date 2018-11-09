#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

/* Main header of binary image */
typedef struct {
    uint8_t magic;
    uint8_t segment_count;
    /* flash read mode (esp_image_spi_mode_t as uint8_t) */
    uint8_t spi_mode;
    /* flash frequency (esp_image_spi_freq_t as uint8_t) */
    uint8_t spi_speed: 4;
    /* flash chip size (esp_image_flash_size_t as uint8_t) */
    uint8_t spi_size: 4;
    uint32_t entry_addr;
    /* WP pin when SPI pins set via efuse (read by ROM bootloader, the IDF bootloader uses software to configure the WP
     * pin and sets this field to 0xEE=disabled) */
    uint8_t wp_pin;
    /* Drive settings for the SPI flash pins (read by ROM bootloader) */
    uint8_t spi_pin_drv[3];
    /* Reserved bytes in ESP32 additional header space, currently unused */
    uint8_t reserved[11];
    /* If 1, a SHA256 digest "simple hash" (of the entire image) is appended after the checksum. Included in image length. This digest
     * is separate to secure boot and only used for detecting corruption. For secure boot signed images, the signature
     * is appended after this (and the simple hash is included in the signed data). */
    uint8_t hash_appended;
} __attribute__((packed))  esp_image_header_t;

typedef struct {
    uint32_t load_addr;
    uint32_t data_len;
} esp_image_segment_header_t;


/**
 * @brief Return a memory area name given an address.
 * @param addr  The address to decode.
 * @return A string description of the memory area.
 */
static const char* area(uint32_t addr) {
	if (addr >= 0x3FF80000 && addr <= 0x3FF81FFF) {
		return "RTS Fast (8K)";
	}
	if (addr >= 0x3FF82000 && addr <= 0x3FF8FFFF) {
		return "Reserved";
	}
	if (addr >= 0x3FF90000 && addr <= 0x3FF9FFFF) {
		return "Internal ROM 1 (64K)";
	}
	if (addr >= 0x3FFA0000 && addr <= 0x3FFADFFF) {
		return "Reserved";
	}
	if (addr >= 0x3FFAE000 && addr <= 0x3FFDFFFF) {
		return "Internal SRAM 2 (200K)";
	}
	if (addr >= 0x3FFE0000 && addr <= 0x3FFFFFFF) {
		return "Internal SRAM 1 (128K)";
	}
	if (addr >= 0x40000000 && addr <= 0x40007FFF) {
		return "Internal ROM 0 0 (32K)";
	}
	if (addr >= 0x40008000 && addr <= 0x4005FFFF) {
		return "Internal ROM 0 (352K)";
	}
	if (addr >= 0x40060000 && addr <= 0x4006FFFF) {
		return "Reserved";
	}
	if (addr >= 0x40070000 && addr <= 0x4007FFFF) {
		return "Internal SRAM 0 (64K)";
	}
	if (addr >= 0x40080000 && addr <= 0x4009FFFF) {
		return "Internal SRAM 0 (128K)";
	}
	if (addr >= 0x400A0000 && addr <= 0x400AFFFF) {
		return "Internal SRAM 1 (64K)";
	}
	if (addr >= 0x400B0000 && addr <= 0x400B7FFF) {
		return "Internal SRAM 1 (32K)";
	}
	if (addr >= 0x400B8000 && addr <= 0x400BFFFF) {
		return "Internal SRAM 1 (32K)";
	}
	if (addr >= 0x400C0000 && addr <= 0x400C1FFF) {
		return "RTC FAST Memory (8K)";
	}
	if (addr >= 0x50000000 && addr <= 0x50001FFF) {
		return "RTC SLOW Memory (8K)";
	}
	if (addr >= 0x3F400000 && addr <= 0x3F7FFFFF) {
		return "External Memory (Data)";
	}
	if (addr >= 0x3F800000 && addr <= 0x3FBFFFFF) {
		return "External Memory (Data)";
	}
	if (addr >= 0x400C2000 && addr <= 0x40BFFFFF) {
		return "External Memory (Instruction)";
	}
	return "Un-described";
} //area


int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s ESP32_BIN_FILE.bin\n", argv[0]);
		return 0;
	}

	const char* fileName = argv[1];
	esp_image_header_t header;
	esp_image_segment_header_t imageSegmentHeader;

	FILE* file = fopen(fileName, "rb");
	if (file == nullptr) {
		printf("Failed to open file %s\n", fileName);
		return 0;
	}

	size_t count = fread(&header, sizeof(esp_image_header_t), 1, file);
	if (count != 1) {
		printf("Failed to read esp_image_header_t\n");
		return 0;
	}

	if (header.magic != 0xE9) {
		printf("Failed to find magic number (0xE9) in BIN file header.\n");
		return 0;
	}

	printf("Dump of ESP32 binary file: %s\n", fileName);
	printf("magic: 0x%x, segment_count: %d, entry_addr: 0x%x - %s, hash_appended: %d\n",
			header.magic, header.segment_count, header.entry_addr, area(header.entry_addr), header.hash_appended);

	printf("\n");
	printf("Seg | Start      | End        | Length            | Area\n");
	printf("----+------------+------------+-------------------+-------------------------------\n");
	for (int i=0; i<header.segment_count; i++) {
		count = fread(&imageSegmentHeader, sizeof(esp_image_segment_header_t),1, file);
		if (count != 1) {
			printf("Failed to read esp_image_segment_header_t\n");
			return 0;
		}

		printf("%3d | 0x%8.8x | 0x%8.8x | %6d (0x%6.6x) | %s", i,
				imageSegmentHeader.load_addr, imageSegmentHeader.load_addr + imageSegmentHeader.data_len,
				imageSegmentHeader.data_len, imageSegmentHeader.data_len, area(imageSegmentHeader.load_addr));
		printf("\n");
		fseek(file, imageSegmentHeader.data_len, SEEK_CUR);
	}

	fclose(file);

	return 0;
} // main
