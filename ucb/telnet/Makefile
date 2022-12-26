#
#	Makefile for telnet
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	telnet
SRCS=		telnet.c
LIBS=

${PROGRAM}:	${SRCS}
	${CC} ${CFLAGS} -o $@ $@.c ${LIBS}

install:
	install -s ${PROGRAM} ${DESTDIR}/usr/ucb/${PROGRAM}

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
telnet: telnet.c /usr/include/sys/types.h /usr/include/sys/socket.h
telnet: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
telnet: /usr/include/sys/ttydev.h /usr/include/sys/time.h /usr/include/time.h
telnet: /usr/include/netinet/in.h /usr/include/arpa/telnet.h
telnet: /usr/include/arpa/inet.h /usr/include/stdio.h /usr/include/ctype.h
telnet: /usr/include/errno.h /usr/include/signal.h /usr/include/setjmp.h
telnet: /usr/include/netdb.h /usr/include/strings.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
