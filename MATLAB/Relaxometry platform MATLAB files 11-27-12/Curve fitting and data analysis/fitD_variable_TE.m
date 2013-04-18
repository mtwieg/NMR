function err=fitD_variable_TE(x,tau,T2a)

fit=(1/x(1)+x(2)*tau.^2);

err=sum(abs(fit(:)-1./T2a(:)).^2);

 plot(tau.^2,1./T2a,tau.^2,fit);

 xlabel('tau^2')
 ylabel('1/T2a')
 drawnow;



