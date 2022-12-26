#	Makefile	4.39	85/10/11
#
DESTDIR=
CFLAGS=	-O

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	ar11 at basename bc cal calendar cb checkeq col comm cpio crypt \
		cut dc deroff diction diff3 efl egrep eqn f77 fgrep file find \
		graph install.sh iostat join learn lex lint look lorder m4 mesg neqn \
		news nohup nroff paste plot prof ptx ranlib ratfor refer rev sleep \
		sort spell spline split struct sum tabs tbl tc tip tk touch tr troff \
		tsort tty uniq units uucp xsend yacc ypcat ypmatch yppasswd ypwhich

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

