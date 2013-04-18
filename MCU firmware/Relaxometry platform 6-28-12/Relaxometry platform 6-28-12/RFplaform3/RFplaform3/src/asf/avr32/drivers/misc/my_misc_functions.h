typedef struct stEXCEPTION_TABLE //THIS TABLE IS MADE FOR AT32UC3C1512C, it has 47 interrupt groups total, and has 476 bytes
{
    unsigned long evUnrecoverableException;
    unsigned long evTLBmultipleHit;
    unsigned long evBusErrorDataFetch;
    unsigned long evBusErrorInstructionFetch;
    unsigned long evNonMaskableInterrupt;
    unsigned long evMissingAddress;
    unsigned long evITLBProtection;
    unsigned long evBreakPoint;
    unsigned long evIllegalOpcode;
    unsigned long evUnimplementedInstruction;
    unsigned long evPrivilegeViolation;
    unsigned long evFloatingPoint;
    unsigned long evCoprocessorAbsent;
    unsigned long evDataAddressRead;
    unsigned long evDataAddressWrite;
    unsigned long evDTLBProtectionRead;
    unsigned long evDTLBProtectionWrite;
    unsigned long evDTLBModified;
    unsigned long evGroup0;                                              // 0x048
    unsigned long evAdd0;                                                // 0x04c
    unsigned long evITLBMiss;
    unsigned long evGroup1;                                              // 0x054
    unsigned long evAdd1;                                                // 0x058
    unsigned long evRes1;                                                // 0x05c
    unsigned long evITLBMissRead;
    unsigned long evGroup2;                                              // 0x064
    unsigned long evAdd2;                                                // 0x068
    unsigned long evRes2;                                                // 0x06c
    unsigned long evITLBMissWrite;
    unsigned long evGroup3;                                              // 0x074
    unsigned long evAdd3;                                                // 0x078
    unsigned long evGroup4;                                              // 0x07c
    unsigned long evAdd4;                                                // 0x080
    unsigned long evGroup5;                                              // 0x084
    unsigned long evAdd5;                                                // 0x088
    unsigned long evGroup6;                                              // 0x08c
    unsigned long evAdd6;                                                // 0x090
    unsigned long evGroup7;                                              // 0x094
    unsigned long evAdd7;                                                // 0x098
    unsigned long evGroup8;                                              // 0x09c
    unsigned long evAdd8;                                                // 0x0a0
    unsigned long evGroup9;                                              // 0x0a4
    unsigned long evAdd9;                                                // 0x0a8
    unsigned long evGroup10;                                             // 0x0ac
    unsigned long evAdd10;                                               // 0x0b0
    unsigned long evGroup11;                                             // 0x0b4
    unsigned long evAdd11;                                               // 0x0b8
    unsigned long evGroup12;                                             // 0x0bc
    unsigned long evAdd12;                                               // 0x0c0
    unsigned long evGroup13;                                             // 0x0c4
    unsigned long evAdd13;                                               // 0x0c8
    unsigned long evGroup14;                                             // 0x0cc
    unsigned long evAdd14;                                               // 0x0d0
    unsigned long evGroup15;                                             // 0x0d4
    unsigned long evAdd15;                                               // 0x0d8
    unsigned long evGroup16;                                             // 0x0dc
    unsigned long evAdd16;                                               // 0x0e0
    unsigned long evGroup17;                                             // 0x0e4
    unsigned long evAdd17;                                               // 0x0e8
    unsigned long evGroup18;                                             // 0x0ec
    unsigned long evAdd18;                                               // 0x0f0
    unsigned long evGroup19;                                             // 0x0f4
    unsigned long evAdd19;                                               // 0x0f8
	
	unsigned long evGroup20;                                             // 0x0fc
    unsigned long evAdd20;                                               // 0x100
	unsigned long evGroup21;                                             // 0x104
    unsigned long evAdd21;                                               // 0x108
	unsigned long evGroup22;                                             // 0x10c
    unsigned long evAdd22;                                               // 0x110
	unsigned long evGroup23;                                             // 0x114
    unsigned long evAdd23;                                               // 0x118
	unsigned long evGroup24;                                             // 0x11c
    unsigned long evAdd24;                                               // 0x120
	unsigned long evGroup25;                                             // 0x124
    unsigned long evAdd25;                                               // 0x128
	unsigned long evGroup26;                                             // 0x12c
    unsigned long evAdd26;                                               // 0x130
	unsigned long evGroup27;                                             // 0x134
    unsigned long evAdd27;                                               // 0x138
	unsigned long evGroup28;                                             // 0x13c
    unsigned long evAdd28;                                               // 0x140
	unsigned long evGroup29;                                             // 0x144
    unsigned long evAdd29;                                               // 0x148
	
	unsigned long evGroup30;                                             // 0x14c
    unsigned long evAdd30;                                               // 0x150
	unsigned long evGroup31;                                             // 0x154
    unsigned long evAdd31;                                               // 0x158
	unsigned long evGroup32;                                             // 0x15c
    unsigned long evAdd32;                                               // 0x160
	unsigned long evGroup33;                                             // 0x164
    unsigned long evAdd33;                                               // 0x168
	unsigned long evGroup34;                                             // 0x16c
    unsigned long evAdd34;                                               // 0x170
	unsigned long evGroup35;                                             // 0x174
    unsigned long evAdd35;                                               // 0x178
	unsigned long evGroup36;                                             // 0x17c
    unsigned long evAdd36;                                               // 0x180
	unsigned long evGroup37;                                             // 0x184
    unsigned long evAdd37;                                               // 0x188
	unsigned long evGroup38;                                             // 0x18c
    unsigned long evAdd38;                                               // 0x190
	unsigned long evGroup39;                                             // 0x194
    unsigned long evAdd39;                                               // 0x198
	
	unsigned long evGroup40;                                             // 0x19c
    unsigned long evAdd40;                                               // 0x1A0
	unsigned long evGroup41;                                             // 0x1A4
    unsigned long evAdd41;                                               // 0x1A8
	unsigned long evGroup42;                                             // 0x1Ac
    unsigned long evAdd42;                                               // 0x1B0
	unsigned long evGroup43;                                             // 0x1B4
    unsigned long evAdd43;                                               // 0x1B8
	unsigned long evGroup44;                                             // 0x1Bc
    unsigned long evAdd44;                                               // 0x1C0
	unsigned long evGroup45;                                             // 0x1C4
    unsigned long evAdd45;                                               // 0x1C8
	unsigned long evGroup46;                                             // 0x1Cc
    unsigned long evAdd46;                                               // 0x1D0
	
    unsigned long evRes3;                                                // 0x1D4
    unsigned long evSupervisorCall;                                      // 0x1D8
} EXCEPTION_TABLE;

