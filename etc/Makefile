#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.14 (Berkeley) 5/16/86
#
DESTDIR=
CFLAGS=	-O 

# the following programs have been moved into usr.etc
#	dcheck dumpfs edquota icheck mkproto ncheck ping 
#	quot quotacheck quotaon repquota savecore tunefs

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR=	XNSrouted ac accton arff arp bad144 badsect biod catman chown clri \
		comsat config cron dcheck diskpart dmesg dump dump.4.1 \
		fastboot fasthalt fingerd flcopy fsck fsirand ftpd \
		gettable getty halt htable icheck ifconfig implog inetd init \
		kgmon mkfs mkhosts mklost+found mknod mkpasswd mount \
		named ncheck newfs nfsd portmap pstat \
		reboot renice restore rexecd rlogind rmt rmuser \
		route routed rpcd rshd rwhod rxformat sa shutdown slattach \
		swapon syslogd talkd telnetd termcap tftpd timed trpt trsp tunefs \
		umount update vf vipw ypbind

all:	${SUBDIR} 

${SUBDIR}: FRC
	cd $@; make ${MFLAGS} 

install:
	-for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); done

clean:
	-rm -f a.out core *.s *.o
	-for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean); done

FRC:

depend:
	for i in ${SUBDIR}; do ( cd $$i; make ${MFLAGS} depend); done

tags:
	for i in ${SUBDIR}; do ( cd $$i; make ${MFLAGS} tags); done
