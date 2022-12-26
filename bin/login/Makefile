#
#	Makefile for login
#
DESTDIR=
CFLAGS=	-O -DUW -DVFS

PROGRAM=	login
SRCS=		login.c
LIBS=

${PROGRAM}:	${SRCS}
	${CC} ${CFLAGS} -o $@ $@.c ${LIBS}

install:
	install -o root -m 4755 -s ${PROGRAM} ${DESTDIR}/bin/${PROGRAM}

tags:
	ctags -tdw *.c 

clean:
	rm -f ${PROGRAM} a.out core *.s *.o made


depend: 
	for i in ${SRCS}; do \
	    cc -M ${INCPATH} ${CFLAGS} $$i | sed 's/\.o//' | \
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
login: login.c /usr/include/sys/param.h /usr/include/machine/machparam.h
login: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/stat.h
login: /usr/include/sys/time.h /usr/include/time.h /usr/include/sys/resource.h
login: /usr/include/sys/file.h /usr/include/sgtty.h /usr/include/sys/ioctl.h
login: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
login: /usr/include/utmp.h /usr/include/signal.h /usr/include/pwd.h
login: /usr/include/stdio.h /usr/include/lastlog.h /usr/include/errno.h
login: /usr/include/ttyent.h /usr/include/syslog.h /usr/include/grp.h
login: /usr/include/ctype.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
