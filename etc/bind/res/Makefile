#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	6.2 (Berkeley) 3/20/86
#

OBJS=	gethostnamadr.o sethostent.o res_comp.o res_debug.o \
	res_init.o res_mkquery.o res_send.o
SRCS=	gethostnamadr.c sethostent.c res_comp.c res_debug.c \
	res_init.c res_mkquery.c res_send.c

CFLAGS= -O ${DEFS} 
TAGSFILE= tags
DESTDIR=

.c.o:
	${CC} -p -c ${CFLAGS} $*.c
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

hostlib hostlib_p: ${OBJS} 
	@echo "building profiled hostlib"
	@cd profiled; ar cru ../hostlib_p ${OBJS}
	@echo "building normal hostlib"
	@ar cru hostlib ${OBJS}
	
install: hostlib hostlib_p
	install -m 644 hostlib  ${DESTDIR}/usr/lib/libresolv.a
	install -m 644 hostlib_p  ${DESTDIR}/usr/lib/libresolv_p.a
	ranlib ${DESTDIR}/usr/lib/libresolv.a
	ranlib ${DESTDIR}/usr/lib/libresolv_p.a

tags:
	cwd=`pwd`; \
	for i in ${SRCS}; do \
		ctags -a -f ${TAGSFILE} $$cwd/$$i; \
	done

clean:
	rm -f *.o errs a.out core hostlib hostlib_p profiled/*.o \
		tags Makefile.bak

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

gethostnamadr.o: gethostnamadr.c /usr/include/sys/types.h
gethostnamadr.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
gethostnamadr.o: /usr/include/netdb.h /usr/include/stdio.h
gethostnamadr.o: /usr/include/arpa/nameser.h /usr/include/resolv.h
sethostent.o: sethostent.c
res_comp.o: res_comp.c /usr/include/sys/types.h /usr/include/stdio.h
res_comp.o: /usr/include/ctype.h /usr/include/arpa/nameser.h
res_debug.o: res_debug.c /usr/include/sys/types.h /usr/include/netinet/in.h
res_debug.o: /usr/include/stdio.h /usr/include/arpa/nameser.h
res_init.o: res_init.c /usr/include/sys/types.h /usr/include/sys/socket.h
res_init.o: /usr/include/netinet/in.h /usr/include/stdio.h
res_init.o: /usr/include/arpa/nameser.h /usr/include/resolv.h
res_mkquery.o: res_mkquery.c /usr/include/stdio.h /usr/include/sys/types.h
res_mkquery.o: /usr/include/netinet/in.h /usr/include/arpa/nameser.h
res_mkquery.o: /usr/include/resolv.h
res_send.o: res_send.c /usr/include/sys/types.h /usr/include/sys/time.h
res_send.o: /usr/include/time.h /usr/include/sys/socket.h
res_send.o: /usr/include/netinet/in.h /usr/include/stdio.h /usr/include/errno.h
res_send.o: /usr/include/arpa/nameser.h /usr/include/resolv.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
