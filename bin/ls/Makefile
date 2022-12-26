#
#	Makefile for ls
#
DESTDIR=
CFLAGS=	-O -DUW -DVFS -DYP

PROGRAM=	ls
SRCS=		ls.c
LIBS=

${PROGRAM}:	${SRCS}
	${CC} ${CFLAGS} -o $@ $@.c ${LIBS}

install:
	install -s ${PROGRAM} ${DESTDIR}/bin/${PROGRAM}

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
ls: ls.c /usr/include/sys/param.h /usr/include/machine/machparam.h
ls: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/stat.h
ls: /usr/include/sys/dir.h /usr/include/stdio.h /usr/include/sgtty.h
ls: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
ls: /usr/include/sys/ttydev.h /usr/include/pwd.h /usr/include/grp.h
ls: /usr/include/utmp.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
