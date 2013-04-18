/*
 * NMRsequence.c
 *
 * Created: 3/3/2012 5:10:31 PM
 *  Author: Twieg
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

#define t_ms 7500

extern DDS_options_t DDS_options;
extern DDS_channel_options_t DDS_options_CH0;
extern DDS_channel_options_t DDS_options_CH1;
extern sequence_t currentsequence;
extern experiment_t experiment;

extern t_sequence_t t_currentsequence;
extern t_experiment_t t_experiment;

extern bool data_fail;

extern uint8_t bank0[samplebuffer_size];
extern uint8_t bank1[samplebuffer_size];

extern uint32_t temp;

#define no_problem 0;
#define T3_bad 1;
#define T5_bad 2;
#define writerate_high 3;
#define TR_Tcpmg_short 4;

uint8_t experiment_problem=0;

void initializeDDS(void)
{
	gpio_local_clr_gpio_pin(DDS_PDN_pin);

	SLEEP(AVR32_PM_SMODE_IDLE);
	
	reset_DDS_local();
	
	DDS_options.channels=CH_EN_0;
	write_CSR(&DDS_options);
	while (!(DDS_SPI->sr & AVR32_SPI_SR_TXEMPTY_MASK));
	
	SLEEP(AVR32_PM_SMODE_IDLE);
	IO_UPDATE_local_long();
	//write common registers
	write_FR1(&DDS_options);
	while (!(DDS_SPI->sr & AVR32_SPI_SR_TXEMPTY_MASK));
	write_FR2(&DDS_options);
	while (!(DDS_SPI->sr & AVR32_SPI_SR_TXEMPTY_MASK));
	
	SLEEP(AVR32_PM_SMODE_IDLE);
	IO_UPDATE_local_long();
	
	//configure DDS channels
	DDS_options.channels=CH_EN_0;
	DDS_options_CH0.POW=t_currentsequence.preppulse[0].RFphase_POW;
	DDS_options_CH0.ASF=0;
	DDS_options_CH0.FTW=t_currentsequence.f0_FTW;
	
	//write_CSR(&DDS_options);
	write_CFR(&DDS_options_CH0);
	write_POW0(&DDS_options_CH0);
	write_ACR(&DDS_options_CH0);
	write_FTW0(&DDS_options_CH0);
	//write profile registers 1, 2, and 3 with amplitudes
	//P0 is zero amplitude
	//P1 is RFamp of prep pulse
	//P2 is LO amp
	//P3 is RF amp of CPMG pulse
	write_profile_word_asf(t_currentsequence.preppulse[0].RFamp,2,&DDS_options_CH0);  //There is something up with the AD9958...
	write_profile_word_asf(t_currentsequence.CPMG.RFamp,3,&DDS_options_CH0);//it always mixes up profile 1 and profile 2...
	write_profile_word_asf(t_currentsequence.CPMG.LOamp,1,&DDS_options_CH0);//so switch them for now...
	
	//write default tuning words for CH1
	//CH1 is by default used for the quadrature LO
	//so initialize its phase as LOph_Q_pow, and its amplitude as LOamp
	DDS_options.channels=CH_EN_1;
	DDS_options_CH1.POW=t_currentsequence.CPMG.LOphase_Q_POW;
	DDS_options_CH1.ASF=0;
	DDS_options_CH1.FTW=t_currentsequence.f0_FTW;
	write_CSR(&DDS_options);
	
	write_CFR(&DDS_options_CH1);
	write_POW0(&DDS_options_CH1);
	write_ACR(&DDS_options_CH1);
	write_FTW0(&DDS_options_CH1);
	//write profile registers 1, 2, and 3 with amplitudes
	//P0 is zero amplitude
	//P1 is RFamp of prep pulse
	//P2 is LO amp
	//P3 is RF amp of CPMG pulse
	write_profile_word_asf(t_currentsequence.preppulse[0].RFamp,2,&DDS_options_CH1);  //There is something up with the AD9958...
	write_profile_word_asf(t_currentsequence.CPMG.RFamp,3,&DDS_options_CH1);//it always mixes up profile 1 and profile 2...
	write_profile_word_asf(t_currentsequence.CPMG.LOamp,1,&DDS_options_CH1);//so switch them for now...
	
	SLEEP(AVR32_PM_SMODE_IDLE);
	IO_UPDATE_local();
	
	DDS_options.channels=CH_EN_0;
	write_CSR(&DDS_options); //set to write to TX channel
	
	set_profile_CH0_off();
	set_profile_CH1_LOamp();
}

void executesequence_SDstorage_multiprep_combined(void)
{	
	initialize_SD_arbitration();
	
	setnextevent(750);
	cpu_irq_enable();	
	tc_start(FAST_TC, FAST_TC_CHANNEL);
	SLEEP(AVR32_PM_SMODE_IDLE);
	
	DAC0->dr0=t_currentsequence.preppulse[0].Vb;
	
	initializeDDS();
	
	temp=SPI1->rdr;	//read RDR and SR so that PDCA does not do a transfer when enabled
	temp=SPI1->sr;
	pdca_disable(SPI1_RX_PDCA_CHANNEL);
	my_pdca_init_channel(SPI1_RX_PDCA_CHANNEL,(uint32_t)(&bank0),(samplebuffer_size/2), SPI1_RX_PDCA_PID,   (uint32_t)(&bank1),  (samplebuffer_size/2), PDCA_TRANSFER_SIZE_HALF_WORD);
	
	pdca_disable(SPI0_TX_PDCA_CHANNEL);
	my_pdca_init_channel(SPI0_TX_PDCA_CHANNEL,(uint32_t)(&bank1),0, SPI0_TX_PDCA_PID, 0, 0, PDCA_TRANSFER_SIZE_BYTE);	//initialize with 0 in TCR and TCRR so that nothing happens until TCR is written
	pdca_enable(SPI0_TX_PDCA_CHANNEL); //needs to be enabled at start of a sequence!
	pdca_disable(SPI0_RX_PDCA_CHANNEL);
	my_pdca_init_channel(SPI0_RX_PDCA_CHANNEL,(uint32_t)(&bank0),0, SPI0_RX_PDCA_PID, 0, 0, PDCA_TRANSFER_SIZE_BYTE);	//initialize with 0 in TCR and TCRR so that nothing happens until TCR is written
	
	
	//SEQUENCE START
	
	
	for(uint8_t i=0;i<t_currentsequence.N_preppulses;i++)
	{
		DDS_options_CH0.POW=t_currentsequence.preppulse[i].RFphase_POW;
		write_POW0(&DDS_options_CH0);	//update DDS with new RF pulse phase and RF amplitude
		write_profile_word_asf(t_currentsequence.preppulse[i].RFamp,2,&DDS_options_CH0);
		IO_UPDATE_local();
	
		SLEEP(AVR32_PM_SMODE_IDLE);
	
		setnextevent(t_experiment.t_TPArise);		//START OF EXCITEMENT PULSE
		gpio_local_set_gpio_pin(TPAbias_pin);		//enable linear TPA bias	
		gpio_local_set_gpio_pin(TXSW_pin);			//TXSW ON
		SLEEP(AVR32_PM_SMODE_IDLE);
	
		set_profile_CH0_amp90();
		gpio_local_set_gpio_pin(GEN2_pin);			//enable CMCD TPA
		
		if(t_currentsequence.preppulse[i].ms_pulsewidth==0)
		{
			setnextevent(t_currentsequence.preppulse[i].t_pulsewidth);
		}
		else
		{
			for(uint16_t mscount=0;mscount<t_currentsequence.preppulse[i].ms_pulsewidth;mscount++)
			{
				setnextevent(t_ms);
				SLEEP(AVR32_PM_SMODE_IDLE);
			}
			setnextevent(t_currentsequence.preppulse[i].t_pulsewidth);
		}
		
		SLEEP(AVR32_PM_SMODE_IDLE);
		set_profile_CH0_off();
		gpio_local_clr_gpio_pin(GEN2_pin);			//CMCD TPA off
		gpio_local_clr_gpio_pin(TXSW_pin);			//TXSW OFF
		gpio_local_clr_gpio_pin(TPAbias_pin);		//linear TPA bias off
		
		if((i+1)<t_currentsequence.N_preppulses) //is next pulse a prep pulse?
		{	//if next pulse is a prep pulse, set as next prep pulse Vb
			DAC0->dr0=t_currentsequence.preppulse[i+1].Vb;
		}
		else
		{	//if next pulse is CPMG, then set as CPMG Vb
			DAC0->dr0=t_currentsequence.CPMG.Vb;
		}
		
		if(t_currentsequence.preppulse[i].ms_T0==0)
		{
			setnextevent(t_currentsequence.preppulse[i].t_T0);
		}
		else
		{
			for(uint16_t mscount=0;mscount<t_currentsequence.preppulse[i].ms_T0;mscount++)
			{
				setnextevent(t_ms);
				SLEEP(AVR32_PM_SMODE_IDLE);
			}
			setnextevent(t_currentsequence.preppulse[i].t_T0);
		}
	}
	
	uint16_t echocount=0;
	
	if(t_currentsequence.echofirst==true)
	{	//if echo is first, do sampling routine, and increment echocount
		DDS_options_CH0.POW=t_currentsequence.CPMG.LOphase_I_POW;
		write_POW0(&DDS_options_CH0);	//if set for quadrature decoding, change TX phase to LO phase
		IO_UPDATE_local();
		set_profile_CH0_LOamp(); //enable LOamp on CH0
	
		while (!(SPI1->sr & AVR32_SPI_SR_TXEMPTY_MASK));//don't disable PDCA until last SPI transfer is done!
		temp=SPI1->rdr;	//read RDR and SR so that PDCA does not do a transfer when enabled
		temp=SPI1->sr;
		pdca_enable(SPI1_RX_PDCA_CHANNEL);
		for(uint32_t i=0;i<t_currentsequence.CPMG.Nsamp;i++)
		{
			SLEEP(AVR32_PM_SMODE_IDLE);
			gpio_local_set_gpio_pin(ADC_CONV_pin);
			SPI1->tdr = (0x0000) | (ADC_PCS << AVR32_SPI_TDR_PCS_OFFSET) | (0 << AVR32_SPI_TDR_LASTXFER_OFFSET);
			gpio_local_clr_gpio_pin(ADC_CONV_pin);
			SPI1->tdr = (0x0000) | (ADC_PCS << AVR32_SPI_TDR_PCS_OFFSET) | (1 << AVR32_SPI_TDR_LASTXFER_OFFSET);
			setnextevent(t_currentsequence.CPMG.t_dt);
		}
		setnextevent(t_currentsequence.CPMG.t_T5);
		while (!(SPI1->sr & AVR32_SPI_SR_TXEMPTY_MASK));//don't disable PDCA until last SPI transfer is done!
		pdca_disable(SPI1_RX_PDCA_CHANNEL);
	
		DDS_options_CH0.POW=t_currentsequence.CPMG.RFphase_POW;
		write_POW0(&DDS_options_CH0);	//change TX phase back to 90
		IO_UPDATE_local();
		set_profile_CH0_off(); //and turn RF back off
		
		while (!(SPI1->sr & AVR32_SPI_SR_TXEMPTY_MASK));//don't disable PDCA until last SPI transfer is done!
		gpio_local_set_gpio_pin(GEN1_pin);
		handle_SPI_arbitration();
		gpio_local_clr_gpio_pin(GEN1_pin);
		
		echocount++;
	}
	else
	{	//if pulse is first, just wait to do a pulse
		DDS_options_CH0.POW=t_currentsequence.CPMG.RFphase_POW;
		write_POW0(&DDS_options_CH0);	//change TX phase back to 90
		IO_UPDATE_local();
	}
	
	while(echocount<t_currentsequence.CPMG.Nechos)
	{
		echocount++;
		SLEEP(AVR32_PM_SMODE_IDLE);					
		gpio_local_clr_gpio_pin(RXSW_pin);			//RXSW OFF
		gpio_local_set_gpio_pin(TPAbias_pin);		//enable linear TPA bias	
		gpio_local_set_gpio_pin(TXSW_pin);			//TXSW ON
		setnextevent(t_experiment.t_TPArise);
		SLEEP(AVR32_PM_SMODE_IDLE);
	
		set_profile_CH0_amp180();
		gpio_local_set_gpio_pin(GEN2_pin);			//enable CMCD TPA
		setnextevent(t_currentsequence.CPMG.t_pulsewidth);
		SLEEP(AVR32_PM_SMODE_IDLE);
		set_profile_CH0_off();
		gpio_local_clr_gpio_pin(GEN2_pin);			//CMCD TPA off
		gpio_local_clr_gpio_pin(TXSW_pin);			//TXSW OFF
		gpio_local_clr_gpio_pin(TPAbias_pin);		//linear TPA bias off
		setnextevent(t_currentsequence.CPMG.t_T3);
	
		DDS_options_CH0.POW=t_currentsequence.CPMG.LOphase_I_POW;
		write_POW0(&DDS_options_CH0);	//change TX phase to LO phase
		IO_UPDATE_local();
		gpio_local_set_gpio_pin(RXSW_pin);			//RXSW ON
		set_profile_CH0_LOamp();					//enable LOamp on CH0
		
		//gpio_local_set_gpio_pin(GEN1_pin);		//debug signal
		handle_SD();
		//gpio_local_clr_gpio_pin(GEN1_pin);		//debug signal
	
		while (!(SPI1->sr & AVR32_SPI_SR_TXEMPTY_MASK));//don't disable PDCA until last SPI transfer is done!
		temp=SPI1->rdr;	//read RDR and SR so that PDCA does not do a transfer when enabled
		temp=SPI1->sr;
		pdca_enable(SPI1_RX_PDCA_CHANNEL);
		for(uint32_t j=0;j<t_currentsequence.CPMG.Nsamp;j++)
		{
			SLEEP(AVR32_PM_SMODE_IDLE);
			gpio_local_set_gpio_pin(ADC_CONV_pin);
			SPI1->tdr = (0x0000) | (ADC_PCS << AVR32_SPI_TDR_PCS_OFFSET) | (0 << AVR32_SPI_TDR_LASTXFER_OFFSET);
			gpio_local_clr_gpio_pin(ADC_CONV_pin);
			SPI1->tdr = (0x0000) | (ADC_PCS << AVR32_SPI_TDR_PCS_OFFSET) | (1 << AVR32_SPI_TDR_LASTXFER_OFFSET);
			setnextevent(t_currentsequence.CPMG.t_dt);
		}
		setnextevent(t_currentsequence.CPMG.t_T5);
		while (!(SPI1->sr & AVR32_SPI_SR_TXEMPTY_MASK));//don't disable PDCA until last SPI transfer is done!
		pdca_disable(SPI1_RX_PDCA_CHANNEL);
	
		DDS_options_CH0.POW=t_currentsequence.CPMG.RFphase_POW;
		write_POW0(&DDS_options_CH0);	//change TX phase back to 90
		IO_UPDATE_local();
		set_profile_CH0_off(); //and turn RF back off
		
		while (!(SPI1->sr & AVR32_SPI_SR_TXEMPTY_MASK));//don't disable PDCA until last SPI transfer is done!
		
		//gpio_local_set_gpio_pin(GEN1_pin);	//debug signal
		handle_SPI_arbitration();
		//gpio_local_clr_gpio_pin(GEN1_pin);	//debug signal
	}	
	
	DAC0->dr0=0;
	
	DDS_options_CH0.POW=t_currentsequence.CPMG.LOphase_I_POW;
	write_POW0(&DDS_options_CH0);	//change TX phase to LO phase
	IO_UPDATE_local();
	set_profile_CH0_LOamp();		 //enable LOamp on CH0
	
	SLEEP(AVR32_PM_SMODE_IDLE);
	setnextevent(150);//delay for ~20us so that LO and baseband amp reach steady state
	temp=SPI1->rdr;	//read RDR and SR so that PDCA does not do a transfer when enabled
	temp=SPI1->sr;
	pdca_enable(SPI1_RX_PDCA_CHANNEL);
	for(uint32_t i=0;i<(t_experiment.Noffsetsamples+1);i++)  //now take samples of offsets.  Take one extra sample to get last conversion result
	{
		SLEEP(AVR32_PM_SMODE_IDLE);
		gpio_local_set_gpio_pin(ADC_CONV_pin);
		SPI1->tdr = (0xF0F0) | (ADC_PCS << AVR32_SPI_TDR_PCS_OFFSET) | (0 << AVR32_SPI_TDR_LASTXFER_OFFSET);
		gpio_local_clr_gpio_pin(ADC_CONV_pin);
		SPI1->tdr = (0xAFAF) | (ADC_PCS << AVR32_SPI_TDR_PCS_OFFSET) | (1 << AVR32_SPI_TDR_LASTXFER_OFFSET);
		setnextevent(t_experiment.t_dt_offsetsamples);
	}
	while (!(SPI1->sr & AVR32_SPI_SR_TXEMPTY_MASK));//don't disable PDCA until last SPI transfer is done!
	pdca_disable(SPI1_RX_PDCA_CHANNEL);
	gpio_local_clr_gpio_pin(RXSW_pin);			//RXSW OFF
	
	SLEEP(AVR32_PM_SMODE_IDLE);
	cpu_irq_disable();
	tc_stop(FAST_TC, FAST_TC_CHANNEL);
	read_TC_sr();
	
	pdca_disable(SPI1_RX_PDCA_CHANNEL);
	
	if((data_fail==false))
	{
		end_sequence_terminate();
	}
	else
	{//if there was a data failure
		handle_SD_failure();	//reinitialize SD card and write first cluster of the sequence with 0xFF
		SD_skip_clusters(t_currentsequence.clusters_per_sequence);	//skip to proper address of next sequence
	}
	shadow_SD_sink_ptr();	//shadow the SD_sink_ptr;
}

bool validate_sequences(void)		
{
	experiment_problem=no_problem;
	if((experiment.MODE<=MODE_max) &&
		(experiment.N_sequences>=Nsequences_min) && (experiment.N_sequences<=Nsequences_max) &&
		(experiment.Noffsetsamples>=1) && (experiment.Noffsetsamples<=Noffsetsamples_max) &&
		(experiment.dt_offsetsamples>=dt_offsetsamples_min) && (experiment.dt_offsetsamples<=dt_offsetsamples_max) &&
		(experiment.Vgain>=Vgain_min) && (experiment.Vgain<=Vgain_max))
	{
		t_experiment.MODE=experiment.MODE;
		t_experiment.Noffsetsamples=experiment.Noffsetsamples;
		t_experiment.t_dt_offsetsamples=((uint16_t)roundf((experiment.dt_offsetsamples * PBA_SPEED/(TC_DIV*1000000))));
		t_experiment.N_sequences=experiment.N_sequences;
		t_experiment.N_experiments=experiment.N_experiments;
		t_experiment.t_Vgain=(uint16_t)roundf(experiment.Vgain/Vref*4096);
	}
	else
	{
		return false;
	}
	
	if((experiment.TPArise>=TPArise_min) && (experiment.TPArise<=TPArise_max))
	{
		t_experiment.t_TPArise=((uint16_t)roundf((experiment.TPArise * PBA_SPEED/(TC_DIV*1000000))));
	}
	else
	{
		return false;
	}
	
	
	float temp_T;
	for(uint8_t i=0;i<experiment.N_sequences;i++)
	{
		currentsequence=experiment.sequence[i];
		
		if(experiment.MODE==MODE_standalone)
		{
			if((currentsequence.TR>=TR_min) && (currentsequence.TR<=TR_max))
			{
				t_currentsequence.t_TR=((uint16_t)roundf((currentsequence.TR *1000.0 )));	//need to correct this once slow TC is set up
			}
			else
			{
				return false;
			}
		}
		
		if(currentsequence.echofirst>1)	//in currentsequence, echofirst is a uint16, but actually represents a bool
		{
			return false;
		}
		{	//if echofirst is 0 or 1, then convert to a bool for t_currentsequence
			t_currentsequence.echofirst=(currentsequence.echofirst==1) ? true:false;
		}
		
		if((currentsequence.N_preppulses<1) || (currentsequence.N_preppulses>Npreppulses_max))
		{
			return false;
		}
		t_currentsequence.N_preppulses=currentsequence.N_preppulses;
		
		uint8_t j=0;
		while(j<(currentsequence.N_preppulses-1))	//check each prep pulse (except for last prep pulse before start of CPMG)
		{
			temp_T=currentsequence.preppulse[j].tau-(currentsequence.preppulse[j].pulsewidth/2)-(currentsequence.preppulse[j+1].pulsewidth/2)-experiment.TPArise;
			if( (currentsequence.preppulse[j].tau>=tau_min) && (currentsequence.preppulse[j].tau<=tau_max) &&
				(currentsequence.preppulse[j].pulsewidth>=pulsewidth_min) && (currentsequence.preppulse[j].pulsewidth<=pulsewidth_max) &&
				(currentsequence.preppulse[j].RFphase>=0) && (currentsequence.preppulse[j].RFphase<360) &&
				(currentsequence.preppulse[j].RFamp>0) && (currentsequence.preppulse[j].RFamp<=1023) &&
				(currentsequence.preppulse[j].Vb>=Vb_min) && (currentsequence.preppulse[j].Vb<=Vb_max) &&
				(temp_T>=T0_preppulse_min) && (temp_T<=T0_preppulse_max))
				{
					t_currentsequence.preppulse[j].ms_T0=0;
					while(temp_T>2000.0)
					{
						t_currentsequence.preppulse[j].ms_T0++;
						temp_T-=1000.0;
					}
					t_currentsequence.preppulse[j].t_T0=((uint16_t)roundf((temp_T * PBA_SPEED/(TC_DIV*1000000))));
					
					
					t_currentsequence.preppulse[j].ms_pulsewidth=0;
					temp_T=currentsequence.preppulse[j].pulsewidth;
					while(temp_T>2000.0)
					{
						t_currentsequence.preppulse[j].ms_pulsewidth++;
						temp_T-=1000.0;
					}
					t_currentsequence.preppulse[j].t_pulsewidth=((uint16_t)roundf((temp_T * PBA_SPEED/(TC_DIV*1000000))));
					
					
					t_currentsequence.preppulse[j].Vb=currentsequence.preppulse[j].Vb;
					t_currentsequence.preppulse[j].RFamp=currentsequence.preppulse[j].RFamp;
				
					t_currentsequence.preppulse[j].RFphase_POW=(uint16_t)roundf(16384*(currentsequence.preppulse[j].RFphase)/360) & 0x3FFF;
				}
				else
				{
					return false;
				}
				
				j++;
		}
		if(t_currentsequence.echofirst==false)
		{	//this is correct T0 when pulse is first
			temp_T=currentsequence.preppulse[j].tau-(currentsequence.preppulse[j].pulsewidth/2)-(currentsequence.CPMG.pulsewidth/2)-experiment.TPArise;	//check final prep pulse
		}
		else
		{	//this is correct T0 when echo is first
			temp_T=currentsequence.preppulse[j].tau-currentsequence.preppulse[j].pulsewidth/2.0-((float)(currentsequence.CPMG.Nsamp-1)*currentsequence.CPMG.dt)/2.0+currentsequence.CPMG.samp_offset;
		}
		
		if( (currentsequence.preppulse[j].tau>=tau_min) && (currentsequence.preppulse[j].tau<=tau_max) &&
				(currentsequence.preppulse[j].pulsewidth>=pulsewidth_min) && (currentsequence.preppulse[j].pulsewidth<=pulsewidth_max) &&
				(currentsequence.preppulse[j].RFphase>=0) && (currentsequence.preppulse[j].RFphase<360) &&
				(currentsequence.preppulse[j].RFamp>0) && (currentsequence.preppulse[j].RFamp<=1023) &&
				(currentsequence.preppulse[j].Vb>=Vb_min) && (currentsequence.preppulse[j].Vb<=Vb_max) &&
				(temp_T>=T0_preppulse_min) && (temp_T<=T0_preppulse_max))
				{
					t_currentsequence.preppulse[j].ms_T0=0;
					while(temp_T>2000.0)
					{
						t_currentsequence.preppulse[j].ms_T0++;
						temp_T-=1000.0;
					}
					t_currentsequence.preppulse[j].t_T0=((uint16_t)roundf((temp_T * PBA_SPEED/(TC_DIV*1000000))));
					
					
					t_currentsequence.preppulse[j].ms_pulsewidth=0;
					temp_T=currentsequence.preppulse[j].pulsewidth;
					while(temp_T>2000.0)
					{
						t_currentsequence.preppulse[j].ms_pulsewidth++;
						temp_T-=1000.0;
					}
					t_currentsequence.preppulse[j].t_pulsewidth=((uint16_t)roundf((temp_T * PBA_SPEED/(TC_DIV*1000000))));
					
					
					t_currentsequence.preppulse[j].Vb=currentsequence.preppulse[j].Vb;
					t_currentsequence.preppulse[j].RFamp=currentsequence.preppulse[j].RFamp;
					
					t_currentsequence.preppulse[j].RFphase_POW=(uint16_t)roundf(16384*(currentsequence.preppulse[j].RFphase)/360) & 0x3FFF;
				}
				else
				{
					return false;
				}
		
		if( (currentsequence.CPMG.tau>=TE_min/2) && (currentsequence.CPMG.tau<=TE_max/2) &&								//check individual CPMG parameters
			(currentsequence.CPMG.pulsewidth>=pulsewidth_min) && (currentsequence.CPMG.pulsewidth<=pulsewidth_max) &&
			(currentsequence.CPMG.dt>=dt_min) && (currentsequence.CPMG.dt<=dt_max) &&
			(currentsequence.CPMG.samp_offset>=samp_offset_min) && (currentsequence.CPMG.samp_offset<=samp_offset_max) &&
			(currentsequence.CPMG.Nsamp>=Nsamp_min) && (currentsequence.CPMG.Nsamp<=Nsamp_max) &&
			(currentsequence.CPMG.Nechos>=Nechos_min) && (currentsequence.CPMG.Nechos<=Nechos_max) &&
			(currentsequence.CPMG.RFphase>=0) && (currentsequence.CPMG.RFphase<360) &&
			(currentsequence.CPMG.RFamp>0) && (currentsequence.CPMG.RFamp<=1023) &&
			(currentsequence.CPMG.Vb>=Vb_min) && (currentsequence.CPMG.Vb<=Vb_max) &&
			(currentsequence.CPMG.LOphase>=0) && (currentsequence.CPMG.LOphase<360) &&
			(currentsequence.CPMG.LOamp>0) && (currentsequence.CPMG.LOamp<=1023) )
			{
				t_currentsequence.CPMG.Nsamp=currentsequence.CPMG.Nsamp;
				t_currentsequence.CPMG.Nechos=currentsequence.CPMG.Nechos;
				t_currentsequence.CPMG.RFamp=currentsequence.CPMG.RFamp;
				t_currentsequence.CPMG.Vb=currentsequence.CPMG.Vb;
				t_currentsequence.CPMG.LOamp=currentsequence.CPMG.LOamp;
				
				t_currentsequence.CPMG.t_tau=((uint16_t)roundf((currentsequence.CPMG.tau * PBA_SPEED/(TC_DIV*1000000))));
				t_currentsequence.CPMG.t_pulsewidth=((uint16_t)roundf((currentsequence.CPMG.pulsewidth * PBA_SPEED/(TC_DIV*1000000))));
				t_currentsequence.CPMG.t_dt=((uint16_t)roundf((currentsequence.CPMG.dt * PBA_SPEED/(TC_DIV*1000000))));
				t_currentsequence.CPMG.t_samp_offset=((int16_t)roundf((currentsequence.CPMG.samp_offset * PBA_SPEED/(TC_DIV*1000000))));
				t_currentsequence.CPMG.RFphase_POW=(uint16_t)roundf(16384*(currentsequence.CPMG.RFphase)/360) & 0x3FFF;
				t_currentsequence.CPMG.LOphase_Q_POW=(uint16_t)roundf(16384*(currentsequence.CPMG.LOphase)/360) & 0x3FFF;	
				if ((currentsequence.CPMG.LOphase)<270)
				{
					t_currentsequence.CPMG.LOphase_I_POW=((uint16_t)roundf(16384*((currentsequence.CPMG.LOphase)+90)/360)) & 0x3FFF;
				}
				else
				{
					t_currentsequence.CPMG.LOphase_I_POW=(uint16_t)roundf(16384*((currentsequence.CPMG.LOphase)-270)/360) & 0x3FFF;
				}
				
				}
			else
			{
				return false;
			}
				
			temp_T=currentsequence.CPMG.tau-currentsequence.CPMG.pulsewidth/2.0-((float)(currentsequence.CPMG.Nsamp-1)*currentsequence.CPMG.dt)/2.0+currentsequence.CPMG.samp_offset; //check value of T3
			if( (temp_T>=T3_min) && (temp_T<=T3_max))
			{
				t_currentsequence.CPMG.t_T3=t_currentsequence.CPMG.t_tau-t_currentsequence.CPMG.t_pulsewidth/2-((t_currentsequence.CPMG.Nsamp-1)*t_currentsequence.CPMG.t_dt)/2+t_currentsequence.CPMG.t_samp_offset;
			}
			else
			{
				experiment_problem=T3_bad;
				return false;
			}
				
			temp_T=2*currentsequence.CPMG.tau-temp_T-((float)(currentsequence.CPMG.Nsamp-1)*currentsequence.CPMG.dt)-currentsequence.CPMG.pulsewidth-experiment.TPArise; //check value of T5
			if( (temp_T>=T5_min) && (temp_T<=T5_max))
			{
				t_currentsequence.CPMG.t_T5=2*t_currentsequence.CPMG.t_tau-t_currentsequence.CPMG.t_T3-((t_currentsequence.CPMG.Nsamp-1)*t_currentsequence.CPMG.t_dt)-t_currentsequence.CPMG.t_pulsewidth-t_experiment.t_TPArise;
			}
			else
			{
				experiment_problem=T5_bad;
				return false;
			}
				
				//compute data rate in MB/s
			float writerate=((float)currentsequence.CPMG.Nsamp)/(2.0*currentsequence.CPMG.tau)*4.0;
			if(writerate>writerate_max)
			{
				experiment_problem=writerate_high;;
				return false;
				
			}
				
			//calculate time of full sequence
			//In host mode, this should also make sure there is enough time to communicate data to host in between sequences
			
			temp_T=currentsequence.CPMG.tau*2.0*((float)currentsequence.CPMG.Nechos);				
			for(j=0;j<currentsequence.N_preppulses;j++)
			{
				temp_T+=currentsequence.preppulse[j].tau;
			}
		
			if(temp_T+100000.0>=currentsequence.TR*1000000.0)		//check that TR is tenth of a second shorter than TR
			{
				experiment_problem=TR_Tcpmg_short;
				return false;
			}
			
			
			
			if( (currentsequence.f0>=f0_min) && (currentsequence.f0<=f0_max) )
			{
				t_currentsequence.f0_FTW=(uint32_t)roundf(4294967296.0*currentsequence.f0/DDS_Fs);
			}	
			else
			{
				return false;
			}			
				//calculate number of block clusters per sequence
			t_currentsequence.clusters_per_sequence=ceilf((currentsequence.CPMG.Nsamp*currentsequence.CPMG.Nechos+t_experiment.Noffsetsamples+1)*4/((float)samplebuffer_size));
			
		t_experiment.t_sequence[i]=t_currentsequence; //save sequence parameters
	}			
		return true;	//return true if sequence passed
}

void gettruesequence(void)  //does the opposite:  looks at realized parameters in t_experiment, convert them back to floats, and store the results in experiment
{
	experiment.MODE=t_experiment.MODE;
	experiment.N_sequences=t_experiment.N_sequences;
	experiment.Noffsetsamples=t_experiment.Noffsetsamples;
	experiment.dt_offsetsamples=t_experiment.t_dt_offsetsamples;
	experiment.Vgain=(float)t_experiment.t_Vgain/4096*Vref;
	experiment.TPArise=(float)t_experiment.t_TPArise*TC_DIV*1000000.0/PBA_SPEED;
	for(uint8_t i=0;i<Nsequences_max;i++)
	{
		t_currentsequence=t_experiment.t_sequence[i];
		
		if(t_experiment.MODE==MODE_standalone)
		{
			currentsequence.TR=(float)(t_currentsequence.t_TR)/1000.0;
		}
		
		currentsequence.N_preppulses=t_currentsequence.N_preppulses;
		
		uint8_t j=0;
		while(j<(Npreppulses_max-1))
		{
			currentsequence.preppulse[j].Vb=t_currentsequence.preppulse[j].Vb;
			currentsequence.preppulse[j].RFamp=t_currentsequence.preppulse[j].RFamp;			
			currentsequence.preppulse[j].pulsewidth=(float)t_currentsequence.preppulse[j].t_pulsewidth*(float)TC_DIV*1000000.0/((float)PBA_SPEED);
			currentsequence.preppulse[j].tau=((float)t_currentsequence.preppulse[j].t_pulsewidth/2.0 + (float)t_currentsequence.preppulse[j].t_T0 + (float)t_experiment.t_TPArise + (float)t_currentsequence.preppulse[j+1].t_pulsewidth/2.0)*(float)TC_DIV*1000000.0/((float)PBA_SPEED);
			currentsequence.preppulse[j].RFphase=(float)t_currentsequence.preppulse[j].RFphase_POW*360.0/16384.0;
			
			j++;
		}
		currentsequence.preppulse[j].Vb=t_currentsequence.preppulse[j].Vb;
		currentsequence.preppulse[j].RFamp=t_currentsequence.preppulse[j].RFamp;			
		currentsequence.preppulse[j].pulsewidth=(float)t_currentsequence.preppulse[j].t_pulsewidth*(float)TC_DIV*1000000.0/((float)PBA_SPEED);
		currentsequence.preppulse[j].tau=((float)t_currentsequence.preppulse[j].t_pulsewidth/2.0 + (float)t_currentsequence.preppulse[j].t_T0 + (float)t_experiment.t_TPArise + (float)t_currentsequence.CPMG.t_pulsewidth/2.0)*(float)TC_DIV*1000000.0/((float)PBA_SPEED);
		currentsequence.preppulse[j].RFphase=(float)t_currentsequence.preppulse[j].RFphase_POW*360.0/16384.0;
		
		currentsequence.CPMG.Nsamp=t_currentsequence.CPMG.Nsamp;
		currentsequence.CPMG.Nechos=t_currentsequence.CPMG.Nechos;
		currentsequence.CPMG.RFamp=t_currentsequence.CPMG.RFamp;
		currentsequence.CPMG.Vb=t_currentsequence.CPMG.Vb;
		currentsequence.CPMG.LOamp=t_currentsequence.CPMG.LOamp;
				
		currentsequence.CPMG.tau=((float)t_currentsequence.CPMG.t_pulsewidth+(float)t_currentsequence.CPMG.t_T3+((float)t_currentsequence.CPMG.t_dt*(float)(t_currentsequence.CPMG.Nsamp-1))+(float)t_currentsequence.CPMG.t_T5+(float)t_experiment.t_TPArise)*(float)TC_DIV*1000000.0/((float)PBA_SPEED*2.0);;
		currentsequence.CPMG.pulsewidth=(float)t_currentsequence.CPMG.t_pulsewidth*(float)TC_DIV*1000000.0/((float)PBA_SPEED);
		currentsequence.CPMG.dt=(float)t_currentsequence.CPMG.t_dt*(float)TC_DIV*1000000.0/((float)PBA_SPEED);
		currentsequence.CPMG.samp_offset=(float)t_currentsequence.CPMG.t_samp_offset*(float)TC_DIV*1000000.0/((float)PBA_SPEED);
		currentsequence.CPMG.RFphase=(float)t_currentsequence.CPMG.RFphase_POW*360.0/16384.0;
		currentsequence.CPMG.LOphase=(float)t_currentsequence.CPMG.LOphase_Q_POW*360.0/16384.0;
		
		currentsequence.f0=(float)t_currentsequence.f0_FTW/4294967296.0*(float)DDS_Fs;
		
		experiment.sequence[i]=currentsequence;
	}
}

uint8_t get_experiment_problem(void)
{
	return experiment_problem;
}