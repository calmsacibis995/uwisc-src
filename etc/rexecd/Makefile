#
#	Makefile for rexecd
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	rexecd
SRCS=		rexecd.c
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
rexecd: rexecd.c /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
rexecd: /usr/include/sys/ttydev.h /usr/include/sys/param.h
rexecd: /usr/include/machine/machparam.h /usr/include/signal.h
rexecd: /usr/include/sys/types.h /usr/include/sys/socket.h
rexecd: /usr/include/sys/time.h /usr/include/time.h /usr/include/netinet/in.h
rexecd: /usr/include/stdio.h /usr/include/errno.h /usr/include/pwd.h
rexecd: /usr/include/signal.h /usr/include/netdb.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
