#
#	Makefile for syslogd
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	syslogd
SRCS=		syslogd.c
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
syslogd: syslogd.c /usr/include/errno.h /usr/include/stdio.h
syslogd: /usr/include/utmp.h /usr/include/ctype.h /usr/include/signal.h
syslogd: /usr/include/sysexits.h /usr/include/strings.h
syslogd: /usr/include/sys/syslog.h /usr/include/sys/types.h
syslogd: /usr/include/sys/param.h /usr/include/machine/machparam.h
syslogd: /usr/include/signal.h /usr/include/sys/types.h
syslogd: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
syslogd: /usr/include/sys/ttydev.h /usr/include/sys/stat.h
syslogd: /usr/include/sys/wait.h /usr/include/sys/socket.h
syslogd: /usr/include/sys/file.h /usr/include/sys/msgbuf.h
syslogd: /usr/include/sys/uio.h /usr/include/sys/un.h /usr/include/sys/time.h
syslogd: /usr/include/time.h /usr/include/sys/resource.h
syslogd: /usr/include/netinet/in.h /usr/include/netdb.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
