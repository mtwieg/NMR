/*
 * SD_arbitration.h
 *
 * Created: 3/7/2012 12:56:40 PM
 *  Author: mtwiegx1
 */ 



//uint8_t check_busy_fast(void);

#define blocks_per_cluster 32
#define bytes_per_block 512
#define experiment_base_address 0
#define data_base_address 16384

static inline uint8_t check_busy_fast(void)
{
	//my_SPI_selectchip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
	//SD_MMC_SPI->cr=AVR32_SPI_CR_FLUSHFIFO_MASK;  //I DONT USE THE FIFO
	while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
	SD_MMC_SPI->tdr =0xFF;
	while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TXEMPTY_MASK));
	//my_SPI_deselectchip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
	return SD_MMC_SPI->rdr;
	}

uint8_t get_saved_r1(void);

uint8_t get_failure_cause(void);

bool did_data_fail(void);

void initialize_SD_arbitration(void);

void handle_SD_old(void);

void handle_SPI_arbitration(void);

void end_sequence_terminate(void);

void send_data_to_host_2(uint32_t SD_start_addr, uint8_t cluster_total);

void send_data_to_host(uint32_t SD_start_addr, uint8_t cluster_total);

void handle_SD(void);

void reset_SD_sink_ptr(void);

void handle_SD_failure(void);

void fill_cluster_with_0xFF(uint32_t addr, uint32_t Nblocks);

void SD_skip_clusters(uint16_t clusters);

void shadow_SD_sink_ptr(void);