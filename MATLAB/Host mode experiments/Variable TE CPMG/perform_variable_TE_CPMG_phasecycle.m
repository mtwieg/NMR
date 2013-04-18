% This script performs a variable TE CPMG experiment, with phase cycling
% Originally used for measuring the ADC and T2 of milk samples

global experiment;  %declare globals
global true_experiment;
global s;
declare_experiment;

define_experiment_example; %define sequence parameters, this is a function you create

experiment.Nexperiments=single(4); %define how many times to repeat experiment
Nseq=8; %number of different TEs in experiment
taumin=50;  %minimum tau1 and tau2
taumax=300; %maximum tau1 and tau2

BW=100000;  %bandwidth of amplitude measurement, should be equal to RF pulse bandwidth

experiment.Nsequences=uint16(Nseq);

if(Nseq>1)  %choose series of tau1 and tau2 based on previously specified parameters
    taulist=((taumin^2:(taumax^2-taumin^2)/(Nseq-1):taumax^2).^0.5);
else
    taulist=taumin;
end

Tcpmg=0.75; %total duration of CPMG, in seconds
Nechoeslist=(round(Tcpmg*1000000./(2*taulist)));

for k=1:Nseq  %set experiment parameters based on defined parameters
    experiment.sequence(k).preppulse(1).tau=single(taulist(k));
    experiment.sequence(k).cpmg.tau=single(taulist(k));
    experiment.sequence(k).cpmg.Nechos=uint32(Nechoeslist(k));
    experiment.sequence(k).preppulse(1).pulsephase=single(0);   %set excitation phase to 0
end

CPXechoes_pos=run_host_mode;  %perform experiment once with RF phase of 0

for k=1:Nseq
    experiment.sequence(k).preppulse(1).pulsephase=single(180);
end

CPXechoes_neg=run_host_mode;  %perform experiment again with RF phase of 180

%get mean real echo values
CPXechoes_sum=CPXechoes_pos-CPXechoes_neg; %take difference of echoes
CPXechoes_sum_mean=squeeze(mean(CPXechoes_sum,1)); %average all experiments
echovalues_f_mean=squeeze(evaluate_echoes_f_real_mean(CPXechoes_sum_mean,BW,100)); %calculate real echo values

%create variables to hold fitting results
magnitude_series=zeros(1,Nseq);
offset_series=zeros(1,Nseq);
T2a_series=zeros(1,Nseq);

%do exponential fit of real echo values
%should estimate approximate T2 and offset for best results.
%amplitude of first echo will be automatically used to guess intial
%magnetization

guessT2=[0 0 0 0 0 0 0 0]; %because this is a variable TE CPMG, T2 will change every experiment
guessoffset=25;

%do fitting using three parameter exponential fit
for k=1:Nseq
    Nechoes=true_experiment.sequence(k).cpmg.Nechos;
    tau=true_experiment.sequence(k).cpmg.tau;
    time=[double(tau)*2/1000000:double(tau)*2/1000000:(double(tau)*2/1000000)*double(Nechoes)]; %calculate time axis for each sequence
    figure(k)
    plot(time,echovalues_f_mean(k,1:Nechoes))
    fitparameters=fminsearch('fitt2offset',[guessoffset echovalues_f_mean(k,1)-guessoffset guessT2(k)],[],time,echovalues_f_mean(k,1:Nechoes));
    magnitude_series(k)=fitparameters(2);
    T2a_series(k)=fitparameters(3);
    offset_series(k)=fitparameters(1);  
end

guessADC=4e9; %ADC returned value of fitD_variable_TE
guessT2=0.2;  %guessed T2, in seconds

k=k+1;
figure(k)
fitT2_ADC=fminsearch('fitD_variable_TE',[guessT2 guessADC],[],double(taulist(:))./1000000,double(T2a_series(:)));
ADC=fitT2_ADC(2);

%calculate D (or G) and T2 based on assumed parameters
gyro=267.5e6;
G=6.8;
D=ADC/(1/3*G^2*gyro^2)
T2=fitT2ADC2(1)
% D=2.299e-9
% G=sqrt(ADC/(1/3*D*gyro^2))

% eval(['save data_Dmeas_example experiment true_experiment CPXechoes_pos CPXechoes_neg taulist Tcpmg Nseq echovalues_mean T2a_series magnitude_series offset_series ADC T2 BW G'])
