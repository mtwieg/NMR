function [] = program_experiment_to_SD()

%this function takes the experiment structure and programs it to the
%platform's SD card, for later use in standalone mode

    if exist('s')==0
        global s;
        priorPorts = instrfind;  % finds any existing Serial Ports in MATLAB
        delete(priorPorts);      % and deletes them 
        s = serial('COM13', 'BaudRate', 3000000)
        set(s,'InputBufferSize',16384,'Outputbuffersize',20000,'Timeout',5,'FlowControl','hardware');
        fopen(s)
    else
        global s;
    end
    pause(0.1);
    fprintf(s,'RESTART');
    pause(0.05);
    fprintf(s,'PROGEXP');
    pause(0.01);
    reply=fgetl(s)
    if strcmp(reply,'good SD card')
        global experiment;
        bytes=struct_2_bytes(experiment);
        fwrite(s,bytes);
        filler=ones(1,16384-length(bytes),'uint8');
        fwrite(s,filler);
        reply=fgetl(s)
    end
end

