#
#	Makefile for rmuser
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	rmuser
SRCS =		main.c rconf.c util.c 
OBJS =		main.o rconf.o util.o 
LIBS=

${PROGRAM}:	${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LIBS}

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
main: main.c /usr/include/stdio.h /usr/include/sys/file.h ./rmuser.h
main: /usr/include/grp.h /usr/include/pwd.h /usr/include/sys/types.h
main: /usr/include/dbm.h /usr/include/ndbm.h
rconf: rconf.c
util: util.c /usr/include/stdio.h /usr/include/sys/file.h ./rmuser.h
util: /usr/include/grp.h /usr/include/pwd.h /usr/include/sys/types.h
util: /usr/include/dbm.h /usr/include/ndbm.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
