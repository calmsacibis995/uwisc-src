#
#	Makefile for /bin/cp
#
DESTDIR=
CFLAGS=	-O -DUW

cp:
	${CC} ${CFLAGS} -o $@ $@.c

install:
	install -s cp ${DESTDIR}/bin/newcp; \
		mv ${DESTDIR}/bin/newcp ${DESTDIR}/bin/cp;

tags:
	ctags -tdw *.c

clean:
	rm -f a.out core *.s *.o made


depend: 
	    cc -M ${INCPATH} cp.c | sed 's/\.o//' | \
	    awk ' { if ($$1 != prev) \
		{ if (rec != "") print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		else rec = rec " " $$2 } } \
		END { print rec } ' >> makedep
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
cp: cp.c /usr/include/stdio.h /usr/include/sys/param.h
cp: /usr/include/machine/machparam.h /usr/include/signal.h
cp: /usr/include/sys/types.h /usr/include/sys/stat.h /usr/include/sys/dir.h
cp: /usr/include/sys/time.h /usr/include/time.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
