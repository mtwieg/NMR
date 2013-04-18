
#include <asf.h>
#include <AD9958.h>
#include <my_misc_functions.h>


void reset_DDS_local(void)
{
	gpio_local_set_gpio_pin(DDS_RESET_pin);
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n");
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n");
	asm volatile("nop\n");
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n");
	asm volatile("nop\n");
	gpio_local_clr_gpio_pin(DDS_RESET_pin);
}

void IO_UPDATE_local(void)
{
	while (!(DDS_SPI->sr & AVR32_SPI_SR_TXEMPTY_MASK));  //pulse lasts ~180ns, seems to be enough to reliably be recognized
	gpio_local_set_gpio_pin(DDS_IOUD_pin);
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	gpio_local_clr_gpio_pin(DDS_IOUD_pin);
}

void IO_UPDATE_local_long(void)
{
	while (!(DDS_SPI->sr & AVR32_SPI_SR_TXEMPTY_MASK));  //pulse lasts ~180ns, seems to be enough to reliably be recognized
	gpio_local_set_gpio_pin(DDS_IOUD_pin);
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n");
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n");
	asm volatile("nop\n");
	asm volatile("nop\n"); 
	asm volatile("nop\n"); 
	asm volatile("nop\n");
	asm volatile("nop\n");
	gpio_local_clr_gpio_pin(DDS_IOUD_pin);
}

void write_CSR(volatile DDS_options_t *DDS_options)
{
	my_fast_SPI_write(DDS_SPI, (register_write | CSR_ADDR),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI, ((DDS_options->channels)<<CH_EN_OFFSET | (DDS_options->SERmode)<<SER_MODE_OFFSET | (DDS_options->LSBfirst)<<LSB_FIRST_OFFSET),DDS_options->pcs, 1);
}


void write_FR1(volatile DDS_options_t *DDS_options)
{
	my_fast_SPI_write(DDS_SPI, (register_write | FR1_ADDR),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI, ((DDS_options->VCOgain)<<(VCO_GAIN_OFFSET-16) | (DDS_options->PLLdiv)<<(PLL_DIV_OFFSET-16) | (DDS_options->PLLdiv)<<(PLL_DIV_OFFSET-16) | (DDS_options->CHRGPMP)<<(CHRGPMP_OFFSET-16)),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI, ((DDS_options->PPC)<<(PPC_OFFSET-8) | (DDS_options->MODLEVEL)<<(MODLEVEL_OFFSET-8)),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI, 0,DDS_options->pcs, 1); //last byte should all be default values
}	


void write_FR2(volatile DDS_options_t *DDS_options)
{
	my_fast_SPI_write(DDS_SPI, (register_write | FR2_ADDR),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI, MY_DEFAULT_FR2,DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI, MY_DEFAULT_FR2,DDS_options->pcs, 1);
}	


void write_CFR(volatile DDS_channel_options_t *DDS_options)
{
	my_fast_SPI_write(DDS_SPI, (register_write | CFR_ADDR),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,(DDS_options->AFPmod)<<(AFP_OFFSET-16),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,(DDS_options->DACfsc)<<(DAC_FSC_OFFSET-8),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI, (DDS_options->coreenable)<<(DCPDN_OFFSET) | (DDS_options->DACenable)<<(DACPDN_OFFSET) | (DDS_options->matchedpipes)<<(MATCHED_PIPES_OFFSET) | (DDS_options->sinorcos)<<(SIN_OUT_OFFSET),DDS_options->pcs, 1);
}	


void write_ACR(volatile DDS_channel_options_t *DDS_options)
{
	my_fast_SPI_write(DDS_SPI, (register_write | ACR_ADDR),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,0,DDS_options->pcs, 0); //first byte is amplitude ramp rate, not using ramp so leave at 0
	my_fast_SPI_write(DDS_SPI, (DDS_options->ampmulenable)<<(AMP_MUL_EN_OFFSET-8) | (DDS_options->RURDenable)<<(AMP_RURD_EN_OFFSET-8) | (DDS_options->ASF)>>8,DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,(DDS_options->ASF) & 255,DDS_options->pcs, 1);
}

void write_FTW0(volatile DDS_channel_options_t *DDS_options)
{
	my_fast_SPI_write(DDS_SPI, (register_write | CFTW0_ADDR),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,(DDS_options->FTW)>>24,DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,(DDS_options->FTW)>>16,DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,(DDS_options->FTW)>>8,DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,(DDS_options->FTW),DDS_options->pcs, 1);
}	


void write_POW0(volatile DDS_channel_options_t *DDS_options)
{
	my_fast_SPI_write(DDS_SPI, (register_write | CPOW0_ADDR),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,(DDS_options->POW)>>8,DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,(DDS_options->POW),DDS_options->pcs, 1);
}

 //word should be 1-15 for addresses 0x0A-0x18
