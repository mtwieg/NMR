%this script performs the constant relaxation self diffusion measurement in
%order to measure T2 or D.  Phase cycling is used

global experiment;  %declare globals
global true_experiment;
declare_experiment;

define_experiment_example; %define sequence parameters, this is a function you create

experiment.Nexperiments=uint32(2);  %number of experiment repetitions

Nseq=8; %number of different prep periods to use in experiment
Vb_90=1300; %excitation RF amplitude
Vb_180=2600; %refocusing RF amplitude
tau=100;    %tau of CPMG
Tcpmg=0.3;  %total duration of CPMG (in seconds)

tau_min=150;    %minimum tau of prep period. Make sure it's large enough to for sequence to not cause errors
tau_total=3000; %fixed value of tau1+tau2
tau1min=tau_total/2+tau_min;
tau1max=tau_total-tau_min;
T3min=tau1min^3+(tau_total-tau1min)^3;
T3max=tau1max^3+(tau_total-tau1max)^3;
T3step=(T3max-T3min)/(Nseq-1);
T3listwanted=(T3min:T3step:T3max);
tau1list=zeros(1,Nseq);
for i=1:Nseq
tau1list(i)=((3*tau_total^2+(9*tau_total^4-4*3*tau_total*(tau_total^3-T3listwanted(i)))^0.5)/(6*tau_total));
end
tau1list;
tau2list=tau_total-tau1list;
clear tau1min tau1max T3min T3max T3step;

BW=50000;   %bandwidth over which to do echo amplitude averaging

%change experiment based on selected parameters
experiment.Nsequences=uint16(Nseq);
Nechoes=(round(Tcpmg*1000000./(2*tau)));
for k=1:Nseq
    experiment.sequence(k).preppulse(1).tau=single(tau1list(k));
    experiment.sequence(k).preppulse(1).Vb=uint16(Vb_90);
    experiment.sequence(k).preppulse(1).pulsephase=single(0);
    
    experiment.sequence(k).preppulse(2).tau=single(tau1list(k)+tau2list(k));
    experiment.sequence(k).preppulse(2).Vb=uint16(Vb_180);
    experiment.sequence(k).preppulse(2).pulsephase=single(90);
    
    experiment.sequence(k).preppulse(3).tau=single(tau2list(k));
    experiment.sequence(k).preppulse(3).Vb=uint16(Vb_180);
    experiment.sequence(k).preppulse(3).pulsephase=single(90);
    
    experiment.sequence(k).Npreppulses=uint16(3);
    experiment.sequence(k).cpmg.tau=single(tau);
    experiment.sequence(k).cpmg.Nechos=uint32(Nechoes);
    experiment.sequence(k).cpmg.Vb=uint16(Vb_180);
    experiment.sequence(k).echofirst=uint16(1);
end
%run experiment with RF phase at 0
CPXechoes_pos=run_host_mode;

for k=1:Nseq
    experiment.sequence(k).preppulse(1).pulsephase=single(180);
end
%run experiment with RF phase at 180
CPXechoes_neg=run_host_mode;

%get mean real echo values
CPXechoes_sum=CPXechoes_pos-CPXechoes_neg; %take difference of echoes
CPXechoes_sum_mean=squeeze(mean(CPXechoes_sum,1)); %average all experiments
echovalues_f_mean=squeeze(evaluate_echoes_f_real_mean(CPXechoes_sum_mean,BW,100)); %calculate real echo values

%calculate time axis
Nechoes=true_experiment.sequence(k).cpmg.Nechos;
time=[double(tau)*2/1000000:double(tau)*2/1000000:(double(tau)*2/1000000)*double(Nechoes)];

%create variables to hold fitting results
magnitude_series=zeros(1,Nseq);
offset_series=zeros(1,Nseq);
T2a_series=zeros(1,Nseq);

%do exponential fit of real echo values
%should estimate approximate T2 and offset for best results.
%amplitude of first echo will be automatically used to guess intial
%magnetization

guessT2=0.1;
guessoffset=25;

for k=1:Nseq
    tau=true_experiment.sequence(k).cpmg.tau;
    figure(k)
    plot(time,echovalues_mean(k,1:Nechoes))
    fitparameters=fminsearch('fitt2offset',[guessoffset echovalues_f_mean(k,2)-guessoffset guessT2],[],time,echovalues_f_mean(k,1:Nechoes));
    magnitude_series(k)=fitparameters(2);
    T2a_series(k)=fitparameters(3);
    offset_series(k)=fitparameters(1); 
end
logmag_series=log(magnitude_series);
TE3listfit=(tau1list/1000000).^3+(tau2list/1000000).^3;

guessADC=4e6; %ADC returned value of fitD_constant_relax

Dparameters=fminsearch('fitD_constant_relax',[double(logmag_series(1)) -guessADC],[],TE3listfit(1:Nseq),logmag_series(1:Nseq));

%calculate D or G based on known parameters
gyro=267.5e6;
G=6.47;
D=Dparameters(2)/(-2/3*gyro^2*G^2)
T2=Dparameters(1)/(-2*tau_total)    %T2 value is rarely accurate...
% D=2.299e-9;
% G=sqrt(Dparameters(2)/(-2/3*gyro^2*D))

% eval(['save data_Dmeas_constantT2_saved experiment true_experiment CPXechoes_pos CPXechoes_neg tau1list tau2list tau_total Tcpmg Nseq echovalues_mean T2a_series magnitude_series offset_series TE3listfit logmag_series Dparameters T2 G'])