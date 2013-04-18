function [echovalues] = evaluate_echoes_f_real(CPXechoes,bandwidth,FTsize)
%this function takes the CPXechoes data and evaluates the real "amplitude" of
%the echoes.  This function assumes that CPXechoes has four non-singleton
%dimensions, and the returned echovalues will be three dimensional.

%FTsize is the desired size of the echoes after padding
%bandwidth is the bandwidth across which to average the echo to get the
%amplitude

global true_experiment;
global experiment;

%first find out what the max number of echoes and samples are for the data
%set
most_echoes=0;
most_samps=0;
most_Nsamp=0;
        for experimentcount=1:true_experiment.Nexperiments
            for sequencecount=1:true_experiment.Nsequences
                if uint32(true_experiment.sequence(sequencecount).cpmg.Nechos)*uint32(true_experiment.sequence(sequencecount).cpmg.Nsamp)>most_samps;
                most_samps=uint32(true_experiment.sequence(sequencecount).cpmg.Nechos)*uint32(true_experiment.sequence(sequencecount).cpmg.Nsamp);
                end
                if uint32(true_experiment.sequence(sequencecount).cpmg.Nsamp)>most_Nsamp;
                most_Nsamp=uint32(true_experiment.sequence(sequencecount).cpmg.Nsamp);
                end
                if uint32(true_experiment.sequence(sequencecount).cpmg.Nechos)>most_echoes;
                most_echoes=uint32(true_experiment.sequence(sequencecount).cpmg.Nechos);
                end
            end
        end

%create echovalues according to most echoes in all sequences
echovalues=zeros(true_experiment.Nexperiments,true_experiment.Nsequences,most_echoes);

%temporary echo with padded size
padded_echo_f_temp=zeros(1,FTsize);

for i=1:experiment.Nexperiments
    for j=1:experiment.Nsequences
        for k=1:true_experiment.sequence(j).cpmg.Nechos
            padded_echo_f_temp=fft(fftshift(padarray(squeeze(CPXechoes(i,j,k,:)),((FTsize-double(true_experiment.sequence(j).cpmg.Nsamp))/2))));
            df=1000000/(FTsize*true_experiment.sequence(j).cpmg.dt); %get frequency resolution
            samps=round(bandwidth/df/2);
            echovalues(i,j,k)=mean(real(padded_echo_f_temp([1:samps FTsize-samps:FTsize])));
        end
    end
end

end

