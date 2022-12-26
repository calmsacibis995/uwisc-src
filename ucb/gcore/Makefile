#
#	Makefile for gcore
#
DESTDIR=
CFLAGS=	-O -DUW

PROGRAM=	gcore
SRCS=		gcore.c

${PROGRAM}:	${SRCS}
	${CC} ${CFLAGS} -o $@ $@.c ${LIBS}

install:
	install -g kmem -m 2755 -s ${PROGRAM} ${DESTDIR}/usr/ucb/${PROGRAM}

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
gcore: gcore.c /usr/include/stdio.h /usr/include/nlist.h
gcore: /usr/include/sys/param.h /usr/include/machine/machparam.h
gcore: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/dir.h
gcore: /usr/include/sys/user.h /usr/include/machine/pcb.h
gcore: /usr/include/sys/dmap.h /usr/include/sys/time.h /usr/include/time.h
gcore: /usr/include/sys/resource.h /usr/include/errno.h /usr/include/sys/proc.h
gcore: /usr/include/machine/pte.h /usr/include/sys/vm.h
gcore: /usr/include/sys/vmparam.h /usr/include/machine/vmparam.h
gcore: /usr/include/sys/vmmac.h /usr/include/sys/vmmeter.h
gcore: /usr/include/sys/vmsystm.h /usr/include/setjmp.h
gcore: gcore.c /usr/include/stdio.h /usr/include/nlist.h
gcore: /usr/include/sys/param.h /usr/include/machine/machparam.h
gcore: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/dir.h
gcore: /usr/include/sys/user.h /usr/include/machine/pcb.h
gcore: /usr/include/sys/dmap.h /usr/include/sys/time.h /usr/include/time.h
gcore: /usr/include/sys/resource.h /usr/include/errno.h /usr/include/sys/proc.h
gcore: /usr/include/machine/pte.h /usr/include/sys/vm.h
gcore: /usr/include/sys/vmparam.h /usr/include/machine/vmparam.h
gcore: /usr/include/sys/vmmac.h /usr/include/sys/vmmeter.h
gcore: /usr/include/sys/vmsystm.h /usr/include/setjmp.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
