/**
 * \file
 *
 * \brief User board initialization template
 *
 */

#include <board.h>
#include <conf_board.h>
#include <asf.h>
#include <user_board.h>
#include <my_misc_functions.h>

extern char LCD_USART_buffer[64];
extern char host_USART_buffer[64];

static void tc_init_fast(volatile avr32_tc_t *tc)
{
	// Options for waveform generation.
	static const tc_waveform_opt_t waveform_opt_1 = {

		.channel  = FAST_TC_CHANNEL,  // Channel selection.
		.bswtrg   = TC_EVT_EFFECT_NOOP, // Software trigger effect on TIOB.
		.beevt    = TC_EVT_EFFECT_NOOP,	// External event effect on TIOB.
		.bcpc     = TC_EVT_EFFECT_NOOP,	// RC compare effect on TIOB.
		.bcpb     = TC_EVT_EFFECT_NOOP,	// RB compare effect on TIOB.
		.aswtrg   = TC_EVT_EFFECT_NOOP,	// Software trigger effect on TIOA.
		.aeevt    = TC_EVT_EFFECT_NOOP,	// External event effect on TIOA.
		.acpc     = TC_EVT_EFFECT_NOOP,	// RC compare effect on TIOA.
		.acpa     = TC_EVT_EFFECT_NOOP,	//RA compare effect on TIOA. 
		.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,	//Waveform selection: Up mode with automatic trigger(reset)
		.enetrg   = false,	//// External event trigger enable.
		.eevt     = 0,	//// External event selection.
		.eevtedg  = TC_SEL_NO_EDGE,	//// External event edge selection.
		.cpcdis   = false,	// Counter disable when RC compare.
		.cpcstop  = false,	// Counter clock stopped with RC compare.
		.burst    = false,	// Burst signal selection
		.clki     = false,	// Clock inversion.
		.tcclks   = TC_CLOCK_SOURCE_TC3	// Internal source clock 3, connected to fPBA / 8.
	};

	// Options for enabling TC interrupts
	static const tc_interrupt_t tc_interrupt = {
		.etrgs = 0,
		.ldrbs = 0,
		.ldras = 0,
		.cpcs  = 1, // Enable interrupt on RC compare alone
		.cpbs  = 0,
		.cpas  = 0,
		.lovrs = 0,
		.covfs = 0
	};
	// Initialize the timer/counter.
	tc_init_waveform(tc, &waveform_opt_1);

	tc_write_rc(tc, FAST_TC_CHANNEL, 10);
	// configure the timer interrupt
	tc_configure_interrupts(tc, FAST_TC_CHANNEL, &tc_interrupt);
}

