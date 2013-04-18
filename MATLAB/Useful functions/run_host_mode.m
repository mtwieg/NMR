function [CPXechoes] = run_host_mode()
%this runs one repetition of the experiment, in host mode
%returns data in CPXechoes format
 
failure_cause=0;

priorPorts = instrfind;  % finds any existing Serial Ports in MATLAB
delete(priorPorts);      % and deletes them 
s = serial('COM13', 'BaudRate', 3000000)    %declare and open serial port
set(s,'InputBufferSize',16384,'Outputbuffersize',20000,'Timeout',5,'FlowControl','hardware');
fopen(s)
s.Bytesavailable 
while(s.Bytesavailable>0)
    s.Bytesavailable
    extradata=fread(s,s.Bytesavailable,'uint8');
end


pause(0.05) 
s.Bytesavailable  %flush serial buffer, multiple times
while(s.Bytesavailable>0)
    s.Bytesavailable
    extradata=fread(s,s.Bytesavailable,'uint8');
    pause(0.05);
end

global experiment;
global true_experiment;

fprintf(s,'RESTART');   %command platform to restart
pause(0.1);

clusters_per_sequence=zeros(1,8);
fprintf(s,'HOSTST');    %tell platform host mode is starting, check SD card
reply=fgetl(s)


if strcmp(reply,'good SD card') %check if SD card was good
    structarray=struct_2_bytes(experiment);
    fwrite(s,structarray);  %send experiment to platform
    pause(0.02);
    reply=fgetl(s) %check if experiment was valid
    if strcmp(reply,'goodexp')  
         while s.BytesAvailable<(length(structarray))   %read back true experiment
                pause(0.01);
            end
        read8=fread(s,length(structarray),'uint8');
        true_experiment=bytes_2_struct_new(uint8(read8));   %save true experiment
        pause(0.02);
        
         while s.BytesAvailable<(length(clusters_per_sequence)) %get calculated number of SD clusters per sequence
                pause(0.01);
            end
        clusters_per_sequence=fread(s,length(clusters_per_sequence),'uint8');
        
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

        %initialize variables to hold data
        CPX_stream=zeros(true_experiment.Nexperiments,true_experiment.Nsequences,most_samps,'double');
        CPX_stream_avg=zeros(true_experiment.Nsequences,most_samps,'double');
        CPXechoes=zeros(true_experiment.Nexperiments,true_experiment.Nsequences,most_echoes,most_Nsamp);
        CPXechoes_avg=zeros(true_experiment.Nsequences,most_echoes,most_Nsamp);

        %tic toc functions are used for TR timing
        tic;
        for experimentcount=1:experiment.Nexperiments
            for sequencecount=1:(experiment.Nsequences)
                A=experimentcount
                B=sequencecount
                exptime=toc;    %get TR timer value
                if exptime<experiment.sequence(sequencecount).TR
                   pause(experiment.sequence(sequencecount).TR-exptime)   %if timer<TR, then wait until TR has elapsed
                end
                tic;  
                
                fwrite(s,[uint8(sequencecount-1) 'sequencetoken']); %tell platform what sequence to do

                reply=fgetl(s)  %platform will tell MATLAB if data was transferred successfully
                if strcmp(reply,'good') %transfer succeeded
                    for clustercount=1:clusters_per_sequence(sequencecount) %get raw data
                        while s.BytesAvailable<16384
                            pause(0.001);
                        end
                        raw16((clustercount-1)*16384/2+1:clustercount*16384/2)=fread(s,16384/2,'uint16');
                    end
                    %convert this raw binary data to proper length, and
                    %double type
                    rawsequence=raw16(1:double(uint32(experiment.sequence(sequencecount).cpmg.Nsamp)*uint32(experiment.sequence(sequencecount).cpmg.Nechos)+1)*2);
