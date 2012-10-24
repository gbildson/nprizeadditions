CFLAGS=-O3 -ffast-math -fomit-frame-pointer -malign-double 

all: svdppmkt ubest ubestan moviebin2userbin utest0b1 utest0b2 sim usvdbkw1 usvdbkw4 usvdbkw5 usvdbkw6 usvdbkw7 usvdbkw8 usvdbkw9 usvdbkw16 usvdspp uspptime uspptime3 usvdspppure usvdspp usvdspp2 usvdspp3 usvdspp4 usvdint usvdint3d usvdint2 utest10 utest104 utest105 utest106 utest107 utest109 utest1010 utest1011 utest1012 utest1013 utest1014 utest1015 utest1016 utestsim1 utestsim2 utestsim3

moviebin2userbin: moviebin2userbin.o basic.o
	$(CC) -o $@ $^

utest0b1: utest.o global.o basic.o ubaseline1.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a 
	$(CC) -o $@ $^

utest0b2: utest2.o global.o basic.o ubaseline1.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a 
	$(CC) -o $@ $^

sim: sim.o global.o basic.o ubaseline1.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a 
	$(CC) -o $@ $^

usvdbkw1: utest.o basic.o usvdbkw1.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

usvdbkw4: utest.o basic.o usvdbkw4.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

usvdbkw5: utest.o basic.o usvdbkw5.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

usvdbkw6: utest.o basic.o usvdbkw6.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

usvdbkw7: utest.o basic.o usvdbkw7.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

usvdbkw8: utest.o basic.o usvdbkw8.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

usvdbkw9: utest.o basic.o usvdbkw9.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

usvdbkw16: utest.o basic.o usvdbkw16.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

uspptime: utest.o basic.o uspptime.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

uspptime3: utest.o basic.o uspptime3.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

usvdspppure: utest.o basic.o usvdspppure.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

svdppmkt: svdppmkt.o 
	$(CC) -o $@ $^

usvdspp: utest.o basic.o usvdspp.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

usvdspp2: utest.o basic.o usvdspp2.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

usvdspp3: utest.o basic.o usvdspp3.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

usvdspp4: utest.o basic.o usvdspp4.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

ubest: utest.o basic.o ubest.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

ubestan: utest.o basic.o ubestan.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

usvdint: utest.o basic.o usvdint.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

usvdint3d: utest.o basic.o usvdint3d.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

usvdint2: utest.o basic.o usvdint2.o weight.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utest10: utest.o basic.o usvdns1b.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utest102: utest.o basic.o usvdns1b2.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utest104: utest.o basic.o usvdns1b4.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utest105: utest.o basic.o usvdns1b5.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utest107: utest.o basic.o usvdns1b7.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utest109: utest.o basic.o usvdns1b9.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utest1010: utest.o basic.o usvdns1b10.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utest1011: utest.o basic.o usvdns1b11.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utest1012: utest.o basic.o usvdns1b12.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utest1013: utest.o basic.o usvdns1b13.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utest1014: utest.o basic.o usvdns1b14.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utest1015: utest.o basic.o usvdns1b15.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utest1016: utest.o basic.o usvdns1b16.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utest106: utest.o basic.o usvdns1b6.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utestsim1: utest.o basic.o usvdnsim1.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utestsim2: utest.o basic.o usvdnsim2.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

utestsim3: utest.o basic.o usvdnsim3.o global.o mix2.o lapack_LINUX.a blas_LINUX.a tmglib_LINUX.a libf2c.a
	$(CC) -o $@ $^

clean:
	rm *.o *.stackdump moviebin2userbin uuserbin2moviebin utest[0-9] *.exe
