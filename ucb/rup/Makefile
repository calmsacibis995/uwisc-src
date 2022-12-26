#
#	Makefile for rup
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	rup
SRCS=		rup.c
LIBS=		-lrpcsvc

${PROGRAM}:	${SRCS}
	${CC} ${CFLAGS} -o $@ $@.c ${LIBS}

install:
	install -s ${PROGRAM} ${DESTDIR}/usr/ucb/${PROGRAM}

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
rup: rup.c /usr/include/stdio.h /usr/include/netdb.h /usr/include/sys/param.h
rup: /usr/include/machine/machparam.h /usr/include/signal.h
rup: /usr/include/sys/types.h /usr/include/sys/stat.h /usr/include/sys/time.h
rup: /usr/include/time.h /usr/include/sys/socket.h /usr/include/rpcsvc/rstat.h
rup: /usr/include/sys/dk.h /usr/include/rpc/rpc.h /usr/include/rpc/types.h
rup: /usr/include/netinet/in.h /usr/include/rpc/xdr.h /usr/include/rpc/auth.h
rup: /usr/include/rpc/clnt.h /usr/include/rpc/rpc_msg.h
rup: /usr/include/rpc/auth_unix.h /usr/include/rpc/svc.h
rup: /usr/include/rpc/svc_auth.h /usr/include/rpc/pmap_clnt.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