void my_fast_SPI_write(volatile avr32_spi_t *spi, uint16_t data,uint8_t pcs, uint8_t lastxfer);

void my_delay(volatile uint16_t count);

void print_long_binary(volatile avr32_usart_t *usart, uint32_t word);

void print_long_bits(volatile avr32_usart_t *usart, uint32_t word);

uint16_t my_slow_SPI_read(volatile avr32_spi_t *spi, uint16_t data,uint8_t pcs, uint8_t lastxfer);

#define setnextevent(t_rc) (AVR32_TC0.channel[0].rc=t_rc)

#define read_TC_sr() (temp=AVR32_TC0.channel[0].sr)

#define clear_TC_CV() (AVR32_TC0.channel[0].cv=0)

void AVR32_LowLevelInit(void);

void fnEnterAVRInterrupt(int iIntGroup, unsigned char ucIntLevel, void (*InterruptFunc)(void));

#define BRANCH_TO_SELF                   0xe08f0000                      // AVR32 machine code to create a forever loop
#define LOAD_PC_WITH_NEXT_VALUE          0x481fd703                      // LDDPC relative plus NOP

#define EXCEPTION_VECTOR_BASE_ADDRESS 0x00000000

#define INT_LEVEL_0                    0x00
#define INT_LEVEL_1                    0x40
#define INT_LEVEL_2                    0x80
#define INT_LEVEL_3                    0xc0

void my_pdca_init_channel(uint32_t pdca_ch_number, uint32_t addr, uint32_t size, uint32_t pid,  uint32_t r_addr, uint32_t r_size, uint32_t transfer_size);


static inline void set_profile_CH0_off(void)
{
gpio_local_clr_gpio_pin(DDS_P1_pin);
gpio_local_clr_gpio_pin(DDS_P0_pin);
}	
	
static inline void set_profile_CH0_amp90(void)
{
gpio_local_clr_gpio_pin(DDS_P1_pin);
gpio_local_set_gpio_pin(DDS_P0_pin);
}
	
static inline void set_profile_CH0_amp180(void)
{
gpio_local_set_gpio_pin(DDS_P1_pin);
gpio_local_set_gpio_pin(DDS_P0_pin);
}
	
static inline void set_profile_CH0_LOamp(void) 
{
gpio_local_set_gpio_pin(DDS_P1_pin);
gpio_local_clr_gpio_pin(DDS_P0_pin);
}	
	
static inline void set_profile_CH1_off(void)
{
gpio_local_clr_gpio_pin(DDS_P3_pin);
gpio_local_clr_gpio_pin(DDS_P2_pin);
}	
	
static inline void set_profile_CH1_amp90(void)
{
gpio_local_clr_gpio_pin(DDS_P3_pin);
gpio_local_set_gpio_pin(DDS_P2_pin);
}
	
static inline void set_profile_CH1_amp180(void)
{
gpio_local_set_gpio_pin(DDS_P3_pin);
gpio_local_set_gpio_pin(DDS_P2_pin);
}
	
static inline void set_profile_CH1_LOamp(void) 
{
gpio_local_set_gpio_pin(DDS_P3_pin);
gpio_local_clr_gpio_pin(DDS_P2_pin);
}

inline static void my_SPI_selectchip(volatile avr32_spi_t *spi, unsigned char chip)
{
	spi->mr &= ~(1 << (AVR32_SPI_MR_PCS_OFFSET + chip));
}
 
inline static void my_SPI_deselectchip(volatile avr32_spi_t *spi, unsigned char chip)
{
	spi->mr |= AVR32_SPI_MR_PCS_MASK;

	// Last transfer, so deassert the current NPCS if CSAAT is set.
	spi->cr = AVR32_SPI_CR_LASTXFER_MASK;
}

inline static void my_SD_SPI_write(volatile avr32_spi_t *spi, uint8_t data)
{
	while (!(spi->sr & AVR32_SPI_SR_TDRE_MASK));
	spi->tdr =data;
}

inline static uint8_t my_SD_SPI_write_read(volatile avr32_spi_t *spi, uint8_t data)
{
	while (!(spi->sr & AVR32_SPI_SR_TDRE_MASK));
	spi->tdr =data;
	while (!(spi->sr & AVR32_SPI_SR_TXEMPTY_MASK));
	return spi->rdr;
}

bool my_sd_mmc_spi_wait_not_busy_noCS(void);