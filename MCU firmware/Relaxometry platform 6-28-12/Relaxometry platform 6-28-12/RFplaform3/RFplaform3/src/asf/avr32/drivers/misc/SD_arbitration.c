/*
 * SD_arbitration.c
 *
 * Created: 3/6/2012 3:39:20 PM
 *  Author: mtwiegx1
 */ 
#include <asf.h>
#include <sequenceparameters.h>
#include <my_misc_functions.h>
#include <SD_arbitration.h>
#include <my_SD_MMC.h>

#define MMC_START_WRITE_BLOCK_MULTI                   25    //start a multiblock write cluster
#define MMC_STARTBLOCK_WRITE_MULTI_TOKEN		0xFC
#define MMC_END_TRANS							0xFD

#define MMC_START_READ_BLOCK_MULTI                   18    ///< start a multiblock read cluster
#define MMC_START_READ_BLOCK_SINGLE                   17
#define MMC_STARTBLOCK_READ_MULTI_TOKEN		0xFE
#define MMC_STOP_READ								12

extern uint8_t dummyblock[512];
extern t_experiment_t t_experiment;

extern uint8_t bank0[samplebuffer_size];
extern uint8_t bank1[samplebuffer_size];

#define transfer_state_SD_busy false
#define transfer_state_SPI_TXing	true
#define bank0sink_bank1source 1
#define bank1sink_bank0source 2

#define FAIL_nofail 0;
#define FAIL_badblock 1;
#define FAIL_badr1_clusterwrite 2;
#define FAIL_overrun 3;

uint16_t save_r1=0;

uint8_t failure_cause=FAIL_nofail;

bool SD_transfer_in_progress=false;
bool SD_transfer_state=transfer_state_SD_busy;
uint8_t bank_switch=bank0sink_bank1source;
bool new_ADC_data_ready=false;
bool data_fail=false;

uint32_t temprdr;

uint32_t SD_sink_ptr;		//sink address of SD card (in bytes), this is not a pointer because....
uint32_t SD_sink_ptr_shadow;
uint32_t temp_SD_sink_ptr;
uint32_t SD_source_ptr;
uint16_t ADC_byte_count;	//counts number of bytes transferred on ADC SPI channel, used to tell when sink bank is filled up
uint16_t block_count; //counts number of blocks transferred so far within a cluster

void initialize_SD_arbitration(void)
{
	bank_switch=bank0sink_bank1source;
	ADC_byte_count=0;
	block_count=0;
	SD_source_ptr=(uint32_t)&(bank1[0]);
	data_fail=false;
	SD_transfer_in_progress=false;
	SD_transfer_state=transfer_state_SD_busy;
	save_r1=0;
	failure_cause=FAIL_nofail;
	gpio_local_clr_gpio_pin(PWM0_pin);
}

void reset_SD_sink_ptr(void)
{
	SD_sink_ptr=data_base_address;
}

bool did_data_fail(void)
{
	return data_fail;
}

uint8_t get_failure_cause(void)
{
	return failure_cause;
}

uint8_t get_saved_r1(void)
{
	return (uint8_t)save_r1;
}

