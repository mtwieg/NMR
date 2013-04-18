function [] = declare_experiment(  )
%this function sets up the structure of experiment, with initial values
%Before running this script, one should declare a global variable named
%experiment

preppulse.tau=single(50.222);   %define basic preppulse structure
preppulse.pulsewidth=single(6.3);
preppulse.pulsephase=single(0);
preppulse.pulseamp=uint16(512);
preppulse.Vb=uint16(100);

cpmg.tau=single(51);            %define basic CPMG structure
cpmg.pulsewidth=single(10.02);
cpmg.pulsephase=single(90.87);
cpmg.pulseamp=uint16(512);
cpmg.Vb=uint16(100);
cpmg.LOphase=single(15);
cpmg.LOamp=uint16(1023);
cpmg.Nsamp=uint16(10);
cpmg.Nechos=uint32(500);
cpmg.dt=single(1.6);
cpmg.samp_offset=single(-3);

sequence.f0=single(8350001.1);  %define basic sequence structure
sequence.preppulse(1:4)=preppulse;
sequence.cpmg=cpmg;
sequence.Npreppulses=uint16(1);
sequence.echofirst=uint16(1);
sequence.TR=single(3);

experiment_t.MODE=uint16(1);    %define basic experiment_t structure
experiment_t.Nsequences=uint16(4);
experiment_t.sequence(1:8)=sequence;
% experiment_t.TR=single(2);
experiment_t.TPArise=single(10.54);
experiment_t.Noffsetsamples=uint32(100);
experiment_t.dt_offsetsamples=single(10);
experiment_t.Nexperiments=uint32(10);
experiment_t.Vgain=single(0.5);
global experiment;
experiment=experiment_t;        %copy to experiment

end

