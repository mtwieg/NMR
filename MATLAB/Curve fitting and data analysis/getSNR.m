function [ SNR ] = getSNR( echovalues,time,guessoffset,guessamplitude,guessT2 )
%used to compute the SNR of an exponential decay
%input the echo values (real), time axis, and the estimated exponential fit
%parameters, and you get back the standard deviation of the error
x=fminsearch('fitt2offset',[guessoffset guessamplitude guessT2],[],time,echovalues);
fit=x(1)+x(2).*exp(-time./x(3));
error=echovalues-fit;
figure(10)
plot(time,error); drawnow;
SNR=x(2)/std(error);

end

