# Description file for the Make command
# Makefile 4.4 85/04/16

VPATH=.:/usr/src/bin/make
OBJECTS=ident.o main.o doname.o misc.o files.o dosys.o gram.o 
LIBES= 
LINT=	lint -ps
CFLAGS=	-O -DASCARCH -I. -I/usr/src/bin/make 

all:	newmake

newmake:	${OBJECTS}
	${CC} -o newmake ${CFLAGS} ${OBJECTS} ${LIBES}

${OBJECTS}:  defs

clean:
	-rm -f *.o gram.c newmake a.out errs

install:
	install -s newmake ${DESTDIR}/bin/make

lint :  dosys.c doname.c files.c main.c misc.c ident.c gram.c
	${LINT} dosys.c doname.c files.c main.c misc.c ident.c gram.c
	rm -f gram.c
