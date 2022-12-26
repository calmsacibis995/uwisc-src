#
# Makefile for NNTP server
#

SRVROBJ = main.o serve.o access.o active.o ahbs.o group.o help.o \
	ihave.o list.o misc.o netaux.o newgroups.o newnews.o nextlast.o \
	post.o parsit.o scandir.o slave.o subnet.o time.o version.o

SRVRSRC = main.c serve.c access.c active.c ahbs.c group.c help.c \
	ihave.c list.c misc.c netaux.c newgroups.c newnews.c nextlast.c \
 	post.c parsit.c scandir.c slave.c subnet.c time.c version.c

SRVRINC = common.h ../common/conf.h ../common/response_codes.h

SRCS	= ${SRVRSRC}

# -ldbm here if you've #define'ed DBM in ../common/conf.h
LIBS	=

CFLAGS	= -O

# Where nntpd is going to live

IDESTDIR	= $(DESTDIR)/etc

all:	nntpd

nntpd: ${SRVROBJ}
	cc ${CFLAGS} -o nntpd ${SRVROBJ} ${LIBS}

install: nntpd
	cp nntpd ${IDESTDIR}/nntpd
	chmod 711 ${IDESTDIR}/nntpd

lint:
	lint ${SRVRSRC}

clean:
	rm -f *.o nntpd make.out

distrib:
	rm -f *.o nntpd make.out
	rm -rf SCCS save

tags:	${SRVRSRC} ${SRVRINC}
	ctags ${SRVRSRC} ${SRVRINC}

# Dependencies

${SRVROBJ}:	${SRVRINC}
