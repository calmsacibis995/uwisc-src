#
# @(#)Makefile 1.1 86/02/05 SMI;
# @(#)Makefile	2.1 86/04/14 NFSSRC
#
DESTDIR=
CFLAGS=-O
TAGSFILE=tags

OBJS= etherxdr.o mountxdr.o rquotaxdr.o rstatxdr.o rusersxdr.o rwallxdr.o \
      sprayxdr.o util.o yppasswdxdr.o
HDRS= ether.h mount.h rwall.h rquota.h rstat.h rusers.h spray.h yppasswd.h

librpcsvc.a: $(OBJS)
	ar cr librpcsvc.a $(OBJS)

install:
	install -c -m 644 librpcsvc.a $(DESTDIR)/usr/lib
	ranlib ${DESTDIR}/usr/lib/librpcsvc.a
	for i in *.h; do \
		(install -c -m 644 $$i ${DESTDIR}/usr/include/rpcsvc) done
tags:
	cwd=`pwd`; \
	for i in ${SRCS}; do \
		ctags -a -f ${TAGSFILE} $$cwd/$$i; \
	done
clean:
	rm -f *.o librpcsvc.a
