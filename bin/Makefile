#
#	Makefile for /bin
#
DESTDIR=
CFLAGS=	-O 

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	adb ar as awk cat cc chgrp chmod cmp cp csh date dd df diff \
		domainname du echo ed expr false grep hostid hostname kill ld \
		ln login ls mail make mkdir mt mv nice nm od pagesize passwd pr \
		ps pwd rcp rm rmail rmdir sed sh size strip stty su sync tar \
		tee test time tp true wall who write


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

