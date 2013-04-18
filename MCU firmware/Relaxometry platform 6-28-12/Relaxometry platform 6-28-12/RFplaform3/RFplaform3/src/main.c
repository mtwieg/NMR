/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * AVR Software Framework (ASF).
 */
#include <asf.h>
#include <my_misc_functions.h>
#include <my_SD_MMC.h>
#include <AD9958.h>
#include <sequenceparameters.h>
#include <math.h>
#include <NMRsequence.h> 
#include <user_board.h>
#include <SD_arbitration.h>
#include <Toplevel.h>

#include <string.h>

//string LCD_string[8];

char LCD_USART_buffer[64];
char host_USART_buffer[64];
//char LCD_CMD[8];

char *cmd_ptr;
//#define DDS_Fs 120000000.0

DDS_options_t DDS_options;
DDS_channel_options_t DDS_options_CH0;
DDS_channel_options_t DDS_options_CH1;

uint8_t dummyblock[512];

uint8_t bank0[samplebuffer_size];
//uint8_t dummyreadbuffer[samplebuffer_size];
uint8_t bank1[samplebuffer_size];

//#define TC_div 8

uint32_t temp;

__attribute__((__interrupt__)) void tc_irq(void)
{
	uint32_t temp=AVR32_TC0.channel[0].sr;
}

int main (void) {
		//initialize DDS configuration
		DDS_options.pcs=DDS_PCS;
		DDS_options.SERmode=SER_MODE_3WIRE;
		DDS_options.LSBfirst=MSB_FIRST;
		DDS_options.VCOgain=DDS_VCO_gain;
		DDS_options.PLLdiv=DDS_PLL_div;		//use 6 for 20MHz clock
		DDS_options.CHRGPMP=CHRGPMP_75uA;
		DDS_options.PPC=0b101;
		DDS_options.MODLEVEL=MODLEVEL_4;

		DDS_options_CH0.pcs=DDS_PCS;
		DDS_options_CH0.channels=CH_EN_0;
		DDS_options_CH0.AFPmod=Amod;
		DDS_options_CH0.DACfsc=DAC_FSC_FULL;
		DDS_options_CH0.coreenable=DCPDN_ON;
		DDS_options_CH0.DACenable=DACPDN_ON;
		DDS_options_CH0.matchedpipes=MATCHED_PIPES_DIS;
		DDS_options_CH0.sinorcos=COS_OUT;
		DDS_options_CH0.ampmulenable=AMP_MUL_EN;
		DDS_options_CH0.RURDenable=AMP_MUL_MANUAL;
		DDS_options_CH0.ASF=512;
		DDS_options_CH0.FTW=357913941;
		DDS_options_CH0.POW=0;

		DDS_options_CH1.pcs=DDS_PCS;
		DDS_options_CH1.channels=CH_EN_1;
		DDS_options_CH1.AFPmod=Amod;
		DDS_options_CH1.DACfsc=DAC_FSC_FULL;
		DDS_options_CH1.coreenable=DCPDN_ON;
		DDS_options_CH1.DACenable=DACPDN_ON;
		DDS_options_CH1.matchedpipes=MATCHED_PIPES_DIS;
		DDS_options_CH1.sinorcos=COS_OUT;
		DDS_options_CH1.ampmulenable=AMP_MUL_EN;
		DDS_options_CH1.RURDenable=AMP_MUL_MANUAL;
		DDS_options_CH1.ASF=512;
		DDS_options_CH1.FTW=357913941;
		DDS_options_CH1.POW=4096;
	
	board_init();	//initialize MCU peripherals
	
	fnEnterAVRInterrupt((int)(AVR32_TC0_IRQ0 / AVR32_INTC_MAX_NUM_IRQS_PER_GRP), INT_LEVEL_0,(void (*)(void))tc_irq);	//create ISR for timer
	
	//initialize DMA channels
	pdca_disable(SPI1_RX_PDCA_CHANNEL);
	my_pdca_init_channel(SPI1_RX_PDCA_CHANNEL,(uint32_t)(&bank0),(samplebuffer_size/2), SPI1_RX_PDCA_PID,   (uint32_t)(&bank1),  (samplebuffer_size/2), PDCA_TRANSFER_SIZE_HALF_WORD);
	
	pdca_disable(SPI0_TX_PDCA_CHANNEL);
	my_pdca_init_channel(SPI0_TX_PDCA_CHANNEL,(uint32_t)(&bank1),0, SPI0_TX_PDCA_PID, 0, 0, PDCA_TRANSFER_SIZE_BYTE);	//initialize with 0 in TCR and TCRR so that nothing happens until TCR is written
	pdca_enable(SPI0_TX_PDCA_CHANNEL); //needs to be enabled at start of a sequence!
	
	pdca_disable(SPI0_RX_PDCA_CHANNEL);
	my_pdca_init_channel(SPI0_RX_PDCA_CHANNEL,(uint32_t)(&bank0),0, SPI0_RX_PDCA_PID, 0, 0, PDCA_TRANSFER_SIZE_BYTE);	//initialize with 0 in TCR and TCRR so that nothing happens until TCR is written
	
	//create a dummy data block for later use
	uint8_t temp=0;	
	for(int i=0;i<512;i++)
		{
			dummyblock[i]=0xFF;
		}
	
	cpu_irq_disable();

	pdca_enable(LCD_USART_RX_PDCA_CHANNEL);
	
	pdca_enable(USB_USART_RX_PDCA_CHANNEL);

	idle();	//start main routine
	
}
