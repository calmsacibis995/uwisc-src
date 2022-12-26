#
#	Makefile for portmap
#
DESTDIR=
CFLAGS=	-O -DUW -DVFS

PROGRAM=	portmap
SRCS=		portmap.c
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
portmap: portmap.c /usr/include/rpc/rpc.h /usr/include/rpc/types.h
portmap: /usr/include/sys/types.h /usr/include/netinet/in.h
portmap: /usr/include/rpc/xdr.h /usr/include/rpc/auth.h /usr/include/rpc/clnt.h
portmap: /usr/include/rpc/rpc_msg.h /usr/include/rpc/auth_unix.h
portmap: /usr/include/rpc/svc.h /usr/include/rpc/svc_auth.h
portmap: /usr/include/rpc/pmap_prot.h /usr/include/stdio.h /usr/include/netdb.h
portmap: /usr/include/sys/socket.h /usr/include/sys/time.h /usr/include/time.h
portmap: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
portmap: /usr/include/sys/ttydev.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
