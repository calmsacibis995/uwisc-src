#
#       Makefile     1.1     86/02/03
#	@(#)Makefile	2.1 86/04/14 NFSSRC
#

SRCS=	ypprot_err.c yp_bind.c yperr_string.c \
	yp_enum.c yp_match.c ypxdr.c \
	yp_all.c yp_master.c yp_order.c ypv1_xdr.c
OBJS=	ypprot_err.o yp_bind.o yperr_string.o \
	yp_enum.o yp_match.o ypxdr.o \
	yp_all.o yp_master.o yp_order.o ypv1_xdr.o
INC=	yp_prot.h ypclnt.h ypv1_prot.h

CFLAGS= -O
# CFLAGS= -g

.c.o:
	${CC} -p -c ${CFLAGS} $*.c
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

yplib yplib_p: ${OBJS}
	@echo "building profiled yplib"
	@cd profiled; ar cru ../yplib_p ${OBJS}
	@echo "building normal yplib"
	@ar cru yplib ${OBJS}

install:
	-mkdir ${DESTDIR}/usr/include/rpcsvc && \
		chown bin ${DESTDIR}/usr/include/rpcsvc && \
		chmod 755 ${DESTDIR}/usr/include/rpcsvc
	-for i in $(INC); do \
		(install -c -m 644 $$i ${DESTDIR}/usr/include/rpcsvc) done

tags: $(SRCS) $(KSRC) $(INC)
	ctags -tw $(SRCS) $(KSRC) $(INC)

ref: tags
	sed 's,	/.*,,' tags | \
	awk ' { printf("%-26s%-16s%s\n", $$1, $$2, $$3) }' > ref

lint:
	lint -bnuvx $(SRCS)

print:
	pr $(INC) $(SRCS) $(KSRC) | lpr -Pvp

clean:
	rm -f $(OBJS) yplib linted made profiled/*.o yplib_p

co:
	co ${COFLAGS} ${SRCS}

ci:
	ci ${CIFLAGS} ${SRCS}

