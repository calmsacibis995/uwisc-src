#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.17 (Berkeley) 6/7/86
#
DESTDIR=
CFLAGS=	-O

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	Mail apply biff checknr clear colcrt colrm compress ctags \
		dbx error ex expand finger fold fp from ftp gcore gprof grep \
		groups head indent last lastcomm leave lisp lock logger man \
		mkstr more msgs netstat pascal printenv quota rdist rlogin \
		rsh rup ruptime rusers rwho sccs script sendbug soelim strings \
		symorder sysline systat tail talk tcopy telnet tftp tn3270 tset \
		ul unexpand unifdef users vacation vgrind vlp vmstat w wc what \
		whereis which whoami whois window xstr yes

all:	${SUBDIR} 

${SUBDIR}: FRC
	cd $@; make ${MFLAGS} 

FRC:

install: ${SUBDIR}
	-for i in ${SUBDIR}; do \
		(cd $$i; \
		echo ''; echo "****** $$i *****"; echo ''; \
		make ${MFLAGS} DESTDIR=${DESTDIR} install); \
	done

clean:
	rm -f a.out core *.s *.o made
	-for i in ${SUBDIR}; do \
		(cd $$i; \
		echo ''; echo "****** $$i *****"; echo ''; \
		make ${MFLAGS} clean); \
	done


depend: 
	-for i in ${SUBDIR}; do \
		(cd $$i; \
		echo ''; echo "****** $$i *****"; echo ''; \
		make ${MFLAGS} depend); \
	done

tags: 
	-for i in ${SUBDIR}; do \
		(cd $$i; \
		echo ''; echo "****** $$i *****"; echo ''; \
		make ${MFLAGS} tags); \
	done

