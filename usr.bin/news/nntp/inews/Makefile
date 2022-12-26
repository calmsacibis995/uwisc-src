#
# Makefile for pseudo-inews
#

SRCS	=	inews.c uname.c ../common/clientlib.c
OBJS	=	inews.o uname.o ../common/clientlib.o
HFILES	=	../common/response_codes.h ../common/conf.h

CFLAGS	= -O

IDESTDIR = $(DESTDIR)/usr/lib/news

inews:  $(OBJS) $(HFILES)
	$(CC) $(CFLAGS) $(OBJS) -o inews

uname.o: ../common/conf.h

../common/clientlib.o:
	${CC} ${CFLAGS} -c ../common/clientlib.c
	mv clientlib.o ../common/clientlib.o

install:
	cp inews ${IDESTDIR}/inews
	chmod 755 ${IDESTDIR}/inews

clean:
	rm -f ${OBJS} inews

distrib:
	rm -f ${OBJS} inews make.out
	rm -rf SCCS

lint:
	lint ${SRCS} ${HFILES}

tags:
	ctags $(CFILES) $(HFILES)