%                     rawsequence_2=rawsequence;
                    rawsequence=bitand((2^14-1),swapbytes(uint16(rawsequence)));
                    rawsequence=uint16(rawsequence*4);
                    rawsequence=typecast(rawsequence,'int16');
                    rawsequence=(rawsequence)/4;
                    rawsequence=double(rawsequence);

                    %do the same to the offset
                    rawoffset=raw16((double(uint32(experiment.sequence(sequencecount).cpmg.Nsamp)*uint32(experiment.sequence(sequencecount).cpmg.Nechos)+1)*2)+1:double(uint32(experiment.sequence(sequencecount).cpmg.Nsamp)*uint32(experiment.sequence(sequencecount).cpmg.Nechos)+uint32(experiment.Noffsetsamples)+1)*2);
%                     rawoffset_2=rawoffset;
                    rawoffset=bitand((2^14-1),swapbytes(uint16(rawoffset)));
                    rawoffset=uint16(rawoffset*4);
                    rawoffset=typecast(rawoffset,'int16');
                    rawoffset=(rawoffset)/4;
                    rawoffset=double(rawoffset);

                    %now convert it to complex data
                    CPXoffset=zeros(1,length(rawoffset)/2);
                    for i=1:length(rawoffset)/2 
                        CPXoffset(1,i)=rawoffset(2*(i)-1)+1i*rawoffset(2*(i));
                    end

                    CPX=zeros(1,length(rawsequence)/2-1);
                    for i=1:length(CPX)
                        CPX(1,i)=rawsequence(2*(i)+1)+1i*rawsequence(2*(i)+2);
                    end

                    %subtract offset
                    CPX_stream(experimentcount,sequencecount,1:length(CPX))=CPX-mean(CPXoffset);

                    %preview of CPX
                    figure(1)
                    plot([1:50],real(CPX(1:50)),[1:50],imag(CPX(1:50)))

                    %now break 3 dimensional CPX_stream into 4 dimensional
                    %CPXechoes
                    for i=1:true_experiment.sequence(sequencecount).cpmg.Nechos
                        CPXechoes(experimentcount,sequencecount,i,:)=CPX_stream(experimentcount,sequencecount,(double(true_experiment.sequence(sequencecount).cpmg.Nsamp)*(i-1)+1):(double(true_experiment.sequence(sequencecount).cpmg.Nsamp)*i));
                    end

                    %do average of both
                    CPX_stream_avg(sequencecount,:)=squeeze(CPX_stream_avg(sequencecount,:))+squeeze(CPX_stream(experimentcount,sequencecount,:)).';
                    CPXechoes_avg(sequencecount,:,:)=squeeze(CPXechoes_avg(sequencecount,:,:))+squeeze(CPXechoes(experimentcount,sequencecount,:,:));
                    selectecho=2;
                    %display a sample echo
                    figure(4)
                    plot([1:length(CPX)],real(squeeze(CPX_stream(experimentcount,sequencecount,1:length(CPX)))),[1:length(CPX)],imag(squeeze(CPX_stream(experimentcount,sequencecount,1:length(CPX)))))
                    title('CPX stream')
                    
                    figure(2)
                    plot([1:length(CPXechoes_avg(sequencecount,selectecho,:))],squeeze(real(CPXechoes_avg(sequencecount,selectecho,:))./double(experimentcount)),[1:length(CPXechoes_avg(sequencecount,selectecho,:))],squeeze(imag(CPXechoes_avg(sequencecount,selectecho,:))./double(experimentcount)))
                    
                    figure(3)
                    plot([1:length(CPX_stream_avg(sequencecount,:))],real(CPX_stream_avg(sequencecount,:)),[1:length(CPX_stream_avg(sequencecount,:))],imag(CPX_stream_avg(sequencecount,:)))
                    title('CPX stream average')
                else
                    uiwait(msgbox('Transfer was bad')); %there was an error
                     while s.BytesAvailable<2
                            pause(0.001);
                      end
                            failure_cause=fread(s,1,'uint8')    %get error cause code
                           r1=fread(s,1,'uint8')    %and break out of routine, continue with next sequence
                    
                     
                end
                
            end
        end
        
        
        
        
        
        
        
        
        
        
    elseif strcmp(reply,'badexp')
        uiwait(msgbox('Experiment is bad'));
        while s.BytesAvailable<1
                            pause(0.001);
        end
        
        failure_cause=fread(s,1,'uint8')
    end
elseif strcmp(reply,'bad SD card')
    
    uiwait(msgbox('BAD SD CARD'));
    return;
end



end

