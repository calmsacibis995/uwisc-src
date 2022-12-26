#
#	Makefile for telnetd
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	telnetd
SRCS=		telnetd.c
LIBS=

${PROGRAM}:	${SRCS}
	${CC} ${CFLAGS} -o $@ $@.c ${LIBS}

install:
	install -s ${PROGRAM} ${DESTDIR}/etc/${PROGRAM}

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
telnetd: telnetd.c /usr/include/sys/param.h /usr/include/machine/machparam.h
telnetd: /usr/include/signal.h /usr/include/sys/types.h
telnetd: /usr/include/sys/socket.h /usr/include/sys/wait.h
telnetd: /usr/include/sys/file.h /usr/include/sys/stat.h
telnetd: /usr/include/sys/time.h /usr/include/time.h /usr/include/netinet/in.h
telnetd: /usr/include/arpa/telnet.h /usr/include/stdio.h /usr/include/signal.h
telnetd: /usr/include/errno.h /usr/include/sgtty.h /usr/include/sys/ioctl.h
telnetd: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
telnetd: /usr/include/netdb.h /usr/include/syslog.h /usr/include/ctype.h
telnetd: /usr/include/utmp.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
