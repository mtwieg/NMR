function [echovalues] = evaluate_echoes_f_imag_mean(CPXechoes,bandwidth,FTsize)
%this function takes the CPXechoes data and evaluates the imaginary "amplitude" of
%the echoes.  This function assumes that CPXechoes has either two (if it's
%just one sequence, with experiments averaged) or three (multiple
%sequences, averaged experiments) dimensions with non-singleton values.  

%FTsize is the desired size of the echoes after padding
%bandwidth is the bandwidth across which to average the echo to get the
%amplitude

global true_experiment;
global experiment;

%get dimensions of CPXechoes
sizematrix=size(CPXechoes);
true_Nseq=sizematrix(1);

%find out what the max number of echoes and samples are for the data
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

%temporary echo with padded size
padded_echo_f_temp=zeros(1,FTsize);

%do analysis differently depending on size of CPXechoes
if length(sizematrix)>2
    echovalues=zeros(true_Nseq,most_echoes);    %CPXechoes had 3 dimensions, so echovalues will have two
    for j=1:true_Nseq
        for k=1:true_experiment.sequence(j).cpmg.Nechos
            padded_echo_f_temp=fft(fftshift(padarray(squeeze(CPXechoes(j,k,:)),((FTsize-double(true_experiment.sequence(j).cpmg.Nsamp))/2))));
            df=1000000/(FTsize*true_experiment.sequence(j).cpmg.dt); %get frequency resolution
            samps=round(bandwidth/df/2);
            echovalues(j,k)=mean(imag(padded_echo_f_temp([1:samps FTsize-samps:FTsize])));
        end
    end 
else
    echovalues=zeros(1,most_echoes);    %CPXechoes had 2 dimensions, so echovalues will have one
    for k=1:true_experiment.sequence(1).cpmg.Nechos
            padded_echo_f_temp=fft(fftshift(padarray(squeeze(CPXechoes(k,:).'),((FTsize-double(true_experiment.sequence(1).cpmg.Nsamp))/2))));
            df=1000000/(FTsize*true_experiment.sequence(1).cpmg.dt); %get frequency resolution
            samps=round(bandwidth/df/2);
            echovalues(k)=mean(imag(padded_echo_f_temp([1:samps FTsize-samps:FTsize])));
    end
end


end

