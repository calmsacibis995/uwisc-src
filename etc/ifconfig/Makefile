#
#	Makefile for ifconfig
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	ifconfig
SRCS=		ifconfig.c
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
ifconfig: ifconfig.c /usr/include/sys/types.h /usr/include/sys/socket.h
ifconfig: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
ifconfig: /usr/include/sys/ttydev.h /usr/include/net/if.h
ifconfig: /usr/include/net/if_arp.h /usr/include/netinet/in.h
ifconfig: /usr/include/netns/ns.h /usr/include/netns/ns_if.h
ifconfig: /usr/include/stdio.h /usr/include/errno.h /usr/include/ctype.h
ifconfig: /usr/include/netdb.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
