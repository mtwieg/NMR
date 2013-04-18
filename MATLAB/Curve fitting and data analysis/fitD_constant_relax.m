function err=fitD_constant_relax(x,TE3,lnI)

fit=x(1)+x(2)*TE3;

err=sum(abs(fit(:)-lnI(:)).^2);

% plot(TE3,lnI,TE3,fit);drawnow;