void write_profile_word_ftw(uint32_t ftw, uint8_t word, volatile DDS_channel_options_t *DDS_options)
{
	my_fast_SPI_write(DDS_SPI, (register_write | (CW1_ADDR+word-1)),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,ftw>>24,DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,ftw>>16,DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,ftw>>8,DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,ftw,DDS_options->pcs, 1);
}

 //word should be 1-15 for addresses 0x0A-0x18
void write_profile_word_pow(uint16_t pow, uint8_t word, volatile DDS_channel_options_t *DDS_options)
{
	uint32_t pow_32=((uint32_t)pow)<<18;
	my_fast_SPI_write(DDS_SPI, (register_write | (CW1_ADDR+word-1)),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,pow_32>>24,DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,pow_32>>16,DDS_options->pcs,0);
	my_fast_SPI_write(DDS_SPI,0,DDS_options->pcs,0);
	my_fast_SPI_write(DDS_SPI,0,DDS_options->pcs,1);
}


void write_profile_word_asf(uint16_t asf, uint8_t word, volatile DDS_channel_options_t *DDS_options)
{
	uint32_t asf_32=((uint32_t)asf)<<22;
	my_fast_SPI_write(DDS_SPI, (register_write | (CW1_ADDR+word-1)),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,asf_32>>24,DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,asf_32>>16,DDS_options->pcs,0);
	my_fast_SPI_write(DDS_SPI,0,DDS_options->pcs,0);
	my_fast_SPI_write(DDS_SPI,0,DDS_options->pcs,1);
}

uint32_t read_CSR(volatile DDS_options_t *DDS_options)
{
	my_slow_SPI_read(DDS_SPI, (register_read | CSR_ADDR),DDS_options->pcs, 0);
	return my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 1);
}

uint32_t read_FR1(volatile DDS_options_t *DDS_options)
{
	my_slow_SPI_read(DDS_SPI, (register_read | FR1_ADDR),DDS_options->pcs, 0);
	volatile uint32_t word = my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 0)<<16;
	word = word + (my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 0)<<8);
	word = word + (my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 1));
	return word;
}

uint32_t read_FR2(volatile DDS_options_t *DDS_options)
{
	my_slow_SPI_read(DDS_SPI, (register_read | FR2_ADDR),DDS_options->pcs, 0);
	volatile uint32_t word = my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 0)<<8;
	word = word + (my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 1));
	return word;
}

uint32_t read_CFR(volatile DDS_options_t *DDS_options)
{
	my_slow_SPI_read(DDS_SPI, (register_read | CFR_ADDR),DDS_options->pcs, 0);
	volatile uint32_t word = my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 0)<<16;
	word = word + (my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 0)<<8);
	word = word + (my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 1));
	return word;
}

uint32_t read_ACR(volatile DDS_options_t *DDS_options)
{
	my_fast_SPI_write(DDS_SPI, (register_read | ACR_ADDR),DDS_options->pcs, 0);
	volatile uint32_t word = my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 0)<<16;
	word = word + (my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 0)<<8);
	word = word + (my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 1));
	return word;
}

uint32_t read_profile_word(uint8_t addr, volatile DDS_options_t *DDS_options)
{
	my_fast_SPI_write(DDS_SPI, (register_read | (CW1_ADDR+addr-1)),DDS_options->pcs, 0);
	volatile uint32_t word = my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 0)<<24;
	word = word + (my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 0)<<16);
	word = word + (my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 0)<<8);
	word = word + (my_slow_SPI_read(DDS_SPI, 0,DDS_options->pcs, 1));
	return word;
}

void clear_phase_accumulators(volatile DDS_options_t *DDS_options) 
{
	my_fast_SPI_write(DDS_SPI, (register_write | FR2_ADDR),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI, MY_DEFAULT_FR2 | (CLEAR_BOTH_PHASE_ACCUMULATORS << (CLEAR_BOTH_PHASE_ACCUMULATORS_OFFSET-8)),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI, MY_DEFAULT_FR2,DDS_options->pcs, 1);
}	

void clear_phase_accumulators2(volatile DDS_channel_options_t *DDS_options) 
{
	my_fast_SPI_write(DDS_SPI, (register_write | CFR_ADDR),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,(DDS_options->AFPmod)<<(AFP_OFFSET-16),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI,(DDS_options->DACfsc)<<(DAC_FSC_OFFSET-8),DDS_options->pcs, 0);
	my_fast_SPI_write(DDS_SPI, (DDS_options->coreenable)<<(DCPDN_OFFSET) | (DDS_options->DACenable)<<(DACPDN_OFFSET) | (DDS_options->matchedpipes)<<(MATCHED_PIPES_OFFSET) | (CLEAR_PHASE_ACCUMULATOR<<CLEAR_PHASE_ACCUMULATOR_OFFSET) | (DDS_options->sinorcos)<<(SIN_OUT_OFFSET),DDS_options->pcs, 1);
}	