void handle_SD(void)  //arbitration handled after TX pulse, before sampling period
{//this version is updated to have extra branch so it's possible to transfer 1 block every echo
	if(data_fail==true)	//do nothing if data failure has already occured
	{
		return;
	}
	//first should check whether data overrun has already occured...
	if(SD_transfer_in_progress==true)
	{//SD transfer is in progress true
		if(SD_transfer_state==transfer_state_SD_busy)
		{//transfer state is SD_busy
			if(check_busy_fast() == 0xFF)
			{//SD card is idle
				if(block_count==blocks_per_cluster)//check if last block of cluster was just sent
				{//yes, it was the last block
					block_count=0;
					while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
					SD_MMC_SPI->tdr =MMC_END_TRANS;	//send END_TRANS token
					my_SPI_deselectchip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
					new_ADC_data_ready=false;	//clear new_ADC_data_ready flag to indicate that the ADC data has been completely written
					SD_transfer_in_progress=false;
					SD_sink_ptr+=blocks_per_cluster*bytes_per_block;
				}
				else
				{//no, it was not the last block
					block_count++;
					while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
					SD_MMC_SPI->tdr =MMC_STARTBLOCK_WRITE_MULTI_TOKEN;	//send start of multiblock block token
					AVR32_PDCA.channel[SPI0_TX_PDCA_CHANNEL].mar=SD_source_ptr;
					AVR32_PDCA.channel[SPI0_TX_PDCA_CHANNEL].tcr=bytes_per_block+2;	//this will start the transfer as well
					SD_transfer_state=transfer_state_SPI_TXing;
				}
			}
		}
		else
		{//transfer state is SPI_TXing
			if((AVR32_PDCA.channel[SPI0_TX_PDCA_CHANNEL].isr & AVR32_PDCA_TRC_MASK) != 0x0000)	//if TCR=0... maybe it would be faster to just read the TCR register?
			{//if PDCA is done sending block...
				while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TXEMPTY_MASK)); //wait until last transfer is done
				SD_MMC_SPI->tdr =0xFF;
				while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TXEMPTY_MASK));
				save_r1=SD_MMC_SPI->rdr;
				//if( (SD_MMC_SPI->rdr & MMC_DR_MASK) != MMC_DR_ACCEPT)
				if( (save_r1 & MMC_DR_MASK) != MMC_DR_ACCEPT)
				{//block NOT accepted
					while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
					SD_MMC_SPI->tdr =0xFF;	//write two dummies
					while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
					SD_MMC_SPI->tdr =0xFF;	//not sure what these dummies do....
					data_fail=true;
					failure_cause=FAIL_badblock;
					my_SPI_deselectchip(SD_MMC_SPI, SD_MMC_SPI_NPCS); //then deselect
				}
				else
				{//block IS accepted
					while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
					SD_MMC_SPI->tdr =0xFF;	//write two dummies
					while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
					SD_MMC_SPI->tdr =0xFF;	//not sure what these dummies do....
					SD_transfer_state=transfer_state_SD_busy;
					SD_source_ptr+=512;
					
					if(check_busy_fast() == 0xFF)
					{
						if(block_count==blocks_per_cluster)//check if last block of cluster was just sent
						{//yes, it was the last block
							block_count=0;
							while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
							SD_MMC_SPI->tdr =MMC_END_TRANS;	//send END_TRANS token
							my_SPI_deselectchip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
							new_ADC_data_ready=false;	//clear new_ADC_data_ready flag to indicate that the ADC data has been completely written
							SD_transfer_in_progress=false;
					
							SD_sink_ptr+=blocks_per_cluster*bytes_per_block;
						}
						else
						{//no, it was not the last block
							block_count++;
							while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
							SD_MMC_SPI->tdr =MMC_STARTBLOCK_WRITE_MULTI_TOKEN;	//send start of multiblock block token
							AVR32_PDCA.channel[SPI0_TX_PDCA_CHANNEL].mar=SD_source_ptr;
							AVR32_PDCA.channel[SPI0_TX_PDCA_CHANNEL].tcr=bytes_per_block+2;	//this will start the transfer as well
							//pdca_load_channel(SPI0_TX_PDCA_CHANNEL, SD_source_ptr, bytes_per_block+2);
							SD_transfer_state=transfer_state_SPI_TXing;
						}
					}
				}
			}
		}
	}
	else
	{//SD transfer not in progress
		if(new_ADC_data_ready==true)
		{//there is new ADC data ready
			my_SPI_selectchip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
			if(check_busy_fast() == 0xFF) //not sure if it's necessary to check if SD card is busy here....
			{//SD card is idle
				while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
				SD_MMC_SPI->tdr=(MMC_START_WRITE_BLOCK_MULTI | 0x40);  // send command
				temp_SD_sink_ptr=((card_type==SD_CARD_2_SDHC) ? (SD_sink_ptr>>9) : (SD_sink_ptr));
				while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
				SD_MMC_SPI->tdr=temp_SD_sink_ptr>>24;				//send ARG		need to make sure that I have some SDHC compatibility!!!
				while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
				SD_MMC_SPI->tdr=temp_SD_sink_ptr>>16;
				while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
				SD_MMC_SPI->tdr=temp_SD_sink_ptr>>8;
				while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
				SD_MMC_SPI->tdr=temp_SD_sink_ptr;
				while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
				SD_MMC_SPI->tdr=0xFF;	//dummy CRC
				
				//get r1
				uint8_t retry=0;
				while((r1 = my_SD_SPI_write_read(SD_MMC_SPI,0xFF)) == 0xFF)
				{
					retry++;
					if(retry > 3) break;
				}
				
				if(r1 != 0x00)
				{	//r1 was not valid
					my_SPI_deselectchip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
					data_fail=true;
					failure_cause=FAIL_badr1_clusterwrite;
					save_r1=r1;
					
				}
				else
				{	//r1 was valid
					while (!(SD_MMC_SPI->sr & AVR32_SPI_SR_TDRE_MASK));
					SD_MMC_SPI->tdr=MMC_STARTBLOCK_WRITE_MULTI_TOKEN; //write start multiblock write token
					
					if(bank_switch==bank0sink_bank1source)
					{
						SD_source_ptr=(uint32_t)&bank1[0];
					}
					else
					{
						SD_source_ptr=(uint32_t)&bank0[0];
					}
					
					pdca_load_channel(SPI0_TX_PDCA_CHANNEL, (void *)SD_source_ptr, bytes_per_block+2);
				
					block_count++;
					SD_transfer_in_progress=true;
					SD_transfer_state=transfer_state_SPI_TXing;
				}
				
				
			}
		}
	}
	
}

void handle_SPI_arbitration(void)		//done after sampling routing
{
	ADC_byte_count+=t_currentsequence.CPMG.Nsamp*4;
	if(ADC_byte_count>=samplebuffer_size)
	{	//ADC sample buffer has finished writing a buffer and has automatically started writing the other bank
		//bank_switch= ~bank_switch;	//toggle bank switch
		
		(bank_switch==bank0sink_bank1source) ? (AVR32_PDCA.channel[SPI1_RX_PDCA_CHANNEL].marr=(uint32_t)&bank0[0]) : (AVR32_PDCA.channel[SPI1_RX_PDCA_CHANNEL].marr=(uint32_t)&bank1[0]);
		
		if(bank_switch==bank0sink_bank1source)
		{
			bank_switch=bank1sink_bank0source;
		}
		else
		{
			bank_switch=bank0sink_bank1source;
		}
		
		while (!(SPI1->sr & AVR32_SPI_SR_TXEMPTY_MASK));// make sure last transfer has actually finished
		AVR32_PDCA.channel[SPI1_RX_PDCA_CHANNEL].tcrr=samplebuffer_size/2; //there are two bytes per SPI1 transfer
		ADC_byte_count-=samplebuffer_size;	//subtract buffer size from byte counter
		if(new_ADC_data_ready==true)
		{
			data_fail=true;
			failure_cause=FAIL_overrun;
			gpio_local_set_gpio_pin(PWM0_pin);
		}
		new_ADC_data_ready=true;
	}
}

