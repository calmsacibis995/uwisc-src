#$Header: Makefile,v 1.1 86/08/26 21:47:30 root Exp $
#
#			-[Thu Feb 10 13:15:17 1983 by jkf]-

# Makefile for VLP

# files required:
#	vlp.l	- lisp program to read source and add troff control
#		characters.
#	vlpmacs - troff macros used by vlp.
#
# vlp and vlpmacs are put in DESTDIR
# If DESTDIR is changed, then the source (vlp.l) must be modified since
# it explicitly points to vlpmacs.

DESTDIR =
.DEFAULT:vlp

vlp:	vlp.l
	/usr/ucb/liszt -qar -o vlp vlp.l

install: vlp vlpmacs
	install vlp ${DESTDIR}/usr/ucb
	install -c -m 664 vlpmacs ${DESTDIR}/usr/lib

clean:
	rm -f vlp
