#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.13 (Berkeley) 3/18/86
#
SRCS=	getnetbyaddr.c getnetbyname.c \
	getnetgrent.c \
	innetgr.c \
	rcmd.c rexec.c ruserpass.c \
	res_comp.c res_debug.c res_init.c res_mkquery.c res_send.c

OBJS=	getnetbyaddr.o getnetbyname.o \
	getnetgrent.o \
	innetgr.o \
	rcmd.o rexec.o ruserpass.o \
	res_comp.o res_debug.o res_init.o res_mkquery.o res_send.o

CFLAGS=	-O -DADDRSORT -DUW
TAGSFILE=tags

.c.o:
	${CC} -p -c ${CFLAGS} $*.c
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

netlib netlib_p: ${OBJS}
	@echo "building profiled netlib"
	@cd profiled; ar cru ../netlib_p ${OBJS}
	@echo "building normal netlib"
	@ar cru netlib ${OBJS}

tags:
	cwd=`pwd`; \
	for i in ${SRCS}; do \
		ctags -a -f ${TAGSFILE} $$cwd/$$i; \
	done

clean:
	rm -f *.o profiled/*.o errs a.out core netlib netlib_p \
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

getnetgrent.o: getnetgrent.c /usr/include/stdio.h /usr/include/ctype.h
getnetgrent.o: /usr/include/rpcsvc/ypclnt.h
innetgr.o: innetgr.c /usr/include/stdio.h /usr/include/ctype.h
innetgr.o: /usr/include/rpcsvc/ypclnt.h
rcmd.o: rcmd.c /usr/include/stdio.h /usr/include/ctype.h /usr/include/pwd.h
rcmd.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
rcmd.o: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/file.h
rcmd.o: /usr/include/sys/signal.h /usr/include/sys/socket.h
rcmd.o: /usr/include/sys/stat.h /usr/include/netinet/in.h /usr/include/netdb.h
rcmd.o: /usr/include/errno.h
rexec.o: rexec.c /usr/include/sys/types.h /usr/include/sys/socket.h
rexec.o: /usr/include/netinet/in.h /usr/include/stdio.h /usr/include/netdb.h
rexec.o: /usr/include/errno.h
ruserpass.o: ruserpass.c /usr/include/stdio.h /usr/include/utmp.h
ruserpass.o: /usr/include/ctype.h /usr/include/sys/types.h
ruserpass.o: /usr/include/sys/stat.h /usr/include/errno.h
res_comp.o: res_comp.c /usr/include/sys/types.h /usr/include/stdio.h
res_comp.o: /usr/include/arpa/nameser.h
res_debug.o: res_debug.c /usr/include/sys/types.h /usr/include/netinet/in.h
res_debug.o: /usr/include/stdio.h /usr/include/arpa/nameser.h
res_init.o: res_init.c /usr/include/sys/types.h /usr/include/sys/socket.h
res_init.o: /usr/include/netinet/in.h /usr/include/stdio.h
res_init.o: /usr/include/arpa/nameser.h /usr/include/resolv.h
res_mkquery.o: res_mkquery.c /usr/include/stdio.h /usr/include/sys/types.h
res_mkquery.o: /usr/include/netinet/in.h /usr/include/arpa/nameser.h
res_mkquery.o: /usr/include/resolv.h
res_send.o: res_send.c /usr/include/sys/param.h
res_send.o: /usr/include/machine/machparam.h /usr/include/signal.h
res_send.o: /usr/include/sys/types.h /usr/include/sys/time.h
res_send.o: /usr/include/time.h /usr/include/sys/socket.h
res_send.o: /usr/include/sys/uio.h /usr/include/netinet/in.h
res_send.o: /usr/include/stdio.h /usr/include/errno.h
res_send.o: /usr/include/arpa/nameser.h /usr/include/resolv.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
