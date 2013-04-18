/**
 * \file
 *
 * \brief User board definition template
 *
 */

#  define USB_USART                 (&AVR32_USART1)			//USB USART assignments third version of RF platform
#  define USB_USART_TX_PDCA_PID		AVR32_PDCA_PID_USART1_TX
#  define USB_USART_RX_PDCA_PID		AVR32_PDCA_PID_USART1_RX
#  define USB_USART_RX_PIN          AVR32_USART1_RXD_0_1_PIN	//PD12
#  define USB_USART_RX_FUNCTION     AVR32_USART1_RXD_0_1_FUNCTION	//
#  define USB_USART_TX_PIN          AVR32_USART1_TXD_0_1_PIN		//PD11
#  define USB_USART_TX_FUNCTION     AVR32_USART1_TXD_0_1_FUNCTION	//
#  define USB_USART_CTS_PIN          AVR32_USART1_CTS_0_1_PIN		//PD11
#  define USB_USART_CTS_FUNCTION     AVR32_USART1_CTS_0_1_FUNCTION	//
#  define USB_USART_RTS_PIN          AVR32_USART1_RTS_0_1_PIN		//PD11
#  define USB_USART_RTS_FUNCTION     AVR32_USART1_RTS_0_1_FUNCTION	//

#  define LCD_USART                 (&AVR32_USART0)			//USART for communicating with LCD
#  define LCD_USART_RX_PDCA_PID		AVR32_PDCA_PID_USART0_RX
#  define LCD_USART_RX_PIN          AVR32_USART0_RXD_0_0_PIN	//PD28
#  define LCD_USART_RX_FUNCTION     AVR32_USART0_RXD_0_0_FUNCTION  //
#  define LCD_USART_TX_PIN          AVR32_USART0_TXD_0_0_PIN		//PD27
#  define LCD_USART_TX_FUNCTION     AVR32_USART0_TXD_0_0_FUNCTION	//

#  define SPI1					    (&AVR32_SPI1)				//THESE ARE SPI ASSIGNMENTS FOR EVK HACK
#  define SPI1_RX_PDCA_PID				AVR32_PDCA_PID_SPI1_RX
#  define SPI1_NPCS2_PIN				AVR32_SPI1_NPCS_3_2_PIN 		//PAB23 is DDS CS
#  define SPI1_NPCS2_FUNCTION			AVR32_SPI1_NPCS_3_2_FUNCTION
#  define SPI1_MISO_PIN				AVR32_SPI1_MISO_0_1_PIN		//PB20
#  define SPI1_MISO_FUNCTION			AVR32_SPI1_MISO_0_1_FUNCTION
#  define SPI1_MOSI_PIN				AVR32_SPI1_MOSI_0_1_PIN		//PB19
#  define SPI1_MOSI_FUNCTION			AVR32_SPI1_MOSI_0_1_FUNCTION
#  define SPI1_SCK_PIN				AVR32_SPI1_SCK_0_1_PIN 		//PB21
#  define SPI1_SCK_FUNCTION			AVR32_SPI1_SCK_0_1_FUNCTION

#  define SPI0						   (&AVR32_SPI0)				//THESE ARE SPI ASSIGNMENTS FOR EVK HACK
#  define SPI0_TX_PDCA_PID			AVR32_PDCA_PID_SPI0_TX
#  define SPI0_RX_PDCA_PID			AVR32_PDCA_PID_SPI0_RX
#  define SD_MMC_SPI					(&AVR32_SPI0)
#  define SD_MMC_SPI_NPCS				0
#  define SPI0_RX_PDCA_PID				AVR32_PDCA_PID_SPI0_RX
#  define SPI0_NPCS0_PIN				AVR32_SPI0_NPCS_0_0_PIN		//PD03 is NPCS0, CS for SD card
#  define SPI0_NPCS0_FUNCTION			AVR32_SPI0_NPCS_0_0_FUNCTION
#  define SPI0_MISO_PIN				AVR32_SPI0_MISO_0_0_PIN		//PD01
#  define SPI0_MISO_FUNCTION			AVR32_SPI0_MISO_0_0_FUNCTION
#  define SPI0_MOSI_PIN				AVR32_SPI0_MOSI_0_0_PIN  	//PD00
#  define SPI0_MOSI_FUNCTION			AVR32_SPI0_MOSI_0_0_FUNCTION
#  define SPI0_SCK_PIN				AVR32_SPI0_SCK_0_0_PIN 		//PD02
#  define SPI0_SCK_FUNCTION			AVR32_SPI0_SCK_0_0_FUNCTION

