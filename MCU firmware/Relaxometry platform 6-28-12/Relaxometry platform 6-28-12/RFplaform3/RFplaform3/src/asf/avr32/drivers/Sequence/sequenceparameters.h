

typedef struct { //36 bytes total
	float		tau;			//half of TE in CPMG										///all time parameters are in TC counts, not us
	float		pulsewidth;			//duration of excitation pulses
	float		RFphase;
	uint16_t	RFamp;			//amplitude of excitiation pulses
	uint16_t	Vb;
	float		LOphase;		//phase of LO
	uint16_t	LOamp;			//amplitude of LO
	uint16_t	Nsamp;			//number of samples per echo
	uint32_t	Nechos;			//number of echos in one sequence
	float		dt;				//sampling period
	float		samp_offset;	//offset of center of sampling routine from predicted echo center
} CPMG_t;

typedef struct {									///all time parameters are in TC counts, not us
	uint16_t	t_tau;
	uint16_t	t_pulsewidth;
	uint16_t	RFphase_POW;
	uint16_t	RFamp;			//amplitude of excitiation pulses
	uint16_t	Vb;
	uint16_t	LOphase_Q_POW;
	uint16_t	LOphase_I_POW;
	uint16_t	LOamp;			//amplitude of LO
	uint16_t	Nechos;			//number of echos in one sequence
	uint16_t	Nsamp;			//number of samples per echo
	uint16_t	t_dt;
	int16_t		t_samp_offset;	//shouldn't this be a signed int?
	uint16_t	t_T3;
	uint16_t	t_T5;
} t_CPMG_t;

typedef struct {	//16 bytes
	float		tau;			//half of TE in CPMG										///all time parameters are in TC counts, not us
	float		pulsewidth;			//duration of excitation pulses
	float		RFphase;
	uint16_t	RFamp;			//amplitude of excitiation pulses
	uint16_t	Vb;
} preppulse_t;

typedef struct {									///all time parameters are in TC counts, not us
	uint16_t	t_T0;
	uint16_t	ms_T0;
	uint16_t	t_pulsewidth;
	uint16_t	ms_pulsewidth;
	uint16_t	RFphase_POW;
	uint16_t	RFamp;			//amplitude of excitiation pulses
	uint16_t	Vb;
} t_preppulse_t;

typedef struct { //8 bytes, +36, +16*4 = 108
	float			f0;
	preppulse_t		preppulse[4];
	CPMG_t			CPMG;
	uint16_t			N_preppulses;
	uint16_t			echofirst;
	float			TR;
} sequence_t;

typedef struct {
	uint32_t		f0_FTW;
	t_preppulse_t	preppulse[4];
	t_CPMG_t			CPMG;
	uint16_t			N_preppulses;
	bool			echofirst;
	uint16_t		clusters_per_sequence;
	uint32_t		t_TR;
} t_sequence_t;

typedef struct {	//12 bytes, +103*8
	uint16_t		MODE;
	uint16_t		N_sequences;
	sequence_t		sequence[8];
	float			TPArise;
	uint32_t		Noffsetsamples;
	float			dt_offsetsamples;
	uint32_t		N_experiments;
	float			Vgain;
	
} experiment_t;

typedef struct {
	uint8_t			MODE;
	uint8_t			N_sequences;
	t_sequence_t	t_sequence[8];
	uint16_t		t_TPArise;
	uint32_t		Noffsetsamples;
	uint16_t		t_dt_offsetsamples;
	uint32_t		N_experiments;
	uint16_t		t_Vgain;
} t_experiment_t;





#define samplebuffer_size 16384

#define slow_TC_DIV 1000
#define TC_DIV 8
#define sequenceID_CPMG 0
#define f0_min 1000000
#define f0_max 80000000
#define tau3_min 25
#define tau3_max 65536.0/PBA_SPEED*TC_DIV*1000000.0
#define tau1_min 15
#define tau1_max 65536.0/PBA_SPEED*TC_DIV*1000000.0
#define tau2_min 15
#define tau2_max 65536.0/PBA_SPEED*TC_DIV*1000000.0
#define TI_min 15
#define TI_max 65536.0/PBA_SPEED*TC_DIV*1000000.0
#define pw90_min 1.5
#define pw90_max 65536.0/PBA_SPEED*TC_DIV*1000000.0
#define pw180_min 1.5
#define pw180_max 65536.0/PBA_SPEED*TC_DIV*1000000.0
#define Nechos_min 1
#define Nechos_max 25000
#define LOphase_min 0
#define LOphase_max 360
#define TPArise_min 2.0
#define TPArise_max 65536.0/PBA_SPEED*TC_DIV*1000000.0
#define Nsamp_min 1
#define Nsamp_max 128
#define dt_min 1.6
#define dt_max 65536.0/PBA_SPEED*TC_DIV*1000000.0
#define samp_offset_min -100
#define samp_offset_max 100

#define Noffsetsamples_max	1000
#define dt_offsetsamples_min 1.6
#define dt_offsetsamples_max 1000.0

#define Nsequences_min	1
#define Nsequences_max	8
#define Npreppulses_max 4

#define TR_min 1.0
#define TR_max 60.0

#define T3_min 15.5
#define T3_max 8000.0
#define T5_min 7.5
#define T5_max 8000.0

#define MODE_max 1
#define MODE_hosted 0
#define MODE_standalone 1

#define Vgain_min 0.2
#define Vgain_max 2.499
#define Vref 2.5

sequence_t currentsequence;

experiment_t experiment;

t_sequence_t t_currentsequence;
t_experiment_t t_experiment;

#define Vb_min 1
#define Vb_max 4095
#define pulsewidth_min 1.5
#define pulsewidth_max 500000.0
#define tau_min 20.0
#define tau_max 1000000.0
#define TE_min 60.0
#define TE_max 16000.0
#define T0_min 15.5
#define T0_max 8000.0
#define T0_preppulse_min 10.0
#define T0_preppulse_max 500000.0
#define writerate_max 1.0
