#	Makefile	4.2	85/04/03

CFLAGS= -O


OBJS= att2224.o bsdtcp.o cds224.o df2.o df12.o dk.o dn.o \
	hys.o hysq.o hys24.o mic.o nov.o pen.o pnet.o rvmacs.o sy.o \
	unet.o usr2400.o va212.o va811.o va820.o vad.o vent.o vmacs.o

aculib.a: ${OBJS}
	ar cr aculib.a ${OBJS}
	-ranlib aculib.a 2>&1 | sed '/no symbol table/d'

${OBJS}: ../uucp.h ../condevs.h

clean:
	rm -f *.o core errs  aculib.a
