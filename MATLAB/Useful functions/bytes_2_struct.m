function [struct] = bytes_2_struct(bytes)
%this function takes a string of bytes sent from the platform and returns
%the experiment structure in its proper form

global experiment;
byteptr=1;
fields={'MODE', 'Nsequences', 'sequence', 'TPArise', 'Noffsetsamples', 'dt_offsetsamples', 'Nexperiments','Vgain'};

cellstruct_1=struct2cell(experiment);
for i=1:length(cellstruct_1)
    if isstruct(cellstruct_1{i})
        cellstruct_2=squeeze(struct2cell(cellstruct_1{i}));
        tempsize=size(cellstruct_2);
        for j=1:tempsize(2);
            for k=1:tempsize(1);
                if isstruct(cellstruct_2{k,j})
                    cellstruct_3=squeeze(struct2cell(cellstruct_2{k,j}));
                    tempsize2=size(cellstruct_3);
                    for l=1:tempsize2(2);
                        for m=1:tempsize2(1);
                                a=cellstruct_3{m,l};
                                b=whos('a');
                                c=b.bytes;
                                cellstruct_3{m,l}=swapbytes(typecast(bytes(byteptr:byteptr+c-1),class(cellstruct_3{m,l})));
                                byteptr=byteptr+c;
                        end
                    end
                    if length(cellstruct_3)==11
                        fields_3={'tau','pulsewidth','pulsephase','pulseamp','Vb','LOphase','LOamp','Nsamp','Nechos','dt','samp_offset'};
                    else
                        fields_3={'tau','pulsewidth','pulsephase','pulseamp','Vb'};
                    end
                   
                    
                    cellstruct_2{k,j}=cell2struct(cellstruct_3,fields_3,1)';
%                     cellstruct_2{k,j}=cellstruct_3;
                else
                     a=cellstruct_2{k,j};
                     b=whos('a');
                     c=b.bytes;
                     cellstruct_2{k,j}=swapbytes(typecast(bytes(byteptr:byteptr+c-1),class(cellstruct_2{k,j})));
                     byteptr=byteptr+c;
                end
            end
        end
        fields_2={'f0','preppulse','cpmg','Npreppulses','echofirst','TR'};
%         x=cell2struct(cellstruct_2,fields4,1)'
%         struct=cell2struct(cellstruct_2,fields,1)
        cellstruct_1{i}=cell2struct(cellstruct_2,fields_2,1)';
%         cellstruct_1{i}=cellstruct_2;
    else
         a=cellstruct_1{i};
         b=whos('a');
         c=b.bytes;
        cellstruct_1{i}=swapbytes(typecast(bytes(byteptr:byteptr+c-1),class(cellstruct_1{i})));
        byteptr=byteptr+c;
    end
end

struct=cell2struct(cellstruct_1,fields,1);

end

