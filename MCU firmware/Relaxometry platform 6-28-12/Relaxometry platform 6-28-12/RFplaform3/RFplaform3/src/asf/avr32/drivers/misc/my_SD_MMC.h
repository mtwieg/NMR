/*
 * my_SD_MMC.h
 *
 * Created: 2/29/2012 3:57:03 PM
 *  Author: mtwiegx1
 */ 
inline void my_SD_SPI_write(volatile avr32_spi_t *spi, uint8_t data);

inline uint8_t my_SD_SPI_write_read(volatile avr32_spi_t *spi, uint8_t data);

uint8_t my_SD_SPI_command(uint8_t command, uint32_t arg);

bool my_SD_SPI_block_write_multi(const void *ram, uint32_t addr, uint32_t Nblocks);

bool my_SD_read_multiblock_PDCA(uint32_t SD_addr, uint32_t bank_addr, uint8_t totalblocks);

bool my_SD_read_experiment_PDCA(uint32_t SD_addr, uint32_t exp_addr, uint32_t bank_addr, uint32_t exp_size, uint8_t totalblocks);
