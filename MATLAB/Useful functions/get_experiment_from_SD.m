function [t_experiment] = get_experiment_from_SD()
    global s;
    
    
    while s.BytesAvailable<16384
                pause(0.01);
    end
    read8=fread(s,16384,'uint8');
    
    t_experiment=bytes_2_struct(uint8(read8));
    
%     then I must parse the bytes back into an experiment structure

end