static void tc_init_slow(volatile avr32_tc_t *tc)
{
	// Options for waveform generation.
	static const tc_waveform_opt_t waveform_opt_2 = {
		.channel  = SLOW_TC_fast_CHANNEL,	// Channel selection.
		.bswtrg   = TC_EVT_EFFECT_NOOP,	// Software trigger effect on TIOB.
		.beevt    = TC_EVT_EFFECT_NOOP,	// External event effect on TIOB.
		.bcpc     = TC_EVT_EFFECT_NOOP,	// RC compare effect on TIOB.
		.bcpb     = TC_EVT_EFFECT_NOOP,	// RB compare effect on TIOB.
		.aswtrg   = TC_EVT_EFFECT_NOOP,	// Software trigger effect on TIOA.
		.aeevt    = TC_EVT_EFFECT_NOOP,	// External event effect on TIOA.
		.acpc     = TC_EVT_EFFECT_CLEAR,	// RC compare effect on TIOA.
		.acpa     = TC_EVT_EFFECT_SET,	// RA compare effect on TIOA. 
		.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,	//Waveform selection: Up mode with automatic trigger(reset)
		.enetrg   = false,	// External event trigger enable.
		.eevt     = 0,	// External event selection.
		.eevtedg  = TC_SEL_NO_EDGE,	// External event edge selection.
		.cpcdis   = false,	// Counter disable when RC compare.
		.cpcstop  = false,	// Counter clock stopped with RC compare.
		.burst    = false,	// Burst signal selection.
		.clki     = false,	// Clock inversion.
		.tcclks   = TC_CLOCK_SOURCE_TC3	// Internal source clock 3, connected to fPBA / 8.
	};

	// Initialize the timer/counter.
	tc_init_waveform(tc, &waveform_opt_2);
	tc_write_rc(tc, SLOW_TC_fast_CHANNEL, 7500); //counter will count milliseconds
	tc_write_ra(tc, SLOW_TC_fast_CHANNEL, 3500); //configure ra so that TIOA0 is toggled
	
	static const tc_waveform_opt_t waveform_opt_3 = {
		.channel  = SLOW_TC_slow_CHANNEL,	// Channel selection.	
		.bswtrg   = TC_EVT_EFFECT_NOOP,	// Software trigger effect on TIOB.
		.beevt    = TC_EVT_EFFECT_NOOP,	// External event effect on TIOB.
		.bcpc     = TC_EVT_EFFECT_NOOP,	// RC compare effect on TIOB.
		.bcpb     = TC_EVT_EFFECT_NOOP,	// RB compare effect on TIOB.
		.aswtrg   = TC_EVT_EFFECT_NOOP,	// Software trigger effect on TIOA.
		.aeevt    = TC_EVT_EFFECT_NOOP,	// External event effect on TIOA.
		.acpc     = TC_EVT_EFFECT_NOOP,	// RC compare effect on TIOA.
		.acpa     = TC_EVT_EFFECT_NOOP,	//RA compare effect on TIOA. 
		.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,	//Waveform selection: Up mode with automatic trigger(reset)
		.enetrg   = false,	//// External event trigger enable.
		.eevt     = 0,	//// External event selection.
		.eevtedg  = TC_SEL_NO_EDGE,	//// External event edge selection.
		.cpcdis   = false,	// Counter disable when RC compare.
		.cpcstop  = false,	// Counter clock stopped with RC compare.
		.burst    = false,	// Burst signal selection.
		.clki     = false,	// Clock inversion.
		.tcclks   = TC_CLOCK_SOURCE_XC0	// Use XC1 as clock source.  Must configure TIOA0 to be XC1
	};
	
	tc_init_waveform(tc, &waveform_opt_3);
	tc_write_rc(tc, SLOW_TC_slow_CHANNEL, 100); //
	tc_select_external_clock(tc,SLOW_TC_slow_CHANNEL,AVR32_TC_BMR_TC0XC0S_TIOA1); //use TIOA1 as XC0
}


	static void configure_hmatrix(uint32_t mode)
{
	// Configure all Slave in Last Default Master
#if (defined AVR32_HMATRIX) 
	for(uint32_t i = 0; i < AVR32_HMATRIX_SLAVE_NUM; i++) {
		AVR32_HMATRIX.SCFG[i].defmstr_type = mode;
	}
#endif
#if (defined AVR32_HMATRIXB)
	for(uint32_t i = 0;i < AVR32_HMATRIXB_SLAVE_NUM; i++) {
		AVR32_HMATRIXB.SCFG[i].defmstr_type = mode;
	}
#endif 
}

