CFLAGS = # -O -s 

vf: vf.c
	cc ${CFLAGS} vf.c -o vf

install: vf fixfiles files.perms
	install -s vf ${DESTDIR}/etc/vf
#	install fixfiles ${DESTDIR}/etc/fixfiles
#	install files.perms ${DESTDIR}/etc/files.perms

fixfiles:;

files.perms:;
