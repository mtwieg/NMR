function [structarray] = struct_2_bytes(struct)
%this function takes an experiment structure and returns a string of bytes
%in to send to the platform in the proper order

structarray=uint8(0);

mystruct=struct2cell(struct);
for i=1:length(mystruct)
%      temp=mystruct{i};
    if isstruct(mystruct{i})
        struct_2=squeeze(struct2cell(mystruct{i}));
        tempsize=size(struct_2);
        for j=1:tempsize(2);
            for k=1:tempsize(1);
%                 temp3=struct_2{k,j};
                if isstruct(struct_2{k,j})
                    struct_3=squeeze(struct2cell(struct_2{k,j}));
                    tempsize2=size(struct_3);
                    for l=1:tempsize2(2);
                        for m=1:tempsize2(1);
                             struct_3{m,l};
                            structarray=[structarray typecast( swapbytes(struct_3{m,l}),'uint8')];
                        end
                    end
                else
                     struct_2{k,j};
                    structarray=[structarray typecast(swapbytes(struct_2{k,j}),'uint8')];
                end
            end
        end
    else
         mystruct{i};
        structarray=[structarray typecast(swapbytes(mystruct{i}),'uint8')];
    end
end
structarray=structarray(2:length(structarray));

end

