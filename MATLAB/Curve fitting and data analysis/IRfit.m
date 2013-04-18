function err=IRfit(x,TI,data)
%x(1) is equilibrium amplitude, x(2) is inverted magnetization, x(3) is T1
fit=x(1)-(x(1)-x(2)).*exp(-TI./x(3));

err=sum(abs(fit(:)-data(:)).^2);

      plot(TI,data,TI,fit);
          drawnow;



