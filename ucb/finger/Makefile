#
#	Makefile for finger
#
DESTDIR=
CFLAGS=	-O -DUW -DYP

PROGRAM=	finger
SRCS=		finger.c
LIBS=

${PROGRAM}:	${SRCS}
	${CC} ${CFLAGS} -o $@ $@.c ${LIBS}

install:
	install -s ${PROGRAM} ${DESTDIR}/usr/ucb/${PROGRAM}
	rm -f ${DESTDIR}/usr/ucb/f
	ln ${DESTDIR}/usr/ucb/finger ${DESTDIR}/usr/ucb/f

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
finger: finger.c /usr/include/sys/types.h /usr/include/sys/stat.h
finger: /usr/include/utmp.h /usr/include/sys/signal.h /usr/include/pwd.h
finger: /usr/include/stdio.h /usr/include/lastlog.h /usr/include/ctype.h
finger: /usr/include/sys/time.h /usr/include/time.h /usr/include/sys/socket.h
finger: /usr/include/netinet/in.h /usr/include/netdb.h /usr/include/ndbm.h
finger: /usr/include/sys/file.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
