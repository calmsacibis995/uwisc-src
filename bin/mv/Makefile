#
#	Makefile for /bin/mv
#
DESTDIR=
CFLAGS=	-O -DUW

mv:
	${CC} ${CFLAGS} -o $@ $@.c

install:
	rm -f ${DESTDIR}/bin/mv; cp mv ${DESTDIR}/bin/mv; rm -f mv; \
		strip ${DESTDIR}/bin/mv; chmod 755 ${DESTDIR}/bin/mv

tags:
	ctags -tdw *.c

clean:
	rm -f a.out core *.s *.o made


depend: 
	    cc -M ${INCPATH} mv.c | sed 's/\.o//' | \
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
mv: mv.c /usr/include/sys/param.h /usr/include/machine/machparam.h
mv: /usr/include/signal.h /usr/include/sys/types.h /usr/include/sys/stat.h
mv: /usr/include/sys/time.h /usr/include/time.h /usr/include/stdio.h
mv: /usr/include/sys/dir.h /usr/include/errno.h /usr/include/signal.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
