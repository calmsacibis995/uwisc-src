#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	6.3 (Berkeley) 3/18/86
#

OBJS=	ether_addr.o getnetent.o getproto.o getprotoent.o getprotoname.o \
		getrpcent.o getservent.o getservbyname.o getservbyport.o 

SRCS=	ether_addr.c getnetent.c getproto.c getprotoent.c getprotoname.c \
		getrpcent.c getservent.c getservbyname.c getservbyport.c 

CFLAGS= -O -DADDRSORT -DUW
TAGSFILE= tags
DESTDIR=

.c.o:
	${CC} -p -c ${CFLAGS} $*.c
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

tablelib tablelib_p: ${OBJS} 
	@echo "building profiled tablelib"
	@cd profiled; ar cru ../tablelib_p ${OBJS}
	@echo "building normal tablelib"
	@ar cru tablelib ${OBJS}
	
tags:
	cwd=`pwd`; \
	for i in ${SRCS}; do \
		ctags -a -f ${TAGSFILE} $$cwd/$$i; \
	done

clean:
	rm -f *.o errs a.out core tablelib tablelib_p profiled/*.o \
		tags Makefile.bak

co:
	co ${COFLAGS} ${SRCS}

ci:
	ci ${CIFLAGS} ${SRCS}

depend:
	for i in ${SRCS}; do \
	    cc -M ${CFLAGS} $$i | awk ' { if ($$1 != prev) \
		{ if (rec != "") print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		else rec = rec " " $$2 } } \
		END { print rec } ' >> makedep; done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it

gethostnamadr.o: gethostnamadr.c /usr/include/sys/param.h
gethostnamadr.o: /usr/include/machine/machparam.h /usr/include/signal.h
gethostnamadr.o: /usr/include/sys/types.h /usr/include/sys/socket.h
gethostnamadr.o: /usr/include/netinet/in.h /usr/include/sys/types.h
gethostnamadr.o: /usr/include/ctype.h /usr/include/netdb.h /usr/include/stdio.h
gethostnamadr.o: /usr/include/errno.h /usr/include/arpa/nameser.h
gethostnamadr.o: /usr/include/resolv.h
sethostent.o: sethostent.c
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
