function err=fitt2offset(x,time,data)

fit=x(1)+x(2).*exp(-time./x(3));

err=sum(abs(fit(:)-data(:)).^2);

%       plot(time,data,time,fit);
%           drawnow;



