#
#	Makefile for dcheck
#
DESTDIR=
CFLAGS=	-O -DUW -DVFS

PROGRAM=	dcheck
SRCS=		dcheck.c
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
dcheck: dcheck.c /usr/include/sys/param.h /usr/include/machine/machparam.h
dcheck: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/dir.h
dcheck: /usr/include/stdio.h
dcheck: dcheck.c /usr/include/sys/param.h /usr/include/machine/machparam.h
dcheck: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/dir.h
dcheck: /usr/include/stdio.h
dcheck: dcheck.c /usr/include/sys/param.h /usr/include/machine/machparam.h
dcheck: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/dir.h
dcheck: /usr/include/stdio.h
dcheck: dcheck.c /usr/include/sys/param.h /usr/include/machine/machparam.h
dcheck: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/dir.h
dcheck: /usr/include/stdio.h
dcheck: dcheck.c /usr/include/sys/param.h /usr/include/machine/machparam.h
dcheck: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/dir.h
dcheck: /usr/include/stdio.h

dcheck: dcheck.c /usr/include/sys/param.h /usr/include/machine/machparam.h
dcheck: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/dir.h
dcheck: /usr/include/stdio.h
dcheck: dcheck.c /usr/include/sys/param.h /usr/include/machine/machparam.h
dcheck: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/dir.h
dcheck: /usr/include/stdio.h
dcheck: dcheck.c /usr/include/sys/param.h /usr/include/machine/machparam.h
dcheck: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/vnode.h
dcheck: /usr/include/ufs/inode.h /usr/include/ufs/fs.h /usr/include/stdio.h
dcheck: dcheck.c /usr/include/sys/param.h /usr/include/machine/machparam.h
dcheck: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/vnode.h
dcheck: /usr/include/ufs/inode.h /usr/include/ufs/fs.h /usr/include/ufs/fsdir.h
dcheck: /usr/include/stdio.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
