#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.10 (Berkeley) 6/7/86
#
DESTDIR=
CFLAGS=	-O
TAGSFILE=tags

# Subdirectories whose routines are included in the making of the
# master tags file (/usr/lib/tags); the Fortran libraries should
# be on this list, but we don't control them....
#
TAGSDIR=libcurses libdbm libln libm libmp libpc librpcsvc libtermlib

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	getNAME learn lib2648 libF77 libI77 libU77 libcurses libdbm libg \
	libln libom libm libmp libpc libplot librpcsvc libtermlib liby lpr \
	makekey makewhatis man me sendmail

all:	${SUBDIR} 

${SUBDIR}: FRC
	echo $@; cd $@; make ${MFLAGS}

FRC:

install:
	-for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); done
	install -c lib.b ${DESTDIR}/usr/lib/lib.b

tags:
	-for i in ${TAGSDIR}; do \
		(cd $$i; make ${MFLAGS} TAGSFILE=../${TAGSFILE} tags); \
	done

clean:
	rm -f a.out core *.s *.o tags
	-for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done

depend:
	-for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} depend); done

# Files listed in ${NSTD} have explicit make lines given below.

# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
