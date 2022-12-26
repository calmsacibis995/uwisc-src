#
#	Makefile for cron
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	cron
SRCS=		cron.c
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

cron: cron.c /usr/include/sys/types.h /usr/include/stdio.h /usr/include/ctype.h
cron: /usr/include/signal.h /usr/include/sys/time.h /usr/include/time.h
cron: /usr/include/sys/stat.h /usr/include/sys/wait.h /usr/include/sys/ioctl.h
cron: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
cron: /usr/include/sys/file.h /usr/include/pwd.h /usr/include/syslog.h
cron: /usr/include/fcntl.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
