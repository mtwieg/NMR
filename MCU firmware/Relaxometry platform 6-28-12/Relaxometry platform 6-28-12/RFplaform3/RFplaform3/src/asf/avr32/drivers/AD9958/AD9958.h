

#define register_write (0)
#define register_read (1<<7)
#define CSR_ADDR 0
#define FR1_ADDR 1
#define FR2_ADDR 2
#define CFR_ADDR 3
#define CFTW0_ADDR 4
#define CPOW0_ADDR 5
#define ACR_ADDR 6
#define LSRR_ADDR 7
#define RDW_ADDR 8
#define FDW_ADDR 9
#define CW1_ADDR 10
#define CW2_ADDR 11
#define CW3_ADDR 12
#define CW4_ADDR 13
#define CW5_ADDR 14
#define CW6_ADDR 15
#define CW7_ADDR 16
#define CW8_ADDR 17
#define CW9_ADDR 18
#define CW10_ADDR 19
#define CW11_ADDR 20
#define CW12_ADDR 21
#define CW13_ADDR 22
#define CW14_ADDR 23
#define CW15_ADDR 24


//CSR register
//[7:6] channel enable registers
#define CH_EN_OFFSET 6
#define CH_EN_0 1
#define CH_EN_1 2
#define CH_EN_BOTH 3
//[2:1] Serial I/O mode
#define SER_MODE_OFFSET 1	//only going to use one of these two modes, not 2 or 4 bit modes...
#define SER_MODE_2WIRE 0
#define SER_MODE_3WIRE 1
//[0] LSB/MSB first
#define LSB_FIRST_OFFSET 0
#define LSB_FIRST	1
#define MSB_FIRST	0

//FR1 register
//[23] VCO_GAIN
#define VCO_GAIN_OFFSET 23
#define VCO_HIGH 1
#define VCO_LOW 0
//[22:18] PLL_DIV
#define PLL_DIV_OFFSET 18
//[17:16]
#define CHRGPMP_OFFSET 16
#define CHRGPMP_75uA 0
#define CHRGPMP_100uA 1
#define CHRGPMP_125uA 2
#define CHRGPMP_150uA 3
//[14:12] Profile pin configuration (PPC)
#define PPC_OFFSET 12
//[11:10] RU/RD control
//not going to use RU/RD, so these bits will stay zero
//[9:8] modlevel
#define MODLEVEL_OFFSET 8
#define MODLEVEL_2	0
#define MODLEVEL_4	1
#define MODLEVEL_8	2
#define MODLEVEL_16	3
//[7] refclock input enable
#define modlevel_OFFSET 7
#define refclk_enable 0
#define refclk_disable 1
//[6] PDN mode
#define PDNmode_OFFSET 6
#define PDN_FAST 0
#define PDN_FULL 1
//[5] SYNC CLK disable
#define SYNC_CLK_EN_OFFSET 5
#define SYNC_CLK_EN 0
#define SYNC_CLK_DIS 1
//[4] DAC REF power down
#define DAC_REF_PDN_OFFSET 5
#define DAC_REF_EN 0
#define DAC_REF_DIS 1
//[3:2]  open?
//[1] manual hardwar sync
#define HRDWR_SYNC_EN_OFFSET 1
#define HRDWR_SYNC_DIS	0
#define HRDWR_SYNC_EN	1
//[0] manual hardwar sync
#define SFTWR_SYNC_EN_OFFSET 0
#define SFTWR_SYNC_DIS	0
#define SFTWR_SYNC_EN	1

//FR2 register
//FR2 seems to have only to do with do with the operation of the accumulators,
//and the syncing of devices.  Not going to want to mess with these, so I'll set them fixed for now...
#define MY_DEFAULT_FR2 0  //everything disabled or set to default
#define CLEAR_BOTH_PHASE_ACCUMULATORS 1
#define CLEAR_BOTH_PHASE_ACCUMULATORS_OFFSET 12


