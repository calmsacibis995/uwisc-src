#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	4.19 (Berkeley) 3/9/86
#
#
# The system has two monitoring schemes, utilizing either ``prof''
# or ``gprof''. DFLMON determines the default profiler; mon.o
# specifies that ``prof'' is the default, gmon.o specifies that
# ``gprof'' is the default.
#
# Machine dependent routines are located in a subtree which parallels
# the top directories.  This subtree is identified by the machine name.
#
# Compatibility routines are kept in directories with a prefixing
# ``compat'' (so they all sort together).
# 
# The C run-time startup code is always machine dependent and expected
# to be located in ${MACHINE}/csu
#
# All files contain sccsid strings, but these are not compiled into
# library objects by default, as a space-saving measure.  To produce
# a library that contains these strings in every object except
# system call stubs, add -DLIBC_SCCS to DEFS below; to put these
# strings into system call stubs, use -DSYSLIBC_SCCS.
#
# There are two possible methods of doing host name look up.  They are
# to use the name server or /etc/hosts. HOSTLOOKUP defines which method
# is to be compiled into libc  :
#								     
#	defining HOSTLOOKUP to be "named", compiles the         
#	host lookup routines that use the BIND name server.
#
#	defining HOSTLOOKUP to be "hosttable", compiles
#	the host lookup routines that use /etc/hosts.
#
# There are alos two possible methods of doing network table (services, 
# networks, protocols ...) look up.  They are to use the yellow pages
# or to look in the tables.  TABLELOOKUP defines which method is to be 
# compiled into libc.
#
#	defining TABLELOOKUP to be "yp", compiles the table lookup routines
#	to use the yellow pages.
#
#	defining TABLELOOKUP to be "table", compiles the table lookup routines
#	that use the tables in /etc
#
#

#HOSTLOOKUP=	hosttable
HOSTLOOKUP=	named
#TABLELOOKUP= yp
TABLELOOKUP= table
#							
DESTDIR=
INSTALL=install -m 644
DFLMON=	mon.o
MACHINE=vax
DEFS=
COMPAT=	compat-4.1 compat-sys5
ALL=	gen inet net ns rpc stdio yp ${MACHINE} ${COMPAT}
TAGSFILE=tags
RCSNAME=NFS
RCSMSG="Added nfs system calls, rpc, and xdr routines"
CIFLAGS='-q -f -m${RCSMSG} -N${RCSNAME}'
COFLAGS='-q'

libc.a libc_p.a: ${MACHINE}/csu ${ALL} net/${HOSTLOOKUP} net/${TABLELOOKUP}
	rm -rf t1 tmp
	-mkdir tmp
	for i in ${ALL}; do (cd tmp; ar x ../$$i/$${i}lib); done
	cd tmp; ar x ../net/${HOSTLOOKUP}/hostlib
	cd tmp; ar x ../net/${TABLELOOKUP}/tablelib
	ls tmp/*.o | sort -t/ +1 > t1
	ar cr libc.a `lorder \`cat t1\` | tsort` ${MACHINE}/csu/${DFLMON}
	ar ma findiop.o libc.a exit.o
	ar ma exit.o libc.a fakcu.o
	rm -f t1 tmp/*
	for i in ${ALL}; do (cd tmp; ar x ../$$i/$${i}lib_p); done
	cd tmp; ar x ../net/${HOSTLOOKUP}/hostlib_p
	cd tmp; ar x ../net/${TABLELOOKUP}/tablelib_p
	ls tmp/*.o | sort -t/ +1 > t1
	ls tmp/*.o | sort -t/ +1 > t1
	ar cr libc_p.a `lorder \`cat t1\` | tsort` ${MACHINE}/csu/${DFLMON}
	ar ma findiop.o libc_p.a exit.o
	ar ma exit.o libc_p.a fakcu.o
	rm -rf t1 tmp

${MACHINE}/csu ${ALL} net/${HOSTLOOKUP} net/${TABLELOOKUP}: FRC
	cd $@; make ${MFLAGS} DEFS="${DEFS}"

FRC:

install: 
	${INSTALL} libc.a ${DESTDIR}/lib/libc.a
	ranlib ${DESTDIR}/lib/libc.a
	${INSTALL} libc_p.a ${DESTDIR}/usr/lib/libc_p.a
	ranlib ${DESTDIR}/usr/lib/libc_p.a
	cd ${MACHINE}/csu; make DESTDIR=${DESTDIR} install

tags:
	for i in ${ALL}; do \
		(cd $$i; make ${MFLAGS} TAGSFILE=../${TAGSFILE} tags); \
	done
	cd net/${HOSTLOOKUP}; \
		make ${MFLAGS} TAGSFILE=../../${TAGSFILE} tags

clean:
	for i in ${MACHINE}/csu ${ALL} net/hosttable net/named; \
		do (cd $$i; make ${MFLAGS} clean); done
	rm -f t1 tmp/*.o
	-rmdir tmp

co:
	for i in ${MACHINE}/csu ${ALL} net/${HOSTLOOKUP}; do \
		(cd $$i; make ${MFLAGS} COFLAGS=${COFLAGS} co); \
	done

ci:
	for i in ${MACHINE}/csu ${ALL} net/${HOSTLOOKUP}; do \
		(cd $$i; make ${MFLAGS} CIFLAGS=${CIFLAGS} ci ); \
	done

depend:
	for i in ${MACHINE}/csu ${ALL} net/hosttable net/named; \
		do (cd $$i; make ${MFLAGS} DEFS=${DEFS} depend); done
