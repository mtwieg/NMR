
#include <asf.h>
#include <Toplevel.h>
#include <user_board.h>
#include <my_misc_functions.h>
#include <NMRsequence.h> 
#include <string.h>
#include <sequenceparameters.h>
#include <SD_arbitration.h>
#include <my_SD_MMC.h>
#include <stdio.h>

extern uint8_t bank0[samplebuffer_size];
extern uint8_t bank1[samplebuffer_size];

extern char LCD_USART_buffer[64];
extern char host_USART_buffer[64];

extern experiment_t experiment;
extern t_experiment_t t_experiment;

char CMD[8];

extern char *cmd_ptr;

bool lcdinit=false;

#define select_standalone 1
#define select_checkSD 2
#define start_hostmode 3
#define get_data 4
#define program_experiment 5
#define SD_card_valid 0
#define SD_card_not_inserted 1
#define SD_card_init_failed 2
#define SD_card_invalid 3

void idle(void)
	{
	while(1)
	{
		uint8_t tempmode=0;
	
		while(tempmode==0)	//wait until a command has been received
		{
			if(lcdinit==false) //check for LCD
			{
				if(detect_lcd()==true)
				{
					usart_write_line(LCD_USART, "play modesel\r\n");
					lcdinit=true;
				}
			}
			tempmode=idle_getmode();
		}
		uint8_t tempsd=check_SD();
		
		switch(tempmode) {
			case start_hostmode:
				if(tempsd!=SD_card_valid)
				{
					usart_write_line(USB_USART, "bad SD card\n");
				}
				else
				{
					usart_write_line(USB_USART, "good SD card\n");
					hostmode_run();
					//go to host execution
				}	
				break;

			case select_standalone:
				if(tempsd==SD_card_valid)
				{	//valid SD card detected
					//retrieve experiment from SD card
					my_SD_read_experiment_PDCA(experiment_base_address,(uint32_t)(&experiment.MODE), (uint32_t)(&bank0[0]), sizeof(experiment), blocks_per_cluster);
					if(validate_sequences()==false)
					{	//sequence was not valid
						usart_write_line(LCD_USART, "play badexp\r\n");
						while(strcmp(get_LCD_cmd(),"BR2\r")!=0);
						usart_write_line(LCD_USART, "play modesel\r\n");
					}
					else
					{	//sequence was valid
						usart_write_line(LCD_USART, "play goodexp\r\n");
						while(strcmp(get_LCD_cmd(),"BR2\r")!=0);
						standalone_mode_run();
					
						//re-enable USB USART to receive commands from host
						pdca_load_channel(USB_USART_RX_PDCA_CHANNEL, (&host_USART_buffer),(uint32_t)(sizeof(host_USART_buffer)));
						pdca_enable(USB_USART_RX_PDCA_CHANNEL);
						USB_USART->cr|=AVR32_USART_CR_STTTO_MASK;	//set to not start counting again until after new character is received
						USB_USART->rtor=15000; //baud rate is 3mbaud, so 15000 gives 5ms timeout
					
						usart_write_line(LCD_USART, "play modesel\r\n");
						//will eventually have branch here to cancel, examine experiment, or proceed
					}
				}
				else if(tempsd==SD_card_not_inserted)
				{	//no SD card inserted
					usart_write_line(LCD_USART, "play noSD\r\n");
					while(strcmp(get_LCD_cmd(),"BR2\r")!=0);
					usart_write_line(LCD_USART, "play modesel\r\n");
				}
				else if(tempsd==SD_card_init_failed)
				{	//SD card failed to init
					usart_write_line(USB_USART, "play SDerr\r\n");
					while(strcmp(get_LCD_cmd(),"BR2\r")!=0);
					usart_write_line(LCD_USART, "play modesel\r\n");
				}
				else if(tempsd==SD_card_invalid)
				{	//SD card init but not valid
					usart_write_line(USB_USART, "play badSD\r\n");
					while(strcmp(get_LCD_cmd(),"BR2\r")!=0);
					usart_write_line(LCD_USART, "play modesel\r\n");
				}	
				break;
				
			case select_checkSD:
				switch (tempsd) {
					case  SD_card_valid:
						usart_write_line(LCD_USART, "play goodSD\r\n");
						while(strcmp(get_LCD_cmd(),"BR2\r")!=0);
						usart_write_line(LCD_USART, "play modesel\r\n");
						break;
					case SD_card_not_inserted:
						usart_write_line(LCD_USART, "play noSD\r\n");
						while(strcmp(get_LCD_cmd(),"BR2\r")!=0);
						usart_write_line(LCD_USART, "play modesel\r\n");
						break;
					case SD_card_init_failed:
						usart_write_line(USB_USART, "play SDerr\r\n");
						while(strcmp(get_LCD_cmd(),"BR2\r")!=0);
						usart_write_line(LCD_USART, "play modesel\r\n");
						break;
					case SD_card_invalid:
						usart_write_line(USB_USART, "play badSD\r\n");
						while(strcmp(get_LCD_cmd(),"BR2\r")!=0);
						usart_write_line(LCD_USART, "play modesel\r\n");
						break;
				}	
				break;

			case program_experiment:
				if(tempsd!=SD_card_valid)
				{
					usart_write_line(USB_USART, "bad SD card\n");
				}
				else
				{
					usart_write_line(USB_USART, "good SD card\n");
					get_experiment_from_host_to_SD();
				
					my_SD_read_experiment_PDCA(experiment_base_address,(uint32_t)(&experiment.MODE), (uint32_t)(&bank1[0]), sizeof(experiment), blocks_per_cluster);
				
					if(validate_sequences()==true)
					{
						usart_write_line(USB_USART, "goodexp\n");
					}
					else
					{
						usart_write_line(USB_USART, "badexp\n");
					}
				}
				break;

			case get_data:
				if(tempsd!=SD_card_valid)
				{
					usart_write_line(USB_USART, "bad SD card\n");	//if SD card is bad, say so and return
				}
				else
				{//if SD card is good, say so.  Then read the experiment and validate it
					usart_write_line(USB_USART, "good SD card\n");	
					//this will read in entire first cluster of SD
					my_SD_read_experiment_PDCA(experiment_base_address,(uint32_t)(&experiment.MODE), (uint32_t)(&bank1[0]), sizeof(experiment), blocks_per_cluster);
				
					if(validate_sequences()==true)
					{	//if experiment is valid, say so, then get true experiment
						usart_write_line(USB_USART, "goodexperiment\n");
						gettruesequence();
						//send entire cluster's worth of data to host.  this will start with experiment and also include other "stuff" in the first cluster
						pdca_disable(USB_USART_TX_PDCA_CHANNEL);//goin to set up PDCA to send all of experiment, then start reading out bank1 until an entire cluster has been written
						my_pdca_init_channel(USB_USART_TX_PDCA_CHANNEL, (uint32_t)(&experiment.MODE),(uint32_t)(sizeof(experiment)), USB_USART_TX_PDCA_PID,   (uint32_t)(&bank1[0]),  (blocks_per_cluster*512-(uint32_t)(sizeof(experiment))), PDCA_TRANSFER_SIZE_BYTE);
						pdca_enable(USB_USART_TX_PDCA_CHANNEL);
						while(!(pdca_get_transfer_status(USB_USART_TX_PDCA_CHANNEL) & AVR32_PDCA_TRC_MASK)); //wait until transfer is done
						pdca_disable(USB_USART_TX_PDCA_CHANNEL);
					
						for(uint8_t i=0;i<Nsequences_max;i++)
						{
							usart_putchar(USB_USART,(uint8_t)(t_experiment.t_sequence[i].clusters_per_sequence));
						}
					
						uint32_t SD_read_ptr=data_base_address;
					
						//now need to send actual data
						for(uint8_t i=0;i<t_experiment.N_experiments;i++)
						{
							for(uint8_t j=0;j<t_experiment.N_sequences;j++)
							{
								while(strcmp(get_HOST_cmd(),"SEND SEQUENCE\n")!=0); //wait for host to ask for each sequence
								send_data_to_host(SD_read_ptr,t_experiment.t_sequence[j].clusters_per_sequence);
								SD_read_ptr+=t_experiment.t_sequence[j].clusters_per_sequence*blocks_per_cluster*bytes_per_block;
							}
						}
					}
					else
					{
						usart_write_line(USB_USART, "badexperiment\n");
					}
				}
				break;		
		}
	}
}

uint8_t idle_getmode(void)
{
	my_delay(30000);
	
	if(lcdinit==true)
	{
		char *LCDCMD=get_LCD_cmd();
		if(strcmp(LCDCMD,"BR1\r")==0)
		{
			return select_standalone;
		}
		else if(strcmp(LCDCMD,"BR2\r")==0)
		{
			return select_checkSD;
		}
	}
	
	char *HOSTCMD=get_HOST_cmd();
	if(strcmp(HOSTCMD,"HOSTST\n")==0)
		{
			return start_hostmode;
		}
		else if(strcmp(HOSTCMD,"GET DATA\n")==0)
		{
			return get_data;
		}
		else if(strcmp(HOSTCMD,"PROGEXP\n")==0)
		{
			return program_experiment;
		}
		
	return 0;
	
};

bool detect_lcd(void)
{
	usart_write_line(LCD_USART, "PING\r");
	my_delay(30000);
	if(strcmp(get_LCD_cmd(),"0\r")==0) //in verbose off mode, pont will be 0\r
	{
		return true;
	}
	else
	{
		return false;
	}
}

char* get_LCD_cmd(void)
{
	if((LCD_USART->csr & AVR32_USART_CSR_TIMEOUT_MASK) != 0)
	{//timeout detected
		LCD_USART->cr|=AVR32_USART_CR_STTTO_MASK;	//set to not start counting again until after new character is received
		LCD_USART->rtor=230;
		
		cmd_ptr=(char *)(AVR32_PDCA.channel[LCD_USART_RX_PDCA_CHANNEL].mar-1);
		if((*(cmd_ptr)==0x0D) && ((cmd_ptr)!=(&LCD_USART_buffer[0])))
		{
			pdca_disable(LCD_USART_RX_PDCA_CHANNEL);
			pdca_load_channel(LCD_USART_RX_PDCA_CHANNEL, (&LCD_USART_buffer),(uint32_t)(sizeof(LCD_USART_buffer)));
			
			*(cmd_ptr+1)=0x00;
			cmd_ptr--;
			while((*(cmd_ptr-1)!=0x0D) && ((cmd_ptr)!=(&LCD_USART_buffer[0])))
			{
				cmd_ptr--;
			}
			
			pdca_enable(LCD_USART_RX_PDCA_CHANNEL);
			return cmd_ptr;
		}
		
		pdca_disable(LCD_USART_RX_PDCA_CHANNEL);
		pdca_load_channel(LCD_USART_RX_PDCA_CHANNEL, (&LCD_USART_buffer),(uint32_t)(sizeof(LCD_USART_buffer)));
		pdca_enable(LCD_USART_RX_PDCA_CHANNEL);
		return "false";
	}
	return "false";
}

char* get_HOST_cmd(void)
{
	if((USB_USART->csr & AVR32_USART_CSR_TIMEOUT_MASK) != 0)
	{//timeout detected
		USB_USART->cr|=AVR32_USART_CR_STTTO_MASK;	//set to not start counting again until after new character is received
		USB_USART->rtor=15000; //baud rate is 3mbaud, so 15000 gives 5ms timeout
		
		pdca_disable(USB_USART_RX_PDCA_CHANNEL);
		cmd_ptr=(char *)(AVR32_PDCA.channel[USB_USART_RX_PDCA_CHANNEL].mar);
		*(cmd_ptr)=0x00;  //make sure command string is null terminated
		cmd_ptr=&host_USART_buffer[0];
		
		pdca_load_channel(USB_USART_RX_PDCA_CHANNEL, (&host_USART_buffer),(uint32_t)(sizeof(host_USART_buffer)));
		pdca_enable(USB_USART_RX_PDCA_CHANNEL);
		return cmd_ptr;
	}
	return "false";
}

uint8_t check_SD(void)
{
	if(SD_card_inserted()==false)
	{
		return SD_card_not_inserted;	
	}
	spi_options_t SD_spiOptions =
	  {
		.reg          = SD_MMC_SPI_NPCS,
		.baudrate     = SD_SPI_SPEED,  // Defined in conf_sd_mmc_spi.h.
		.bits         = 8,          // Defined in conf_sd_mmc_spi.h.
		.spck_delay   = 0,
		.trans_delay  = 0,
		.stay_act     = 1,
		.spi_mode     = 0,
		.modfdis      = 1
	  };
  
	if(sd_mmc_spi_init(SD_spiOptions, PBA_SPEED)==false)
	{
		return SD_card_init_failed;
	}
  
	if(card_type!=SD_CARD_2_SDHC)
	{
		return SD_card_invalid;
	}
	return SD_card_valid;
}

bool SD_card_inserted(void)
{
	if(gpio_get_pin_value(SD_detect_pin)==0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

uint8_t hostmode_run(void)
{
	//first receive and verify full experiment
	//set USB PDCA to store experiment
	USB_USART->rtor=0;	//disable timeout
	pdca_disable(USB_USART_RX_PDCA_CHANNEL);
	my_pdca_init_channel(USB_USART_RX_PDCA_CHANNEL, (uint32_t)(&experiment.MODE),(uint32_t)(sizeof(experiment)), USB_USART_RX_PDCA_PID,   0,  0, PDCA_TRANSFER_SIZE_BYTE);
	pdca_enable(USB_USART_RX_PDCA_CHANNEL);
	
	while(!(pdca_get_transfer_status(USB_USART_RX_PDCA_CHANNEL) & AVR32_PDCA_TRC_MASK)); //wait until transfer is done
	
	if(validate_sequences()==false)
	{
		usart_write_line(USB_USART, "badexp\n");
		
		usart_putchar(USB_USART,(uint8_t)(get_experiment_problem()));
		
		my_pdca_init_channel(USB_USART_RX_PDCA_CHANNEL, (uint32_t)(&host_USART_buffer),(uint32_t)(sizeof(host_USART_buffer)),USB_USART_RX_PDCA_PID,0,0, PDCA_TRANSFER_SIZE_BYTE);
		pdca_enable(USB_USART_RX_PDCA_CHANNEL);
		USB_USART->cr|=AVR32_USART_CR_STTTO_MASK; //set timeout to stop until new character is received
		USB_USART->rtor=15000;	//set to timeout in 1ms
		
		return 0;
	}
	
	usart_write_line(USB_USART, "goodexp\n");
	pdca_disable(USB_USART_TX_PDCA_CHANNEL);
	gettruesequence();
	my_pdca_init_channel(USB_USART_TX_PDCA_CHANNEL, (uint32_t)(&experiment.MODE),(uint32_t)(sizeof(experiment)), USB_USART_TX_PDCA_PID,   0,  0, PDCA_TRANSFER_SIZE_BYTE);
	pdca_enable(USB_USART_TX_PDCA_CHANNEL);
	while(!(pdca_get_transfer_status(USB_USART_TX_PDCA_CHANNEL) & AVR32_PDCA_TRC_MASK)); //wait until transfer is done
	pdca_disable(USB_USART_TX_PDCA_CHANNEL);
	
	//change back USB USART to accept command tokens
	my_pdca_init_channel(USB_USART_RX_PDCA_CHANNEL, (uint32_t)(&host_USART_buffer),(uint32_t)(sizeof(host_USART_buffer)),USB_USART_RX_PDCA_PID,0,0, PDCA_TRANSFER_SIZE_BYTE);
	pdca_enable(USB_USART_RX_PDCA_CHANNEL);
	
	for(uint8_t i=0;i<Nsequences_max;i++)
	{
		usart_putchar(USB_USART,(uint8_t)(t_experiment.t_sequence[i].clusters_per_sequence));
	}
	
	USB_USART->cr|=AVR32_USART_CR_STTTO_MASK; //set timeout to stop until new character is received
	USB_USART->rtor=15000;	//set to timeout in 1ms
	
	reset_SD_sink_ptr();
	uint32_t SD_read_ptr=data_base_address;
	
	DAC1->dr0=t_experiment.t_Vgain; //set gain of RF amp
	
	while(1)	//once in host mode, this will run until a restart command is received
	{
		char *HOSTCMD="false";
		while(strcmp(HOSTCMD,"false")==0)
		{
			HOSTCMD=get_HOST_cmd();
			if(strcmp((HOSTCMD+1),"sequencetoken")==0)
			{
				uint8_t sequenceindex=*HOSTCMD;
				t_currentsequence=t_experiment.t_sequence[sequenceindex];
				
				executesequence_SDstorage_multiprep_combined();
				
				if(did_data_fail())
				{
					usart_write_line(USB_USART, "fail\n");
					usart_putchar(USB_USART,(uint8_t)(get_failure_cause()));
					usart_putchar(USB_USART,(uint8_t)(get_saved_r1()));
				}					
				else
				{
					usart_write_line(USB_USART, "good\n");
					send_data_to_host(SD_read_ptr,t_currentsequence.clusters_per_sequence);
				}	
				
				
				SD_read_ptr+=t_currentsequence.clusters_per_sequence*blocks_per_cluster*bytes_per_block;
				//set back to command mode
				pdca_disable(USB_USART_RX_PDCA_CHANNEL);
				my_pdca_init_channel(USB_USART_RX_PDCA_CHANNEL, (uint32_t)(&host_USART_buffer),(uint32_t)(sizeof(host_USART_buffer)),USB_USART_RX_PDCA_PID,0,0, PDCA_TRANSFER_SIZE_BYTE);
				pdca_enable(USB_USART_RX_PDCA_CHANNEL);
				USB_USART->cr|=AVR32_USART_CR_STTTO_MASK; //set timeout to stop until new character is received
				USB_USART->rtor=15000;	//set to timeout in 1ms
			}
			else if(strcmp((HOSTCMD),"RESTART\n")==0)
			{	//want to handle start of new host mode experiment
				return 1;
			}
		}
			
	}
	return 1;	
}

bool standalone_mode_run(void)
{
	char tempstring1[24];
	while(detect_lcd()==false);
	usart_write_line(LCD_USART, "play stndprog\r\n");
	while(detect_lcd()==false);
	
	usart_write_line(LCD_USART, "DMETER_VALUE 1 1\r\n");
	while(detect_lcd()==false);
	
	sprintf(tempstring1,"DMETER_VALUE 2 %u\r\n",t_experiment.N_experiments);
	usart_write_line(LCD_USART, tempstring1);
	while(detect_lcd()==false);
	
	usart_write_line(LCD_USART, "DMETER_VALUE 3 1\r\n");
	while(detect_lcd()==false);
	
	sprintf(tempstring1,"DMETER_VALUE 4 %u\r\n",t_experiment.N_sequences);
	usart_write_line(LCD_USART, tempstring1);
	while(detect_lcd()==false);
	
	reset_SD_sink_ptr(); //start saving data at data_base_addr
	
	DAC1->dr0=t_experiment.t_Vgain;	//set gain of RF amp
	
	tc_write_rc(SLOW_TC,SLOW_TC_slow_CHANNEL,t_experiment.t_sequence[0].t_TR);
	tc_start(SLOW_TC, SLOW_TC_slow_CHANNEL);	//start slow timer
	tc_start(SLOW_TC, SLOW_TC_fast_CHANNEL);	//start slow timer
	pdca_disable(USB_USART_RX_PDCA_CHANNEL);	//Not going to need USB USART for a while
	pdca_disable(USB_USART_TX_PDCA_CHANNEL);
	
	bool progscreen=true;	//state variable for user interface
	
	for(uint16_t experimentcount=0;experimentcount<t_experiment.N_experiments;experimentcount++)
	{
		for(uint8_t sequencecount=0;sequencecount<t_experiment.N_sequences;sequencecount++)
		{
			t_currentsequence=t_experiment.t_sequence[sequencecount]; //get sequence
			while((tc_read_sr(SLOW_TC, SLOW_TC_slow_CHANNEL) & AVR32_TC_SR1_CPCS_MASK)==0);
			
			tc_write_rc(SLOW_TC,SLOW_TC_slow_CHANNEL,t_experiment.t_sequence[sequencecount].t_TR);
			
			
				char *LCDCMD=get_LCD_cmd();
				if((progscreen==true) && (strcmp(LCDCMD,"BR2\r")==0))	//BR2 is first cancel button
				{
					progscreen=false;
					usart_write_line(LCD_USART, "play confcanc\r\n");
					while(detect_lcd()==false);
				}
				else if((progscreen==false) && (strcmp(LCDCMD,"BR3\r")==0))	//BR3 is confirm cancel button
				{
					usart_write_line(LCD_USART, "play modesel\r\n"); //maybe modsel isn't necessary here, but will be done after return?
					return false;
				}
				else if((progscreen==false) && (strcmp(LCDCMD,"BR1\r")==0))	//BR1 if resume button
				{
					progscreen=true;
					usart_write_line(LCD_USART, "play stndprog\r\n");
					
					while(detect_lcd()==false);
	
					sprintf(tempstring1,"DMETER_VALUE 1 %u\r",(experimentcount+1));
					usart_write_line(LCD_USART, tempstring1);
					
					while(detect_lcd()==false);
	
					sprintf(tempstring1,"DMETER_VALUE 2 %u\r",t_experiment.N_experiments);
					usart_write_line(LCD_USART, tempstring1);
					
					while(detect_lcd()==false);
					
					sprintf(tempstring1,"DMETER_VALUE 3 %u\r",(sequencecount+1));
					usart_write_line(LCD_USART, tempstring1);
					
					while(detect_lcd()==false);
	
					sprintf(tempstring1,"DMETER_VALUE 4 %u\r",t_experiment.N_sequences);
					usart_write_line(LCD_USART, tempstring1);
					
					while(detect_lcd()==false);
					
				}
				else if(progscreen==true)
				{
					sprintf(tempstring1,"DMETER_VALUE 1 %u\r",(experimentcount+1));
					gpio_set_gpio_pin(GEN1_pin);
					usart_write_line(LCD_USART, tempstring1);
					while(wait_for_CR()==false);
					gpio_clr_gpio_pin(GEN1_pin);
					
					
					sprintf(tempstring1,"DMETER_VALUE 3 %u\r",(sequencecount+1));
					gpio_set_gpio_pin(GEN1_pin);
					usart_write_line(LCD_USART, tempstring1);
					while(wait_for_CR()==false);
					gpio_clr_gpio_pin(GEN1_pin);
					
				}
				executesequence_SDstorage_multiprep_combined();
		}
		
	}
	
	return true;
}

//this gets an experiment, plus additional data, from the host
//A full cluster will be transferred
void get_experiment_from_host_to_SD(void)
{
	USB_USART->rtor=0; //going to use PDCA for receiving data, so disable receive timeout
	pdca_disable(USB_USART_RX_PDCA_CHANNEL);
	my_pdca_init_channel(USB_USART_RX_PDCA_CHANNEL, (uint32_t)(&bank0[0]),samplebuffer_size, USB_USART_RX_PDCA_PID,   0,  0, PDCA_TRANSFER_SIZE_BYTE);
	pdca_enable(USB_USART_RX_PDCA_CHANNEL);
	
	while(!(pdca_get_transfer_status(USB_USART_RX_PDCA_CHANNEL) & AVR32_PDCA_TRC_MASK)); //wait until transfer is done
	
	pdca_disable(USB_USART_RX_PDCA_CHANNEL);	//reset USART RX channel to receive host commands
	
	USB_USART->cr|=AVR32_USART_CR_STTTO_MASK;	//changing back to receiving command mode
	USB_USART->rtor=15000; //so reenable timeout
	
	pdca_load_channel(USB_USART_RX_PDCA_CHANNEL, (&host_USART_buffer),(uint32_t)(sizeof(host_USART_buffer)));
	pdca_enable(USB_USART_RX_PDCA_CHANNEL);
	
	spi_selectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
	while(check_busy_fast()!=0xFF);
	spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
	
	my_SD_SPI_block_write_multi(bank0,experiment_base_address,blocks_per_cluster);	//write data to SD
	
	spi_selectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
	while(check_busy_fast()!=0xFF);
	spi_unselectChip(SD_MMC_SPI, SD_MMC_SPI_NPCS);
}

bool wait_for_CR(void)
{
	if((LCD_USART->csr & AVR32_USART_CSR_TIMEOUT_MASK) != 0)
	{//timeout detected
		LCD_USART->cr|=AVR32_USART_CR_STTTO_MASK;	//set to not start counting again until after new character is received
		LCD_USART->rtor=230;
		
		pdca_disable(LCD_USART_RX_PDCA_CHANNEL);
		pdca_load_channel(LCD_USART_RX_PDCA_CHANNEL, (&LCD_USART_buffer),(uint32_t)(sizeof(LCD_USART_buffer)));
		pdca_enable(LCD_USART_RX_PDCA_CHANNEL);
		return true;
	}
	return false;
}