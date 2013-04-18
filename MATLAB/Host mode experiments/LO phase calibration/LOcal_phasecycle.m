%this script is designed to determine the correct LO phase in order to
%shift the NMR echo into the real axis.  It does several simple CPMG
%experiments, finds the complex resulting complex magnetization, and
%calculated the phase offset needed to achieve the proper LO.  Phase
%cycling is used.

global experiment;  %define globals
global true_experiment;
declare_experiment;
global s;

define_experiment_example; %define sequence parameters, this is a function you create

experiment.Nexperiments=uint32(16);  %define how many times to repeat experiment

BW=50000;  %bandwidth of amplitude measurement, should be equal to RF pulse bandwidth
Nft=100;    %how much to pad echoes

experiment.sequence(1).preppulse(1).pulsephase=single(0);
CPXechoes_pos=run_host_mode;  %perform experiment with RF phase of 0

experiment.sequence(1).preppulse(1).pulsephase=single(0);
CPXechoes_neg=run_host_mode; %perform experiment again with RF phase of 180

CPXechoes_sum=CPXechoes_pos-CPXechoes_neg;  %take difference of echoes

%get real and imaginary echo values
CPXechoes_sum_mean=squeeze(mean(CPXechoes_sum,1)); %average all experiments
echovalues_f_real_mean=evaluate_echoes_f_real_mean(CPXechoes_sum_mean,BW,Nft);  %calculate complex echo values
echovalues_f_imag_mean=evaluate_echoes_f_imag_mean(CPXechoes_sum_mean,BW,Nft);

%calculate time axis
time=[true_experiment.sequence(1).cpmg.tau*2:true_experiment.sequence(1).cpmg.tau*2:true_experiment.sequence(1).cpmg.tau*2*double(true_experiment.sequence(1).cpmg.Nechos)]./1000000;


%do exponential fit of complex echo values
%should estimate approximate T2 and offset for best results.
%amplitude of first echo will be automatically used to guess intial
%magnetization

guessT2=0.001;
guessoffset=-100;

figure(1)
plot(time,echovalues_f_real_mean,time,echovalues_f_imag_mean)
figure(2);
fitparameters=fminsearch('fitt2offset',[guessoffset echovalues_f_real_mean(1,1)-guessoffset guessT2],[],time,echovalues_f_real_mean(1,:));
CPX_M0=fitparameters(2);
fitparameters=fminsearch('fitt2offset',[guessoffset echovalues_f_imag_mean(1,1)-guessoffset guessT2],[],time,echovalues_f_imag_mean(1,:));
%CPX_M0 is fit complex magnetization
CPX_M0=CPX_M0+i*fitparameters(2);
    
%calculate phase of magnetization
if real(CPX_M0)>=0
    phase=atand(imag(CPX_M0)/real(CPX_M0));
else
    phase=180-atand(imag(CPX_M0)/-real(CPX_M0));
end

%calculate new LO phase
newphase=true_experiment.sequence(1).cpmg.LOphase-phase;
if newphase>=360
    newphase=newphase-360
elseif newphase<0
    newphase=newphase+360
else
    newphase=newphase
end

% eval(['save data_saved experiment true_experiment CPXechoes_sum echovalues_f_real_mean BW'])