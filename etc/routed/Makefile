#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.2 (Berkeley) 2/7/86
#
ALL=	routed tools
OBJS=	af.o if.o input.o main.o output.o startup.o tables.o timer.o \
	trace.o inet.o
CFLAGS=
C2=	/lib/c2
INLINE= /sys/machine/inline/inline
DESTDIR=
LDFLAGS=

all:	${ALL}

routed: ${OBJS}
	${CC} -o routed ${LDFLAGS} ${OBJS}

tools: FRC
	cd tools; make ${MFLAGS} DESTDIR=${DESTDIR};

install: ${ALL}
	install -s routed ${DESTDIR}/etc
	cd tools; make ${MFLAGS} DESTDIR=${DESTDIR} install;

# The following can be deleted where not appropriate to use the kernel's
# inline code expansions.
.c.o:
	${CC} -S ${CFLAGS} $*.c
	${C2} $*.s | ${INLINE} | ${AS} -o $*.o
	rm -f $*.s

clean:
	rm -f routed *.o core a.out t.c linterrs typescript errs
	cd tools; make ${MFLAGS} clean;

${OBJS}: defs.h af.h defs.h interface.h table.h trace.h \
	/usr/include/protocols/routed.h

FRC:

