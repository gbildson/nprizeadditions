CFLAGS=-O3 
#CFLAGS=-O3 '-Wl,--large-address-aware' -lm -llapack
#CFLAGS=-O3 -ffast-math -fomit-frame-pointer -malign-double -mtune=i686 

all: rbm ubest rbmcond

rbm: utest.o basic.o rbm.o weight.o global.o mix2.o 
	$(CC) -o $@ $^ -lm -llapack

rbmcond: utest.o basic.o rbmcond.o weight.o global.o mix2.o 
	$(CC) -o $@ $^ -lm -llapack

ubest: utest.o basic.o ubest.o weight.o global.o mix2.o 
	$(CC) -o $@ $^ -lm -llapack

clean:
	rm *.o *.stackdump rbm rbmcond ubest *.exe
