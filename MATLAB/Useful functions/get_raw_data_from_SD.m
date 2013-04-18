function [raw_sequences] = get_raw_data_from_SD()
    %grabs raw data from last experiment from platform's SD card
    %meant for use in standalone mode only
    
    raw_sequences=0;
    global s;
    priorPorts = instrfind;  % finds any existing Serial Ports in MATLAB
    delete(priorPorts);      % and deletes them 
    s = serial('COM13', 'BaudRate', 3000000)
    set(s,'InputBufferSize',16384,'Outputbuffersize',20000,'Timeout',5,'FlowControl','hardware');
    fopen(s)
    
    global experiment;
    global true_experiment;
    declare_experiment;
    global clusters_per_sequence;
    
    fprintf(s,'GET DATA');
    pause(0.01);
    reply=fgetl(s)
    if strcmp(reply,'good SD card')
        pause(0.01);
        reply=fgetl(s)
        if strcmp(reply,'goodexperiment')
            
            declare_experiment;
            true_experiment=get_experiment_from_SD(); %get true experiment

             clusters_per_sequence=zeros(1,8);   %get clusters per sequence
            while s.BytesAvailable<(length(clusters_per_sequence))
                    pause(0.01);
            end
            clusters_per_sequence=fread(s,length(clusters_per_sequence),'uint8');

            raw_sequences=zeros(true_experiment.Nexperiments,true_experiment.Nsequences,max(clusters_per_sequence)*16384/2,'uint16');

            for i=1:true_experiment.Nexperiments
                i
                for j=1:true_experiment.Nsequences
                    fprintf(s,'SEND SEQUENCE');
                    for clustercount=1:clusters_per_sequence(j)                 
                        while s.BytesAvailable<16384
                            pause(0.01);
                        end
                        raw_sequences(i,j,((clustercount-1)*16384/2+1):clustercount*16384/2)=fread(s,(16384/2),'uint16');
                    end
                end
            end
        end
       
    end
end