//GPIO assignments for RF platform 3
#  define ADC_CONV_pin					AVR32_PIN_PC06	
#  define DDS_IOUD_pin					AVR32_PIN_PB23
#  define DDS_RESET_pin					AVR32_PIN_PA13
#  define DDS_PDN_pin					AVR32_PIN_PA11
#  define DDS_P0_pin					AVR32_PIN_PC03
#  define DDS_P1_pin					AVR32_PIN_PC02
#  define DDS_P2_pin					AVR32_PIN_PC01
#  define DDS_P3_pin					AVR32_PIN_PC00

#  define TPAbias_pin					AVR32_PIN_PA08
#  define TXSW_pin						AVR32_PIN_PA07
#  define RXSW_pin						AVR32_PIN_PA06
#  define PWM0_pin						AVR32_PIN_PC17
#  define PWM1_pin						AVR32_PIN_PC15
#  define GEN1_pin						AVR32_PIN_PA09
#  define GEN2_pin						AVR32_PIN_PA10
#  define SD_detect_pin					AVR32_PIN_PD10

#  define DAC0					(&AVR32_DACIFB0)
#  define DAC1					(&AVR32_DACIFB1)
#  define DAC0A_pin				AVR32_DAC0A_PIN  	//PD00
#  define DAC0A_FUNCTION			AVR32_DAC0A_FUNCTION
#  define DAC1A_pin				AVR32_DAC1A_PIN  	//PD00
#  define DAC1A_FUNCTION			AVR32_DAC1A_FUNCTION

# define DAC_PRESCALER_CLK	7500000

#define DDS_PCS 7
#define ADC_PCS 14

#define FAST_TC				(&AVR32_TC0)
#define FAST_TC_CHANNEL		0
#define SLOW_TC				(&AVR32_TC1)
#define SLOW_TC_fast_CHANNEL		1
#define SLOW_TC_slow_CHANNEL		0

#define TC1_TCIOA1_pin			AVR32_PIN_PB02

#define OSC0_STARTUP    0	//for external oscillator
#define FOSC0       20000000
#define CPU_SPEED   60000000
#define PBA_SPEED   60000000

#define desired_DDS_Fs 300000000
#if (desired_DDS_Fs>500000000 || (desired_DDS_Fs>160000000 && desired_DDS_Fs < 255000000))
#error "Specified Fs is out of range"
#endif

#define DDS_PLL_div (desired_DDS_Fs/FOSC0)
#if (DDS_PLL_div<4 || DDS_PLL_div>20)
#error "DDS PLL multiplication factor out of range"
#endif

#define DDS_Fs (FOSC0*DDS_PLL_div)
#if (DDS_Fs>500000000 || (DDS_Fs>160000000 && DDS_Fs < 255000000))
#error "Actual Fs is out of range"
#endif

#if DDS_Fs<=160000000
#define DDS_VCO_gain 0
#else
#define DDS_VCO_gain 1
#endif

#define SD_SPI_SPEED 20000000
#define DDS_SPI		(&AVR32_SPI1)

#define SPI1_RX_PDCA_CHANNEL 0
#define SPI0_RX_PDCA_CHANNEL 1
#define SPI0_TX_PDCA_CHANNEL 2
#define USB_USART_TX_PDCA_CHANNEL 3
#define USB_USART_RX_PDCA_CHANNEL 4
#define LCD_USART_RX_PDCA_CHANNEL 5

#define MY_PDCA					    (&AVR32_PDCA)				//THESE ARE SPI ASSIGNMENTS FOR EVK HACK

#  define EXAMPLE_TARGET_PBACLK_FREQ_HZ FOSC0  // PBA clock target frequency, in Hz

#ifndef USER_BOARD_H
#define USER_BOARD_H

#endif // USER_BOARD_H
