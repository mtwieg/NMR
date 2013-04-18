% function [ CPXechoes ] = get_echoes_from_SD()
%this script gets the raw data from the platform's SD card, then converts
%it to the CPXechoes format
global experiment;
global true_experiment;
global clusters_per_sequence;
clusters_per_sequence=zeros(1,8);
rawdata=get_raw_data_from_SD; %first data is just in blocks of clusters, unsigned 16 bit integers
CPXechoes=zeros(true_experiment.Nexperiments,true_experiment.Nsequences,true_experiment.sequence(1).cpmg.Nechos,true_experiment.sequence(1).cpmg.Nsamp,'single');

for i=1:true_experiment.Nexperiments
    for j=1:true_experiment.Nsequences
        rawsequence=squeeze(rawdata(i,j,(1:double(uint32(true_experiment.sequence(j).cpmg.Nsamp)*uint32(true_experiment.sequence(j).cpmg.Nechos)+1)*2)));
        i
                    rawsequence=bitand((2^14-1),swapbytes(uint16(rawsequence)));
                    rawsequence=uint16(rawsequence*4);
                    rawsequence=typecast(rawsequence,'int16');
                    rawsequence=(rawsequence)/4;
                    rawsequence=double(rawsequence);
                    % ******************
                    rawoffset=squeeze(rawdata((double(uint32(true_experiment.sequence(j).cpmg.Nsamp)*uint32(true_experiment.sequence(j).cpmg.Nechos)+1)*2)+1:double(uint32(true_experiment.sequence(j).cpmg.Nsamp)*uint32(true_experiment.sequence(j).cpmg.Nechos)+uint32(true_experiment.Noffsetsamples)+1)*2));

                    rawoffset=bitand((2^14-1),swapbytes(uint16(rawoffset)));
                    rawoffset=uint16(rawoffset*4);
                    rawoffset=typecast(rawoffset,'int16');
                    rawoffset=(rawoffset)/4;
                    rawoffset=double(rawoffset);

                    CPXoffset=zeros(1,length(rawoffset)/2);
                    for k=1:length(rawoffset)/2 
                        CPXoffset(1,k)=rawoffset(2*(k)-1)+1i*rawoffset(2*(k));
                    end

                    CPX=zeros(1,length(rawsequence)/2-1);
                    for k=1:length(CPX)
                        CPX(1,k)=rawsequence(2*(k)+1)+1i*rawsequence(2*(k)+2);
                    end

                    CPX_stream=zeros(1,length(rawsequence)/2-1);
                    CPX_stream(1:length(CPX))=CPX-mean(CPXoffset);

                    for k=1:true_experiment.sequence(j).cpmg.Nechos
                        CPXechoes(i,j,k,:)=single(CPX_stream((double(true_experiment.sequence(1).cpmg.Nsamp)*(k-1)+1):(double(true_experiment.sequence(1).cpmg.Nsamp)*k)));
                    end
    end
end
    clear rawdata;
    clear rawsequence;
    clear CPX_stream;
    clear rawoffset;
    clear CPX;
    clear CPXoffset;
    clear clusters_per_sequence;

