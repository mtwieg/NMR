#include <asf.h>
#include <my_misc_functions.h>

void my_fast_SPI_write(volatile avr32_spi_t *spi, uint16_t data,uint8_t pcs, uint8_t lastxfer)
{
	while (!(spi->sr & AVR32_SPI_SR_TDRE_MASK));
	spi->tdr = (data & 0x00FF) | (pcs << AVR32_SPI_TDR_PCS_OFFSET) | (lastxfer << AVR32_SPI_TDR_LASTXFER_OFFSET); 
}

uint16_t my_slow_SPI_read(volatile avr32_spi_t *spi, uint16_t data,uint8_t pcs, uint8_t lastxfer)
{
	while (!(spi->sr & AVR32_SPI_SR_TXEMPTY_MASK));
	if((spi->sr & AVR32_SPI_SR_RDRF_MASK) != 0)
	{
		uint32_t temp=spi->rdr;
	}
	while (!(spi->sr & AVR32_SPI_SR_TDRE_MASK));
	spi->tdr = (data & 0x00FF) | (pcs << AVR32_SPI_TDR_PCS_OFFSET) | (lastxfer << AVR32_SPI_TDR_LASTXFER_OFFSET); 
	while (!(spi->sr & AVR32_SPI_SR_RDRF_MASK));
	return spi->rdr;
}

void my_delay(volatile uint16_t count)	//rough time delay
{
	for(volatile int16_t i=0;i<count;i++)
	{
		for(volatile int16_t j=0;j<5;j++)
		{
			
		}
	}
}


//The default interrupt handling library from ASF utter garbage. Need to do it my own way.
//TO USE THIS CODE, THE LINKER SCRIPT MUST BE CHANGED TO RESERVE A PORTION OF THE SRAM FOR THE INTERRUPT TABLE
//SHOULD LOOK SOMETHING LIKE THIS:

//MEMORY
//{
    //FLASH (rxai!w) : ORIGIN = 0x80000000, LENGTH = 512K
    //CPUSRAM (wxa!ri) : ORIGIN = 0x000001DC, LENGTH = 0xFE24			<<ORIGIN IS OFFSET
    //USERPAGE : ORIGIN = 0x80800000, LENGTH = 512
    //FACTORYPAGE : ORIGIN = 0x80800200, LENGTH = 512
    //FLASHVAULT_FLASH_SIZE (r) : ORIGIN = 0x80800400, LENGTH = 8
    //FLASHVAULT_RAM_SIZE (r) : ORIGIN = 0x80800408, LENGTH = 8
//}

extern void _evba;
static const unsigned char ucGroupLocation[] = {18, 21, 25};
void AVR32_LowLevelInit(void)	
{
    EXCEPTION_TABLE *ptrEventTable = (EXCEPTION_TABLE *)EXCEPTION_VECTOR_BASE_ADDRESS; // place an event table at the start of RAM
    unsigned long *ulPtrEntries = &ptrEventTable->evUnrecoverableException;
    int i = 0;
    while (i++ < (sizeof(EXCEPTION_TABLE)/sizeof(unsigned long))) {
        *ulPtrEntries++ = BRANCH_TO_SELF;                                // fill the event table with forever loops to catch unexpected exceptions
    }
	Set_system_register(AVR32_EVBA, EXCEPTION_VECTOR_BASE_ADDRESS);
}

void fnEnterAVRInterrupt(int iIntGroup, unsigned char ucIntLevel, void (*InterruptFunc)(void))
{
    unsigned long *ptrEventTable = (unsigned long *)EXCEPTION_VECTOR_BASE_ADDRESS; //start *ptr event table as EVBA
    unsigned long *ptrIntPriority = (unsigned long *)AVR32_INTC_ADDRESS;
    unsigned long ulGroupLocation;
    ptrIntPriority += iIntGroup;
    if (iIntGroup >= 3) {		//find offset of interrupt table entry relative to EVBA
        ulGroupLocation = (29 + ((iIntGroup - 3) * 2));
    }
    else {
        ulGroupLocation = ucGroupLocation[iIntGroup];
    }
    ptrEventTable += ulGroupLocation;	
    *ptrEventTable++ = LOAD_PC_WITH_NEXT_VALUE;
    *ptrEventTable = (unsigned long)InterruptFunc;
    *ptrIntPriority = (((unsigned long)ucIntLevel << 24) | ((ulGroupLocation) << 2));
} 

void my_pdca_init_channel(uint32_t pdca_ch_number, uint32_t addr, uint32_t size, uint32_t pid,  uint32_t r_addr, uint32_t r_size, uint32_t transfer_size)
{
	volatile avr32_pdca_channel_t *pdca_channel = &AVR32_PDCA.channel[pdca_ch_number];	
	pdca_channel->mar = addr;
  pdca_channel->tcr = size;
  pdca_channel->psr = pid;
  pdca_channel->marr = r_addr;
  pdca_channel->tcrr = r_size;
  pdca_channel->mr =transfer_size << AVR32_PDCA_SIZE_OFFSET;
  
}  