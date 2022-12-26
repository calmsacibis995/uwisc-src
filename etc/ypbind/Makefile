#
#	Makefile for ypbind
#
DESTDIR=
CFLAGS=	-O -DUW 

PROGRAM=	ypbind
SRCS=		ypbind.c
LIBS=

${PROGRAM}:	${SRCS}
	${CC} ${CFLAGS} -o $@ $@.c ${LIBS}

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
ypbind: ypbind.c /usr/include/stdio.h /usr/include/errno.h
ypbind: /usr/include/signal.h /usr/include/sys/time.h /usr/include/time.h
ypbind: /usr/include/sys/wait.h /usr/include/rpc/rpc.h /usr/include/rpc/types.h
ypbind: /usr/include/sys/types.h /usr/include/netinet/in.h
ypbind: /usr/include/rpc/xdr.h /usr/include/rpc/auth.h /usr/include/rpc/clnt.h
ypbind: /usr/include/rpc/rpc_msg.h /usr/include/rpc/auth_unix.h
ypbind: /usr/include/rpc/svc.h /usr/include/rpc/svc_auth.h
ypbind: /usr/include/sys/dir.h /usr/include/netdb.h /usr/include/sys/ioctl.h
ypbind: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
ypbind: /usr/include/sys/socket.h /usr/include/rpcsvc/yp_prot.h
ypbind: /usr/include/rpcsvc/ypv1_prot.h /usr/include/rpcsvc/ypclnt.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
