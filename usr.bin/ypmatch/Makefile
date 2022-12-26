#
#	Makefile for ypmatch
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	ypmatch
SRCS=		ypmatch.c
LIBS=

${PROGRAM}:	${SRCS}
	${CC} ${CFLAGS} -o $@ $@.c ${LIBS}

install:
	install -s ${PROGRAM} ${DESTDIR}/usr/bin/${PROGRAM}

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
ypmatch: ypmatch.c /usr/include/stdio.h /usr/include/rpc/rpc.h
ypmatch: /usr/include/rpc/types.h /usr/include/sys/types.h
ypmatch: /usr/include/netinet/in.h /usr/include/rpc/xdr.h
ypmatch: /usr/include/rpc/auth.h /usr/include/rpc/clnt.h
ypmatch: /usr/include/rpc/rpc_msg.h /usr/include/rpc/auth_unix.h
ypmatch: /usr/include/rpc/svc.h /usr/include/rpc/svc_auth.h
ypmatch: /usr/include/netdb.h /usr/include/sys/time.h /usr/include/time.h
ypmatch: /usr/include/sys/socket.h /usr/include/rpcsvc/yp_prot.h
ypmatch: /usr/include/rpcsvc/ypclnt.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
