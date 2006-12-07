Wp = [2200 12000]/22050
Rp = 0.005; Rs = 1000;
[n, Wc] = cheb1ord(Wp, [0, 1], Rp, Rs)
[a, b] = cheby1(n, Rp, Wc)
freqz(a,b,1024,44100);
