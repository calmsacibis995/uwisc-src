#
#	Makefile for rpcd
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	rpcd
SRCS=		rpcd.c
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
rpcd: rpcd.c /usr/include/rpc/rpc.h /usr/include/rpc/types.h
rpcd: /usr/include/sys/types.h /usr/include/netinet/in.h /usr/include/rpc/xdr.h
rpcd: /usr/include/rpc/auth.h /usr/include/rpc/clnt.h
rpcd: /usr/include/rpc/rpc_msg.h /usr/include/rpc/auth_unix.h
rpcd: /usr/include/rpc/svc.h /usr/include/rpc/svc_auth.h
rpcd: /usr/include/rpc/pmap_prot.h /usr/include/sys/ioctl.h
rpcd: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
rpcd: /usr/include/sys/socket.h /usr/include/sys/file.h /usr/include/sys/wait.h
rpcd: /usr/include/arpa/inet.h /usr/include/errno.h /usr/include/stdio.h
rpcd: /usr/include/signal.h /usr/include/netdb.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