void board_init(void)
{
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */
	gpio_local_init();
	
	static pcl_freq_param_t pcl_freq_param =
{
	.cpu_f        = CPU_SPEED,
	.pba_f        = PBA_SPEED,
	.osc0_f       = FOSC0,
	.osc0_startup = OSC0_STARTUP
};

	if (pcl_configure_clocks(&pcl_freq_param) != PASS)
	while (true);
	
	configure_hmatrix(AVR32_HMATRIXB_DEFMSTR_TYPE_NO_DEFAULT);
	
	AVR32_LowLevelInit();
	
	//configure all GPIO
	gpio_local_enable_pin_output_driver(ADC_CONV_pin);
	gpio_local_clr_gpio_pin(ADC_CONV_pin);
	gpio_local_enable_pin_output_driver(DDS_IOUD_pin);
	gpio_local_clr_gpio_pin(DDS_IOUD_pin);
	gpio_local_enable_pin_output_driver(DDS_RESET_pin);
	gpio_local_clr_gpio_pin(DDS_RESET_pin);
	gpio_local_enable_pin_output_driver(DDS_PDN_pin);
	gpio_local_set_gpio_pin(DDS_PDN_pin);
	
	gpio_local_enable_pin_output_driver(DDS_P0_pin);
	gpio_local_clr_gpio_pin(DDS_P0_pin);
	gpio_local_enable_pin_output_driver(DDS_P1_pin);
	gpio_local_clr_gpio_pin(DDS_P1_pin);
	gpio_local_enable_pin_output_driver(DDS_P2_pin);
	gpio_local_clr_gpio_pin(DDS_P2_pin);
	gpio_local_enable_pin_output_driver(DDS_P3_pin);
	gpio_local_clr_gpio_pin(DDS_P3_pin);
	
	gpio_local_enable_pin_output_driver(RXSW_pin);
	gpio_local_clr_gpio_pin(RXSW_pin);
	gpio_local_enable_pin_output_driver(TXSW_pin);
	gpio_local_clr_gpio_pin(TXSW_pin);
	gpio_local_enable_pin_output_driver(TPAbias_pin);
	gpio_local_clr_gpio_pin(TPAbias_pin);
	gpio_local_enable_pin_output_driver(GEN1_pin);
	gpio_local_clr_gpio_pin(GEN1_pin);
	gpio_local_enable_pin_output_driver(GEN2_pin);
	gpio_local_clr_gpio_pin(GEN2_pin);
	
	gpio_local_enable_pin_output_driver(PWM0_pin);
	gpio_local_clr_gpio_pin(PWM0_pin);
	
	gpio_local_disable_pin_output_driver(SD_detect_pin);
	
	//configure all peripheral IO
	
	static const gpio_map_t GCLK_GPIO_MAP =
	{
		{AVR32_SCIF_GCLK_0_1_PIN, AVR32_SCIF_GCLK_0_1_FUNCTION}
	};
	
	gpio_enable_module(GCLK_GPIO_MAP,
			sizeof(GCLK_GPIO_MAP) / sizeof(GCLK_GPIO_MAP[0]));
	
	genclk_enable_config(9, GENCLK_SRC_CLK_CPU, 2);
	
	static const gpio_map_t SPI_GPIO_MAP =
	{
		{SPI1_SCK_PIN, SPI1_SCK_FUNCTION},
		{SPI1_MOSI_PIN, SPI1_MOSI_FUNCTION},
		{SPI1_MISO_PIN, SPI1_MISO_FUNCTION},
		{SPI1_NPCS2_PIN, SPI1_NPCS2_FUNCTION}
	};
	
	gpio_enable_module(SPI_GPIO_MAP,
			sizeof(SPI_GPIO_MAP) / sizeof(SPI_GPIO_MAP[0]));


spi_options_t SPI1_OPTIONS_0 = 
{
  .reg			= 0,		//! The SPI channel to set up.
  .baudrate		= 30000000,	//! Preferred baudrate for the SPI.
  .bits			=16,	//! Number of bits in each character (8 to 16).
  .spck_delay	=0,	//! Delay before first clock pulse after selecting slave (in PBA clock periods). 
  .trans_delay	=0,	//! Delay between each transfer/character (in PBA clock periods).
  .stay_act		=0,	 //! Sets this chip to stay active after last transfer to it.
  .spi_mode		=1,	//! Which SPI mode to use when transmitting.
  .modfdis		=1	 //! Disables the mode fault detection.
};

spi_options_t SPI1_OPTIONS_3 = 
{
  .reg			= 3,		//! The SPI channel to set up.
  .baudrate		= 30000000,	//! Preferred baudrate for the SPI.
  .bits			=8,	//! Number of bits in each character (8 to 16).
  .spck_delay	=0,	//! Delay before first clock pulse after selecting slave (in PBA clock periods).
  .trans_delay	=0,	//! Delay between each transfer/character (in PBA clock periods).
  .stay_act		=1,	//! Sets this chip to stay active after last transfer to it.
  .spi_mode		=0,	//! Which SPI mode to use when transmitting.
  .modfdis		=1	//! Disables the mode fault detection.
};

	spi_initMaster(SPI1, &SPI1_OPTIONS_0); 
	spi_selectionMode(SPI1,1,0,0);
	spi_enable(SPI1);
	spi_setupChipReg(SPI1,&SPI1_OPTIONS_0,PBA_SPEED);
	spi_setupChipReg(SPI1,&SPI1_OPTIONS_3,PBA_SPEED);
	spi_selectChip(SPI1, 3);
	
	static const gpio_map_t USB_USART_GPIO_MAP =
	{
		{USB_USART_RX_PIN, USB_USART_RX_FUNCTION},
		{USB_USART_TX_PIN, USB_USART_TX_FUNCTION},
		{USB_USART_RTS_PIN, USB_USART_RTS_FUNCTION},
		{USB_USART_CTS_PIN, USB_USART_CTS_FUNCTION}
	};
	
	gpio_enable_module(USB_USART_GPIO_MAP,
			sizeof(USB_USART_GPIO_MAP) / sizeof(USB_USART_GPIO_MAP[0]));
			
	static const usart_options_t USB_USART_OPTIONS =
	{
		.baudrate     = 3000000,
		.charlength   = 8,
		.paritytype   = USART_NO_PARITY,
		.stopbits     = USART_1_STOPBIT,
		.channelmode  = USART_NORMAL_CHMODE
	};
			
	usart_init_hw_handshaking(USB_USART, &USB_USART_OPTIONS, PBA_SPEED);
	
	static const gpio_map_t LCD_USART_GPIO_MAP =
	{
		{LCD_USART_RX_PIN, LCD_USART_RX_FUNCTION},
		{LCD_USART_TX_PIN, LCD_USART_TX_FUNCTION}
	};
	
	gpio_enable_module(LCD_USART_GPIO_MAP,
			sizeof(LCD_USART_GPIO_MAP) / sizeof(LCD_USART_GPIO_MAP[0]));
			
	static const usart_options_t LCD_USART_OPTIONS =
	{
		.baudrate     = 115200,
		.charlength   = 8,
		.paritytype   = USART_NO_PARITY,
		.stopbits     = USART_1_STOPBIT,
		.channelmode  = USART_NORMAL_CHMODE
	};
			
	usart_init_rs232(LCD_USART, &LCD_USART_OPTIONS, PBA_SPEED);
	LCD_USART->cr|=AVR32_USART_CR_STTTO_MASK; //set timeout to stop until new character is received
	LCD_USART->rtor=230;	//set to timeout in 2ms
	
	
	my_pdca_init_channel(LCD_USART_RX_PDCA_CHANNEL, (uint32_t)(&LCD_USART_buffer),(uint32_t)(sizeof(LCD_USART_buffer)),LCD_USART_RX_PDCA_PID,0,0, PDCA_TRANSFER_SIZE_BYTE);
	pdca_disable(LCD_USART_RX_PDCA_CHANNEL);
	
	my_pdca_init_channel(USB_USART_RX_PDCA_CHANNEL, (uint32_t)(&host_USART_buffer),(uint32_t)(sizeof(host_USART_buffer)),USB_USART_RX_PDCA_PID,0,0, PDCA_TRANSFER_SIZE_BYTE);
	pdca_disable(USB_USART_RX_PDCA_CHANNEL);
	
	USB_USART->cr|=AVR32_USART_CR_STTTO_MASK; //set timeout to stop until new character is received
	USB_USART->rtor=15000;	//set to timeout in 1ms
	
	// GPIO pins used for SD/MMC interface
  static const gpio_map_t SD_MMC_SPI_GPIO_MAP =
  {
    {SPI0_SCK_PIN,  SPI0_SCK_FUNCTION },  // SPI Clock.
    {SPI0_MISO_PIN, SPI0_MISO_FUNCTION},  // MISO.
    {SPI0_MOSI_PIN, SPI0_MOSI_FUNCTION},  // MOSI.
    {SPI0_NPCS0_PIN, SPI0_NPCS0_FUNCTION}   // Chip Select NPCS.
  };

   //SPI options.
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

  // Assign I/Os to SPI.
  gpio_enable_module(SD_MMC_SPI_GPIO_MAP,sizeof(SD_MMC_SPI_GPIO_MAP) / sizeof(SD_MMC_SPI_GPIO_MAP[0]));
  // Initialize as master.
  spi_initMaster(SPI0, &SD_spiOptions);
  // Set SPI selection mode: variable_ps, pcs_decode, delay.
  spi_selectionMode(SPI0, 0, 0, 0);
  // Enable SPI module.
  spi_enable(SPI0);
  // Initialize SD/MMC driver with SPI clock (PBA).
  sd_mmc_spi_init(SD_spiOptions, PBA_SPEED);
  
  tc_init_fast(FAST_TC);
  tc_init_slow(SLOW_TC);
  
  static const gpio_map_t DACIFB_GPIO_MAP =
  {
    {AVR32_DACREF_PIN,AVR32_DACREF_FUNCTION},
    {AVR32_ADCREFP_PIN,AVR32_ADCREFP_FUNCTION},
    {AVR32_ADCREFN_PIN,AVR32_ADCREFN_FUNCTION},
    {DAC0A_pin, DAC0A_FUNCTION},
	{DAC1A_pin, DAC1A_FUNCTION}
  };
  
  gpio_enable_module(DACIFB_GPIO_MAP, sizeof(DACIFB_GPIO_MAP) / sizeof(DACIFB_GPIO_MAP[0]));
  
  dacifb_opt_t dacifb_opt = {
    .reference                  = DACIFB_REFERENCE_EXT ,        // VDDANA Reference
    .channel_selection          = DACIFB_CHANNEL_SELECTION_A,     // Selection Channels A&B
    .low_power                  = false,                          // Low Power Mode
    .dual                       = false,                          // Dual Mode
    .prescaler_clock_hz         = DAC_PRESCALER_CLK     // Prescaler Clock (Should be 500Khz)
};

// DAC Channel Configuration
dacifb_channel_opt_t dacifb_channel_opt = {
    .auto_refresh_mode    = true,                      // Auto Refresh Mode
    .trigger_mode         = DACIFB_TRIGGER_MODE_MANUAL, // Trigger selection
    .left_adjustment      = false,                      // Right Adjustment
    .data_shift           = 0,                          // Number of Data Shift
    .data_round_enable    = false                       // Data Rouding Mode                                              };
};

volatile avr32_dacifb_t *dacifb0 = &AVR32_DACIFB0; // DACIFB IP registers address
volatile avr32_dacifb_t *dacifb1 = &AVR32_DACIFB1; // DACIFB IP registers address

//The factory calibration for DADIFB is broken, so use manual calibration
//dacifb_get_calibration_data(DAC0,&dacifb_opt,0);

dacifb_opt.gain_calibration_value=0x0090;
dacifb_opt.offset_calibration_value=0x0153;

  // configure DACIFB0
dacifb_configure(DAC0,&dacifb_opt,PBA_SPEED);

dacifb_configure_channel(DAC0,DACIFB_CHANNEL_SELECTION_A,&dacifb_channel_opt,DAC_PRESCALER_CLK);

dacifb_start_channel(DAC0,DACIFB_CHANNEL_SELECTION_A,PBA_SPEED);

//The factory calibration for DADIFB is broken, so use manual calibration					
dacifb_set_value(DAC0,DACIFB_CHANNEL_SELECTION_A,false,1024);					   
					   				 
//dacifb_get_calibration_data(DAC1, &dacifb_opt,1);
							
dacifb_opt.gain_calibration_value=0x0084;
dacifb_opt.offset_calibration_value=0x0102;							

  // configure DACIFB1
dacifb_configure(DAC1,&dacifb_opt,PBA_SPEED);
				   
dacifb_configure_channel(DAC1,DACIFB_CHANNEL_SELECTION_A,&dacifb_channel_opt,DAC_PRESCALER_CLK);

dacifb_start_channel(DAC1,DACIFB_CHANNEL_SELECTION_A,PBA_SPEED);
					   
dacifb_set_value(DAC1,DACIFB_CHANNEL_SELECTION_A,false,1024);					   				   				 

DAC0->dr0=2048;
DAC1->dr0=4095;
}
