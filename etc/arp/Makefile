#
#	Makefile for arp
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	arp
SRCS=		arp.c
LIBS=		-lm

${PROGRAM}:	${SRCS}
	${CC} ${CFLAGS} -o $@ $@.c ${LIBS}

install:
	install -g kmem -m 2755 -s ${PROGRAM} ${DESTDIR}/etc/${PROGRAM}

tags:
	ctags -tdw *.c 

clean:
	rm -f ${PROGRAM} a.out core *.s *.o made


depend: 
	for i in ${SRCS}; do \
	    cc -M ${INCPATH} ${CFLAGS} $$i | sed 's/\.o//' | \
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
arp: arp.c /usr/include/stdio.h /usr/include/sys/types.h
arp: /usr/include/sys/socket.h /usr/include/netinet/in.h
arp: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
arp: /usr/include/sys/ttydev.h /usr/include/errno.h /usr/include/netdb.h
arp: /usr/include/nlist.h /usr/include/net/if.h /usr/include/net/if_arp.h
arp: /usr/include/netinet/if_ether.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
