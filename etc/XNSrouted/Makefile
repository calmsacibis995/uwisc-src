#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.5 (Berkeley) 9/17/85";
#
#	Makefile for Xerox Routing Information Protocol
#	Daemon
#
ALL=	XNSrouted
SRCS=	af.c if.c input.c main.c output.c startup.c tables.c timer.c \
	trace.c
OBJS=	af.o if.o input.o main.o output.o startup.o tables.o timer.o \
	trace.o
CFLAGS=  -O
C2=	/lib/c2
ASMSCRIPT= /sys/machine/asm.sed
DESTDIR=
LDFLAGS=

all:	${ALL}

XNSrouted: ${OBJS}
	${CC} -g -o XNSrouted ${LDFLAGS} ${OBJS}

install: XNSrouted
	install -s XNSrouted ${DESTDIR}/etc

clean:
	rm -f XNSrouted *.o core a.out t.c linterrs typescript errs

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
af: af.c ./defs.h /usr/include/sys/types.h /usr/include/sys/socket.h
af: /usr/include/net/route.h /usr/include/netns/ns.h /usr/include/netns/idp.h
af: /usr/include/stdio.h /usr/include/syslog.h ./protocol.h ./trace.h
af: ./interface.h ./table.h ./af.h
if: if.c ./defs.h /usr/include/sys/types.h /usr/include/sys/socket.h
if: /usr/include/net/route.h /usr/include/netns/ns.h /usr/include/netns/idp.h
if: /usr/include/stdio.h /usr/include/syslog.h ./protocol.h ./trace.h
if: ./interface.h ./table.h ./af.h
input: input.c ./defs.h /usr/include/sys/types.h /usr/include/sys/socket.h
input: /usr/include/net/route.h /usr/include/netns/ns.h
input: /usr/include/netns/idp.h /usr/include/stdio.h /usr/include/syslog.h
input: ./protocol.h ./trace.h ./interface.h ./table.h ./af.h
main: main.c ./defs.h /usr/include/sys/types.h /usr/include/sys/socket.h
main: /usr/include/net/route.h /usr/include/netns/ns.h /usr/include/netns/idp.h
main: /usr/include/stdio.h /usr/include/syslog.h ./protocol.h ./trace.h
main: ./interface.h ./table.h ./af.h /usr/include/sys/ioctl.h
main: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
main: /usr/include/sys/time.h /usr/include/time.h /usr/include/net/if.h
main: /usr/include/net/if_arp.h /usr/include/errno.h /usr/include/nlist.h
main: /usr/include/signal.h
output: output.c ./defs.h /usr/include/sys/types.h /usr/include/sys/socket.h
output: /usr/include/net/route.h /usr/include/netns/ns.h
output: /usr/include/netns/idp.h /usr/include/stdio.h /usr/include/syslog.h
output: ./protocol.h ./trace.h ./interface.h ./table.h ./af.h
startup: startup.c ./defs.h /usr/include/sys/types.h /usr/include/sys/socket.h
startup: /usr/include/net/route.h /usr/include/netns/ns.h
startup: /usr/include/netns/idp.h /usr/include/stdio.h /usr/include/syslog.h
startup: ./protocol.h ./trace.h ./interface.h ./table.h ./af.h
startup: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
startup: /usr/include/sys/ttydev.h /usr/include/net/if.h
startup: /usr/include/net/if_arp.h /usr/include/nlist.h /usr/include/syslog.h
tables: tables.c ./defs.h /usr/include/sys/types.h /usr/include/sys/socket.h
tables: /usr/include/net/route.h /usr/include/netns/ns.h
tables: /usr/include/netns/idp.h /usr/include/stdio.h /usr/include/syslog.h
tables: ./protocol.h ./trace.h ./interface.h ./table.h ./af.h
tables: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
tables: /usr/include/sys/ttydev.h /usr/include/errno.h
timer: timer.c ./defs.h /usr/include/sys/types.h /usr/include/sys/socket.h
timer: /usr/include/net/route.h /usr/include/netns/ns.h
timer: /usr/include/netns/idp.h /usr/include/stdio.h /usr/include/syslog.h
timer: ./protocol.h ./trace.h ./interface.h ./table.h ./af.h
trace: trace.c ./defs.h /usr/include/sys/types.h /usr/include/sys/socket.h
trace: /usr/include/net/route.h /usr/include/netns/ns.h
trace: /usr/include/netns/idp.h /usr/include/stdio.h /usr/include/syslog.h
trace: ./protocol.h ./trace.h ./interface.h ./table.h ./af.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
