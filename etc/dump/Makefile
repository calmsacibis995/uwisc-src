#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.4 (Berkeley) 5/28/86
#
#	dump.h			header file
#	dumpitime.c		reads /etc/dumpdates
#	dumpmain.c		driver
#	dumpoptr.c		operator interface
#	dumptape.c		handles the mag tape and opening/closing
#	dumptraverse.c		traverses the file system
#	unctime.c		undo ctime
#
#	DEBUG			use local directory to find ddate and dumpdates
#	TDEBUG			trace out the process forking
#
PSRCS = \
	/usr/include/protocols/dumprestore.h dump.h \
	dumpmain.c dumptraverse.c dumptape.c dumpoptr.c dumpitime.c unctime.c

SRCS =  \
	dump.h dumpitime.c \
	dumpmain.c dumpoptr.c dumprmt.c dumprtape.c \
	dumptraverse.c unctime.c

OBJS = dumpitime.o dumpmain.o dumpoptr.o \
	dumptape.o dumptraverse.o unctime.o

ROBJS = dumpitime.o dumprmain.o dumpoptr.o \
	dumprmt.o dumprtape.o dumptraverse.o unctime.o

CFLAGS = -O -DUW -DVFS

all: dump rdump

rdump:	$(ROBJS)
	$(CC) $(CFLAGS) $(ROBJS) -o rdump

dump:	$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o dump

install: rdump dump
	install -s -m 6755 -g tty rdump $(DESTDIR)/etc
	install -s -m 2755 -g tty dump $(DESTDIR)/etc

dumprmain.c:
	ln -s dumpmain.c dumprmain.c
dumprmain.o: dumprmain.c
	${CC} -c -DRDUMP ${CFLAGS} dumprmain.c

dumprtape.c:
	ln -s dumptape.c dumprtape.c
dumprtape.o: dumprtape.c
	${CC} -c -DRDUMP ${CFLAGS} dumprtape.c

clean:
	rm -f *.o rdump dump errs dumprmain.c dumprtape.c

lint:
	lint $(DFLAGS) $(SRCS)

print:
	pr $(PSRCS) | lpr

depend:	${SRCS}
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
dump: dump.h /usr/include/stdio.h /usr/include/ctype.h /usr/include/sys/param.h
dump: /usr/include/machine/machparam.h /usr/include/signal.h
dump: /usr/include/sys/types.h /usr/include/sys/stat.h /usr/include/sys/time.h
dump: /usr/include/time.h /usr/include/ufs/fs.h /usr/include/sys/vnode.h
dump: /usr/include/ufs/inode.h /usr/include/protocols/dumprestore.h
dump: /usr/include/ufs/fsdir.h /usr/include/utmp.h /usr/include/signal.h
dump: /usr/include/fstab.h
dumpitime: dumpitime.c ./dump.h /usr/include/stdio.h /usr/include/ctype.h
dumpitime: /usr/include/sys/param.h /usr/include/machine/machparam.h
dumpitime: /usr/include/signal.h /usr/include/sys/types.h
dumpitime: /usr/include/sys/stat.h /usr/include/sys/time.h /usr/include/time.h
dumpitime: /usr/include/ufs/fs.h /usr/include/sys/vnode.h
dumpitime: /usr/include/ufs/inode.h /usr/include/protocols/dumprestore.h
dumpitime: /usr/include/ufs/fsdir.h /usr/include/utmp.h /usr/include/signal.h
dumpitime: /usr/include/fstab.h /usr/include/sys/file.h
dumpmain: dumpmain.c ./dump.h /usr/include/stdio.h /usr/include/ctype.h
dumpmain: /usr/include/sys/param.h /usr/include/machine/machparam.h
dumpmain: /usr/include/signal.h /usr/include/sys/types.h
dumpmain: /usr/include/sys/stat.h /usr/include/sys/time.h /usr/include/time.h
dumpmain: /usr/include/ufs/fs.h /usr/include/sys/vnode.h
dumpmain: /usr/include/ufs/inode.h /usr/include/protocols/dumprestore.h
dumpmain: /usr/include/ufs/fsdir.h /usr/include/utmp.h /usr/include/signal.h
dumpmain: /usr/include/fstab.h
dumpoptr: dumpoptr.c ./dump.h /usr/include/stdio.h /usr/include/ctype.h
dumpoptr: /usr/include/sys/param.h /usr/include/machine/machparam.h
dumpoptr: /usr/include/signal.h /usr/include/sys/types.h
dumpoptr: /usr/include/sys/stat.h /usr/include/sys/time.h /usr/include/time.h
dumpoptr: /usr/include/ufs/fs.h /usr/include/sys/vnode.h
dumpoptr: /usr/include/ufs/inode.h /usr/include/protocols/dumprestore.h
dumpoptr: /usr/include/ufs/fsdir.h /usr/include/utmp.h /usr/include/signal.h
dumpoptr: /usr/include/fstab.h
dumprmt: dumprmt.c /usr/include/sys/param.h /usr/include/machine/machparam.h
dumprmt: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/mtio.h
dumprmt: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
dumprmt: /usr/include/sys/ttydev.h /usr/include/sys/socket.h
dumprmt: /usr/include/sys/time.h /usr/include/time.h /usr/include/sys/vnode.h
dumprmt: /usr/include/ufs/inode.h /usr/include/netinet/in.h
dumprmt: /usr/include/stdio.h /usr/include/pwd.h /usr/include/netdb.h
dumprmt: /usr/include/protocols/dumprestore.h
dumprtape: dumprtape.c /usr/include/sys/file.h ./dump.h /usr/include/stdio.h
dumprtape: /usr/include/ctype.h /usr/include/sys/param.h
dumprtape: /usr/include/machine/machparam.h /usr/include/signal.h
dumprtape: /usr/include/sys/types.h /usr/include/sys/stat.h
dumprtape: /usr/include/sys/time.h /usr/include/time.h /usr/include/ufs/fs.h
dumprtape: /usr/include/sys/vnode.h /usr/include/ufs/inode.h
dumprtape: /usr/include/protocols/dumprestore.h /usr/include/ufs/fsdir.h
dumprtape: /usr/include/utmp.h /usr/include/signal.h /usr/include/fstab.h
dumptraverse: dumptraverse.c ./dump.h /usr/include/stdio.h /usr/include/ctype.h
dumptraverse: /usr/include/sys/param.h /usr/include/machine/machparam.h
dumptraverse: /usr/include/signal.h /usr/include/sys/types.h
dumptraverse: /usr/include/sys/stat.h /usr/include/sys/time.h
dumptraverse: /usr/include/time.h /usr/include/ufs/fs.h
dumptraverse: /usr/include/sys/vnode.h /usr/include/ufs/inode.h
dumptraverse: /usr/include/protocols/dumprestore.h /usr/include/ufs/fsdir.h
dumptraverse: /usr/include/utmp.h /usr/include/signal.h /usr/include/fstab.h
unctime: unctime.c /usr/include/sys/types.h /usr/include/sys/time.h
unctime: /usr/include/time.h /usr/include/stdio.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
