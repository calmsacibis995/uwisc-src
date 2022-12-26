#
#	Makefile for rlogin
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	rlogin
SRCS=		rlogin.c
LIBS=

${PROGRAM}:	${SRCS}
	${CC} ${CFLAGS} -o $@ $@.c ${LIBS}

install:
	install -o root -m 4755 -s ${PROGRAM} ${DESTDIR}/usr/ucb/${PROGRAM}

tags:
	ctags -tdw *.c 

clean:
	rm -f ${PROGRAM} a.out core *.s *.o made


depend: 
	for i in ${SRCS}; do \
	    cc -M ${INCPATH} $$i | sed 's/\.o//' | \
	    awk ' { if ($$1 != prev) \
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
# DEPENDENCIES MUST END AT END OF FILE
rlogin: rlogin.c /usr/include/sys/param.h /usr/include/machine/machparam.h
rlogin: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/errno.h
rlogin: /usr/include/sys/file.h /usr/include/sys/socket.h
rlogin: /usr/include/sys/wait.h /usr/include/netinet/in.h /usr/include/stdio.h
rlogin: /usr/include/sgtty.h /usr/include/sys/ioctl.h
rlogin: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
rlogin: /usr/include/errno.h /usr/include/pwd.h /usr/include/signal.h
rlogin: /usr/include/setjmp.h /usr/include/netdb.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
