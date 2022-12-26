#
# Makefile for NNTP server support programs
#

OBJS	=	mkgrdates.o

SRCS	=	mkgrdates.c

HFILES	=	../common/conf.h

CFLAGS= -O

# Where these support programs will live

IDESTDIR	= $(DESTDIR)/usr/lib/news

mkgrdates: mkgrdates.o
	cc ${CFLAGS} -o mkgrdates mkgrdates.o

install: mkgrdates
	cp mkgrdates ${IDESTDIR}/mkgrdates
	chmod 755 ${IDESTDIR}/mkgrdates

lint:
	lint mkgrdates.c

clean:
	rm -f *.o mkgrdates

distrib:
	rm -f *.o mkgrdates make.out
	rm -rf SCCS old

tags:	${SRCS} ${HFILES}
	ctags ${SRCS} ${HFILES}

${OBJS}:	${HFILES}
