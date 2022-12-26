#
#	Makefile for /usr/etc
#
DESTDIR=
CFLAGS=	-O

# Programs that live in subdirectories, and have makefiles of their own.
#
# rarpd is not included because I can't find the include file net/nit.h
# rpc.rusersd not included ( the routine nonuser isn't anywhere )
SUBDIR=	analyze dumpfs edquota mkproto nfsstat \
		ping quot quotacheck quotaon repquota rpc.mountd rpc.rquotad \
		rpc.rstatd rpc.rwalld rpc.sprayd rpc.yppasswdd rpcinfo \
		rwall showmount spray yp ypserv


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

