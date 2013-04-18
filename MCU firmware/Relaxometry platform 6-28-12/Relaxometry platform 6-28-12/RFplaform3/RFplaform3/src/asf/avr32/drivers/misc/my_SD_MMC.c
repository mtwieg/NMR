/*
 * my_SD_MMC.c
 *
 * Created: 2/29/2012 3:57:22 PM
 *  Author: mtwiegx1
 */ 
#include <asf.h>
#include <my_misc_functions.h>
#include <my_SD_MMC.h>
#include <sd_mmc_spi.h>

extern uint8_t dummyblock[512];

#define MMC_START_WRITE_BLOCK_MULTI                   25    //start a multiblock write cluster
#define MMC_STARTBLOCK_WRITE_MULTI_TOKEN		0xFC
#define MMC_END_TRANS							0xFD

#define MMC_START_READ_BLOCK_MULTI                   18    ///< start a multiblock read cluster
#define MMC_START_READ_BLOCK_SINGLE                   17
#define MMC_STARTBLOCK_READ_MULTI_TOKEN		0xFE
#define MMC_STOP_READ								12

uint8_t my_SD_SPI_command(uint8_t command, uint32_t arg)
{
  uint8_t retry;

  my_SD_SPI_write(SD_MMC_SPI, 0xFF);            // write dummy byte
  my_SD_SPI_write(SD_MMC_SPI, command | 0x40);  // send command
  my_SD_SPI_write(SD_MMC_SPI, arg>>24);         // send parameter
  my_SD_SPI_write(SD_MMC_SPI, arg>>16);
  my_SD_SPI_write(SD_MMC_SPI, arg>>8 );
  my_SD_SPI_write(SD_MMC_SPI, arg    );
  switch(command)
  {
      case MMC_GO_IDLE_STATE:
         my_SD_SPI_write(SD_MMC_SPI, 0x95);
         break;
      case MMC_SEND_IF_COND:
         my_SD_SPI_write(SD_MMC_SPI, 0x87);
         break;
      default:
         my_SD_SPI_write(SD_MMC_SPI, 0xff);
         break;
  }

  // end command
  // wait for response
  // if more than 8 retries, card has timed-out and return the received 0xFF
  retry = 0;
  r1    = 0xFF;
  while((r1 = my_SD_SPI_write_read(SD_MMC_SPI,0xFF)) == 0xFF)
  {
    retry++;
    if(retry > 10) break;
  }
  return r1;
}

bool my_SD_SPI_block_write_multi(const void *ram, uint32_t addr, uint32_t Nblocks)
{
	const uint8_t *_ram = ram;
	
	my_SPI_selectchip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
	if(card_type == SD_CARD_2_SDHC) {
    r1 = my_SD_SPI_command(MMC_START_WRITE_BLOCK_MULTI , addr>>9);
  } else {
    r1 = my_SD_SPI_command(MMC_START_WRITE_BLOCK_MULTI , addr);
  }
  
  if(r1 != 0x00)
  {
    my_SPI_deselectchip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
    return false;
  }
  my_SD_SPI_write(SD_MMC_SPI, 0xFF); 
  
  for(uint16_t i=0;i<Nblocks;i++)
  {
	  my_SD_SPI_write(SD_MMC_SPI,MMC_STARTBLOCK_WRITE_MULTI_TOKEN);
	 // write data
	 for(uint16_t j=0;j<MMC_SECTOR_SIZE;j++)
	{
		my_SD_SPI_write(SD_MMC_SPI,*_ram++);
	}

	my_SD_SPI_write(SD_MMC_SPI,0xFF);    // send CRC (field required but value ignored)
	my_SD_SPI_write(SD_MMC_SPI,0xFF);

	// read data response token
	r1 = my_SD_SPI_write_read(SD_MMC_SPI,0xFF);
	if( (r1&MMC_DR_MASK) != MMC_DR_ACCEPT)
	{
		my_SD_SPI_write(SD_MMC_SPI,0xFF);    // send dummy bytes
		my_SD_SPI_write(SD_MMC_SPI,0xFF);
		spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
		break;
		//return false;         // return ERROR byte
	}

	spi_write(SD_MMC_SPI,0xFF);    // send dummy bytes
	spi_write(SD_MMC_SPI,0xFF);
	
	r1=my_sd_mmc_spi_wait_not_busy_noCS(); //wait for busy WITHOUT CHANGING CS LINE
	if(r1==false)
	{
		break;
	}		
  }
  
  my_SD_SPI_write(SD_MMC_SPI,MMC_END_TRANS);
  
  my_sd_mmc_spi_wait_not_busy_noCS(); //wait for busy WITHOUT CHANGING CS LINE	
  
  // release chip select
  my_SPI_deselectchip(SD_MMC_SPI, SD_MMC_SPI_NPCS);  // unselect SD_MMC_SPI
  
  return true;
}

bool my_sd_mmc_spi_wait_not_busy_noCS(void)
{
  uint32_t retry;
  // Select the SD_MMC memory gl_ptr_mem points to
  retry = 0;
  while((r1 = sd_mmc_spi_send_and_read(0xFF)) != 0xFF)
  {
    retry++;
    if (retry == 200000)
    {
      spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
      return false;
    }
  }
  //spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
  return true;
}

bool my_SD_read_multiblock_PDCA(uint32_t SD_addr, uint32_t bank_addr, uint8_t totalblocks)
{
	
	//uint8_t *_ram = ram;
//	uint16_t  i;
    uint16_t  read_time_out;
//	unsigned short data_read;
	if (false == sd_mmc_spi_wait_not_busy())
    return false;
	
	spi_selectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
	
	 if(card_type == SD_CARD_2_SDHC) {
    r1 = sd_mmc_spi_command(MMC_START_READ_BLOCK_MULTI, SD_addr>>9);
  } else {
    r1 = sd_mmc_spi_command(MMC_START_READ_BLOCK_MULTI, SD_addr);
  }
  
   if (r1 != 0x00)
  {
    spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);  // unselect SD_MMC_SPI
    return false;
  }
  