//CFR register
//[23:22] modulation type AFP
#define AFP_OFFSET 22
#define Nomod 0
#define Amod 1
#define Fmod 2
#define Pmod 3
//[21:16] open
//[15:13] these bits are for linear sweeps, deactivate them all by leaving them as 0
//[12:11] open
//[10] must be zero
//[9:8] DAC full scale current
#define DAC_FSC_OFFSET 8
#define DAC_FSC_FULL 3
//[7] digital core power down
#define DCPDN_OFFSET 7
#define DCPDN_OFF 1
#define DCPDN_ON 0
//[6] digital core power down
#define DACPDN_OFFSET 6
#define DACPDN_OFF 1
#define DACPDN_ON 0
//[5] matched pipe delay enable
#define MATCHED_PIPES_OFFSET 5
#define MATCHED_PIPES_EN 1
#define MATCHED_PIPES_DIS 0
//[4] autoclear sweep accum, not even using sweeps, so leave at default 0
//[3] manually clear sweep accum.  Not using.
//[2] autoclear phase accum.  not going to use this, leave as default 0
//[1] manually clear phase accum.  Not using
#define CLEAR_PHASE_ACCUMULATOR 1
#define CLEAR_PHASE_ACCUMULATOR_OFFSET 1
//[0] sine or cosine out
#define SIN_OUT_OFFSET 0
#define COS_OUT 0
#define SIN_OUT 1

//ACR register
//[23:16] amplitude ramp rate
#define AMP_RR_OFFSET 16
//[15:14] amplitude step size
#define AMP_SS_OFFSET 14
//[13] open
//[12] amplitude multiplier enable
#define AMP_MUL_EN_OFFSET 12
#define AMP_MUL_EN 1
#define AMP_MUL_DIS 0
//[11] RUDR enable
#define AMP_RURD_EN_OFFSET 11
#define AMP_MUL_MANUAL 0
#define AMP_MUL_AUTO 1
//[10] load ARR at IOUPDATE, not using ramp, so leave at default
//[9:0] amplitude scale factor

typedef struct
{
	uint8_t pcs;		//! The DDS channel(s) to set up.
    uint8_t channels;		//! modulation type
    uint8_t AFPmod;		//! dac full scale current
    uint8_t DACfsc;		//! core power down
    uint8_t coreenable;		//! DAC power down
    uint8_t DACenable;		//! matched pipes
    uint8_t matchedpipes;		//! sin or cos output
    uint8_t sinorcos;
    uint8_t ampmulenable;
    uint8_t RURDenable;
    uint16_t ASF;
    uint32_t FTW;
    uint16_t POW;
} DDS_channel_options_t;

typedef struct
{
	uint8_t pcs;		//! The DDS channel(s) to set up.
    uint8_t channels;		//! serial mode
    uint8_t SERmode;		//! LSB or MSB first
    uint8_t LSBfirst;		//! Gain range of VCO
    uint8_t VCOgain;		//! PLL divisor
    uint8_t PLLdiv;		//! charge pump current
    uint8_t CHRGPMP;		//! levels of modulation
    uint8_t PPC;			//! PDN mode
    uint8_t MODLEVEL;		//! modulation type
} DDS_options_t;

void reset_DDS_local(void);

void IO_UPDATE_local(void);

void IO_UPDATE_local_long(void);

void write_CSR(volatile DDS_options_t *DDS_options);

void write_FR1(volatile DDS_options_t *DDS_options);

void write_FR2(volatile DDS_options_t *DDS_options);

void write_CFR(volatile DDS_channel_options_t *DDS_options);

void write_ACR(volatile DDS_channel_options_t *DDS_options);

void write_FTW0(volatile DDS_channel_options_t *DDS_options);

void write_POW0(volatile DDS_channel_options_t *DDS_options);

void write_profile_word_ftw(uint32_t ftw, uint8_t word, volatile DDS_channel_options_t *DDS_options);

void write_profile_word_pow(uint16_t pow, uint8_t word, volatile DDS_channel_options_t *DDS_options);

void write_profile_word_asf(uint16_t asf, uint8_t word, volatile DDS_channel_options_t *DDS_options); //word should be 1-15 for addresses 0x0A-0x18

uint32_t read_CSR(volatile DDS_options_t *DDS_options);

uint32_t read_FR1(volatile DDS_options_t *DDS_options);

uint32_t read_FR2(volatile DDS_options_t *DDS_options);

uint32_t read_CFR(volatile DDS_options_t *DDS_options);

uint32_t read_ACR(volatile DDS_options_t *DDS_options);

uint32_t read_profile_word(uint8_t addr, volatile DDS_options_t *DDS_options);

void clear_phase_accumulators(volatile DDS_options_t *DDS_options);

void clear_phase_accumulators2(volatile DDS_channel_options_t *DDS_options);


