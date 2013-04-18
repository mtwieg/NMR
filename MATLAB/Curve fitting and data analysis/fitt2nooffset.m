function err=fitt2nooffset(x,time,data)

fit=x(1).*exp(-time./x(2));

err=sum(abs(fit(:)-data(:)).^2);

% plot(time,data,time,fit);drawnow;