while(totalblocks)
{
  
  read_time_out = 30000;
  while((r1 = sd_mmc_spi_send_and_read(0xFF)) == 0xFF)
  {
     read_time_out--;
     if (read_time_out == 0)   // TIME-OUT
     {
       spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS); // unselect SD_MMC_SPI
       return false;
     }
  }

  // check token
  if (r1 != MMC_STARTBLOCK_READ)
  {
    spi_write(SD_MMC_SPI,0xFF);
    spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);  // unselect SD_MMC_SPI
    return false;
  }
  
	pdca_disable(SPI0_RX_PDCA_CHANNEL);
	AVR32_PDCA.channel[SPI0_RX_PDCA_CHANNEL].mar=bank_addr;
	AVR32_PDCA.channel[SPI0_RX_PDCA_CHANNEL].tcr=MMC_SECTOR_SIZE;
	pdca_enable(SPI0_RX_PDCA_CHANNEL);
	
	AVR32_PDCA.channel[SPI0_TX_PDCA_CHANNEL].mar=(uint32_t)(&dummyblock[0]); //initialize SPI0 to write from dummy block
	AVR32_PDCA.channel[SPI0_TX_PDCA_CHANNEL].tcr=512;
	pdca_enable(SPI0_TX_PDCA_CHANNEL);
	while((AVR32_PDCA.channel[SPI0_TX_PDCA_CHANNEL].isr & AVR32_PDCA_TRC_MASK) == 0x0000); //wait for block write to complete
	while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TXEMPTY_MASK));
	bank_addr+=512;
	pdca_disable(SPI0_RX_PDCA_CHANNEL);
	pdca_disable(SPI0_TX_PDCA_CHANNEL);
  // load 16-bit CRC (ignored)
  spi_write(SD_MMC_SPI,0xFF);
  spi_write(SD_MMC_SPI,0xFF);

  // continue delivering some clock cycles
  spi_write(SD_MMC_SPI,0xFF);
  totalblocks--;
}  
  sd_mmc_spi_command(MMC_STOP_READ, 0);  //for some reason, R1b from this always is 0x7F, all error flags are set!  not sure why...
  // release chip select
  spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);  // unselect SD_MMC_SPI

  return true;   // Read done.
}

bool my_SD_read_experiment_PDCA(uint32_t SD_addr, uint32_t exp_addr, uint32_t bank_addr, uint32_t exp_size, uint8_t totalblocks)
{
    uint16_t  read_time_out;
	if (false == sd_mmc_spi_wait_not_busy())
    return false;
	
	spi_selectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
	
	 if(card_type == SD_CARD_2_SDHC) {
    r1 = sd_mmc_spi_command(MMC_START_READ_BLOCK_MULTI, SD_addr>>9);
  } else {
    r1 = sd_mmc_spi_command(MMC_START_READ_BLOCK_MULTI, SD_addr);
  }
  
   if (r1 != 0x00)
  {
    spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);  // unselect SD_MMC_SPI
    return false;
  }
  
	pdca_disable(SPI0_RX_PDCA_CHANNEL);
	AVR32_PDCA.channel[SPI0_RX_PDCA_CHANNEL].mar=exp_addr;
	AVR32_PDCA.channel[SPI0_RX_PDCA_CHANNEL].tcr=exp_size;
	AVR32_PDCA.channel[SPI0_RX_PDCA_CHANNEL].marr=bank_addr;
	AVR32_PDCA.channel[SPI0_RX_PDCA_CHANNEL].tcrr=(totalblocks*512-exp_size);
	
while(totalblocks)
{
  
  read_time_out = 30000;
  while((r1 = sd_mmc_spi_send_and_read(0xFF)) == 0xFF)
  {
     read_time_out--;
     if (read_time_out == 0)   // TIME-OUT
     {
       spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS); // unselect SD_MMC_SPI
       return false;
     }
  }

  // check token
  if (r1 != MMC_STARTBLOCK_READ)
  {
    spi_write(SD_MMC_SPI,0xFF);
    spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);  // unselect SD_MMC_SPI
    return false;
  }
  
	pdca_enable(SPI0_RX_PDCA_CHANNEL);
	AVR32_PDCA.channel[SPI0_TX_PDCA_CHANNEL].mar=(uint32_t)(&dummyblock[0]); //initialize SPI0 to write from dummy block
	AVR32_PDCA.channel[SPI0_TX_PDCA_CHANNEL].tcr=512;
	pdca_enable(SPI0_TX_PDCA_CHANNEL);
	while((AVR32_PDCA.channel[SPI0_TX_PDCA_CHANNEL].isr & AVR32_PDCA_TRC_MASK) == 0x0000); //wait for block write to complete
	while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TXEMPTY_MASK));
	bank_addr+=512;
	pdca_disable(SPI0_RX_PDCA_CHANNEL);
	pdca_disable(SPI0_TX_PDCA_CHANNEL);
  // load 16-bit CRC (ignored)
  spi_write(SD_MMC_SPI,0xFF);
  spi_write(SD_MMC_SPI,0xFF);

  // continue delivering some clock cycles
  spi_write(SD_MMC_SPI,0xFF);
  totalblocks--;
}  
  sd_mmc_spi_command(MMC_STOP_READ, 0);  //for some reason, R1b from this always is 0x7F, all error flags are set!  not sure why...

  // release chip select
  spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);  // unselect SD_MMC_SPI

  return true;   // Read done.
}

