#
#	Makefile for mail
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	mail
SRCS=		mail.c
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
mail: mail.c /usr/include/sys/types.h /usr/include/sys/stat.h
mail: /usr/include/sys/file.h /usr/include/ctype.h /usr/include/stdio.h
mail: /usr/include/pwd.h /usr/include/utmp.h /usr/include/signal.h
mail: /usr/include/setjmp.h /usr/include/sysexits.h /usr/include/sys/socket.h
mail: /usr/include/netinet/in.h /usr/include/netdb.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
