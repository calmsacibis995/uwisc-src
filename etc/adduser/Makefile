OBJS = main.o conf.o add.o getdata.o
OBJ2 = main.2.8.o conf.2.8.o add.2.8.o getdata.2.8.o
SRCS = main.c add.c getdata.c adduser.h
#
#	add -DYP for yeller pages
#
CFLAGS= -O -DUW -DNFS

main.o: main.c adduser.h
	cc ${CFLAGS} -DBSD4_2 -c main.c

conf.o: conf.c
	cc ${CFLAGS} -DBSD4_2 -c conf.c

add.o: add.c adduser.h
	cc ${CFLAGS} -DBSD4_2 -c add.c

getdata.o: getdata.c adduser.h
	cc ${CFLAGS} -DBSD4_2 -c getdata.c

adduser: ${OBJS}
	cc ${CFLAGS} -o adduser ${OBJS} -ldbm

clean:
	rm -f adduser adduser.2.8 core a.out *.o

print:
	lpr -p ${SRCS} 