void end_sequence_terminate(void) //done normally at end of sequence.  Finishes writing any remaining data
{
	ADC_byte_count+=(t_experiment.Noffsetsamples+1)*4;  //need to add in offset samples, plus dummy conversion!
	bool extra_ADC_data=false;
	if(ADC_byte_count>=samplebuffer_size)
	{	//ADC sample buffer has finished writing a buffer and has automatically started writing the other bank
		extra_ADC_data=true;	//set flag indicating that sink bank overflowed during offset samples, and that another cluster will need to be sent to collect it
		ADC_byte_count-=samplebuffer_size;	//subtract buffer size from byte counter
		if(new_ADC_data_ready==true)
		{
			data_fail=true;
			failure_cause=FAIL_overrun;
			gpio_local_set_gpio_pin(PWM0_pin);
		}
		new_ADC_data_ready=true;
	}
	
	while(SD_transfer_in_progress==true)	//wait for any ongoing SD cluster to finish
	{
		handle_SD();
	}
	
	new_ADC_data_ready=true; //there will be new data no matter what, due to offset samples (I am setting this true here so it is not cleared by finishing of previous SD_arb routine)
	if(bank_switch==bank0sink_bank1source)
		{
			bank_switch=bank1sink_bank0source;
		}
		else
		{
			bank_switch=bank0sink_bank1source;
		}
		
	while(new_ADC_data_ready==true)	//wait for that one to finish too...
	{
		handle_SD();
	}
	
	new_ADC_data_ready=extra_ADC_data;	//if there is extra ADC data, then need to do one final cluster
	if(bank_switch==bank0sink_bank1source)
		{
			bank_switch=bank1sink_bank0source;
		}
		else
		{
			bank_switch=bank0sink_bank1source;
		}
	
	while(new_ADC_data_ready==true)	//wait for that one to finish too...
	{
		handle_SD();
	}
	
	my_SPI_deselectchip(SD_MMC_SPI, SD_MMC_SPI_NPCS); //then deselect
}

void send_data_to_host(uint32_t SD_start_addr, uint8_t cluster_total)
{
	uint32_t SD_addr=SD_start_addr;
	my_pdca_init_channel(USB_USART_TX_PDCA_CHANNEL,0,0,USB_USART_TX_PDCA_PID,0,0,PDCA_TRANSFER_SIZE_BYTE);
	pdca_enable(USB_USART_TX_PDCA_CHANNEL);

	my_SD_read_multiblock_PDCA(SD_addr,(uint32_t)(&bank0[0]), blocks_per_cluster);	//must read a cluster from SD before UART can send it
	SD_addr+=blocks_per_cluster*bytes_per_block;
	bank_switch=bank1sink_bank0source;
	
	for(uint8_t clustercount=1;clustercount<cluster_total;clustercount++)
	{	
		if(bank_switch==bank0sink_bank1source)
		{
			pdca_load_channel(USB_USART_TX_PDCA_CHANNEL,&bank1[0],samplebuffer_size);
			my_SD_read_multiblock_PDCA(SD_addr,(uint32_t)(&bank0[0]), blocks_per_cluster);
			SD_addr+=blocks_per_cluster*bytes_per_block;
			while((pdca_get_transfer_status(USB_USART_TX_PDCA_CHANNEL) & AVR32_PDCA_TRC_MASK) == 0x00000000);
			bank_switch=bank1sink_bank0source;
		}
		else
		{
			pdca_load_channel(USB_USART_TX_PDCA_CHANNEL,&bank0[0],samplebuffer_size);
			my_SD_read_multiblock_PDCA(SD_addr,(uint32_t)(&bank1[0]), blocks_per_cluster);
			SD_addr+=blocks_per_cluster*bytes_per_block;
			while((pdca_get_transfer_status(USB_USART_TX_PDCA_CHANNEL) & AVR32_PDCA_TRC_MASK) == 0x00000000);
			bank_switch=bank0sink_bank1source;
		}
		
	}
	
	if(bank_switch==bank0sink_bank1source)	//send final cluster
	{
		pdca_load_channel(USB_USART_TX_PDCA_CHANNEL,&bank1[0],samplebuffer_size);
		while((pdca_get_transfer_status(USB_USART_TX_PDCA_CHANNEL) & AVR32_PDCA_TRC_MASK) == 0x00000000);
	}
	else
	{
		pdca_load_channel(USB_USART_TX_PDCA_CHANNEL,&bank0[0],samplebuffer_size);
		while((pdca_get_transfer_status(USB_USART_TX_PDCA_CHANNEL) & AVR32_PDCA_TRC_MASK) == 0x00000000);
	}
	
	pdca_disable(USB_USART_TX_PDCA_CHANNEL);
}

