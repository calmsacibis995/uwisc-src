#
#	Makefile for bc
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	bc
SRCS=		
LIBS=

${PROGRAM}:	
	${YACC} ${YFLAGS} $@.y
	${CC} -c ${CFLAGS} y.tab.c
	mv y.tab.o $@.o
	${CC} -o $@ $@.o
	rm -f $@.o

install:
	install -s ${PROGRAM} ${DESTDIR}/usr/bin/${PROGRAM}
	echo '# see make depend above' >> Makefile
tags:
	ctags -tdw *.c 

clean:
	rm -f ${PROGRAM} a.out core *.s *.o made


depend: 
	for i in ${PROGRAM} ; do \
	    cc -M ${INCPATH} $$i.y | sed 's/\.o//' | \
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
bc: bc.y /usr/include/stdio.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
# see make depend above
# see make depend above
# see make depend above
# see make depend above
# see make depend above
# see make depend above