void send_data_to_host_2(uint32_t SD_start_addr, uint8_t cluster_total)
{
	uint32_t SD_addr=SD_start_addr;
	my_pdca_init_channel(USB_USART_TX_PDCA_CHANNEL,0,0,USB_USART_TX_PDCA_PID,0,0,PDCA_TRANSFER_SIZE_BYTE);
	pdca_enable(USB_USART_TX_PDCA_CHANNEL);

	my_SD_read_multiblock_PDCA(SD_addr,(uint32_t)(&bank0[0]), blocks_per_cluster);	//must read a cluster from SD before UART can send it
	SD_addr+=blocks_per_cluster*bytes_per_block;
	bank_switch=bank1sink_bank0source;
	
	for(uint8_t clustercount=1;clustercount<cluster_total;clustercount++)
	{
		if(bank_switch==bank0sink_bank1source)
		{
			pdca_load_channel(USB_USART_TX_PDCA_CHANNEL,&bank1[0],samplebuffer_size);
			my_SD_read_multiblock_PDCA(SD_addr,(uint32_t)(&bank0[0]), blocks_per_cluster);
			SD_addr+=blocks_per_cluster*bytes_per_block;
			while((pdca_get_transfer_status(USB_USART_TX_PDCA_CHANNEL) & AVR32_PDCA_TRC_MASK) == 0x00000000);
			bank_switch=bank1sink_bank0source;
		}
		else
		{
			pdca_load_channel(USB_USART_TX_PDCA_CHANNEL,&bank0[0],samplebuffer_size);
			my_SD_read_multiblock_PDCA(SD_addr,(uint32_t)(&bank1[0]), blocks_per_cluster);
			SD_addr+=blocks_per_cluster*bytes_per_block;
			while((pdca_get_transfer_status(USB_USART_TX_PDCA_CHANNEL) & AVR32_PDCA_TRC_MASK) == 0x00000000);
			bank_switch=bank0sink_bank1source;
		}
	}
	
	if(bank_switch==bank0sink_bank1source)	//send final cluster
	{
		pdca_load_channel(USB_USART_TX_PDCA_CHANNEL,&bank1[0],samplebuffer_size);
		while((pdca_get_transfer_status(USB_USART_TX_PDCA_CHANNEL) & AVR32_PDCA_TRC_MASK) == 0x00000000);
	}
	else
	{
		pdca_load_channel(USB_USART_TX_PDCA_CHANNEL,&bank0[0],samplebuffer_size);
		while((pdca_get_transfer_status(USB_USART_TX_PDCA_CHANNEL) & AVR32_PDCA_TRC_MASK) == 0x00000000);
	}
	
	pdca_disable(USB_USART_TX_PDCA_CHANNEL);
}

void handle_SD_failure(void)
{	//first reset and reinitialize SD card
	new_ADC_data_ready=false;
	my_SPI_deselectchip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
	sd_mmc_spi_internal_init();
	//load shadowed SD_sink_ptr
	//and fill cluster at that location with 0xFF
	fill_cluster_with_0xFF(SD_sink_ptr_shadow, blocks_per_cluster);
}

void fill_cluster_with_0xFF(uint32_t addr, uint32_t Nblocks)
{
	my_SPI_selectchip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
	if(card_type == SD_CARD_2_SDHC) {
    r1 = my_SD_SPI_command(MMC_START_WRITE_BLOCK_MULTI , addr>>9);
  } else {
    r1 = my_SD_SPI_command(MMC_START_WRITE_BLOCK_MULTI , addr);
  }
  
  if(r1 != 0x00)
  {
    my_SPI_deselectchip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
    return;
  }
  my_SD_SPI_write(SD_MMC_SPI, 0xFF); 
  
  for(uint16_t i=0;i<Nblocks;i++)
  {
	  my_SD_SPI_write(SD_MMC_SPI,MMC_STARTBLOCK_WRITE_MULTI_TOKEN);
	 // write data
	 for(uint16_t j=0;j<MMC_SECTOR_SIZE;j++)
	{
		my_SD_SPI_write(SD_MMC_SPI,0xFF);
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
}

void SD_skip_clusters(uint16_t clusters)	//this is used after handling a data failure
{
	SD_sink_ptr=SD_sink_ptr_shadow+clusters*samplebuffer_size;	//increase SD_sink_ptr by some number of clusters
}

void shadow_SD_sink_ptr(void)
{
	SD_sink_ptr_shadow=SD_sink_ptr;
}	