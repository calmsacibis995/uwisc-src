#
# 	Makefile	1.1	85/05/30
#
DESTDIR=
#ifdef	KERNEL
DISTUSER=distuser
DISTKERN=distkern
#endif

SRCS=	auth_none.c auth_unix.c authunix_prot.c \
	clnt_perror.c clnt_raw.c clnt_simple.c clnt_tcp.c clnt_udp.c \
	pmap_clnt.c pmap_getmaps.c pmap_getport.c pmap_prot.c pmap_rmt.c rpc_prot.c \
	svc.c svc_auth.c svc_auth_unix.c svc_raw.c svc_simple.c \
	svc_tcp.c svc_udp.c xdr.c xdr_array.c xdr_float.c xdr_mem.c xdr_rec.c xdr_reference.c xdr_stdio.c

#ifdef KERNEL
KSRCS=	auth_kern.c clnt_kudp.c subr_kudp.c svc_kudp.c\
	xdr_mbuf.c
KALL=	auth.h auth_unix.h clnt.h rpc_msg.h svc.h svc_auth.h\
	types.h xdr.h auth_none.c auth_unix.c authunix_prot.c\
	rpc_prot.c svc.c svc_auth.c svc_auth_unix.c xdr.c xdr_array.c xdr_float.c xdr_mem.c xdr_reference.c \
	$(KSRCS)

#endif
OBJS=	auth_none.o auth_unix.o authunix_prot.o clnt_perror.o clnt_raw.o\
	clnt_simple.o clnt_tcp.o clnt_udp.o \
	pmap_clnt.o pmap_getmaps.o pmap_getport.o pmap_prot.o pmap_rmt.o rpc_prot.o \
	svc.o svc_auth.o svc_auth_unix.o svc_raw.o svc_simple.o \
	svc_tcp.o svc_udp.o xdr.o xdr_array.o xdr_float.o xdr_mem.o xdr_rec.o xdr_reference.o xdr_stdio.o
INC=	auth.h auth_unix.h clnt.h pmap_clnt.h\
	pmap_prot.h rpc.h rpc_msg.h svc.h svc_auth.h types.h xdr.h

CFLAGS= -O

.c.o:
	${CC} -p -c ${CFLAGS} $*.c
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

rpclib rpclib_p: ${OBJS}
	@echo "building profiled rpclib"
	@cd profiled; ar cru ../rpclib_p ${OBJS}
	@echo "building normal rpclib"
	@ar cru rpclib ${OBJS}

tags: $(SRCS) $(KSRCS) $(INC)
	ctags -tw $(SRCS) $(KSRCS) $(INC)

ref: tags
	sed 's,	/.*,,' tags | \
	awk ' { printf("%-26s%-16s%s\n", $$1, $$2, $$3) }' > ref

lint:
	lint -bnuvx $(SRCS)

print:
	pr $(INC) $(SRCS) $(KSRCS) | lpr -Pvp

clean:
	rm -f $(OBJS) librpc.a linted made profiled/*.o

co:
	co ${COFLAGS} ${SRCS}

ci:
	ci ${CIFLAGS} ${SRCS}

#ifdef KERNEL
distuser:
	-mkdir ${DISTUSER}
	@for i in $(SRCS) $(INC) ; do \
	    echo $$i; \
	    cp disclaimer ${DISTUSER}/$$i; \
	    chmod +w ${DISTUSER}/$$i; \
	    unifdef.sh -UKERNEL $$i\
		| sed -e "s,./. <> ./,," >> ${DISTUSER}/$$i;\
	done
	@unifdef.sh -UKERNEL Makefile >> ${DISTUSER}/Makefile
	
distkern:
	-mkdir ${DISTKERN}
	@for i in $(KALL) ; do \
	    echo $$i; unifdef.sh -DKERNEL $$i > ${DISTKERN}/$$i; done
#endif

depend:
	@-grep '^#include' $(SRCS) | grep -v '<' | grep -v '../' | \
	sed 's/:[^"]*"\([^"]*\)".*/: \1/' | sed 's/\.[cs]:/.o:/' | \
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' >> makedep
	@echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	@echo '$$r makedep' >>eddep
	@echo 'w' >>eddep
	@cp Makefile makefile.bak
	@ed - Makefile < eddep
	@rm eddep makedep makefile.bak



# DO NOT DELETE THIS LINE

clnt_raw.o: types.h xdr.h auth.h clnt.h rpc_msg.h
clnt_tcp.o: types.h xdr.h auth.h clnt.h rpc_msg.h pmap_clnt.h
clnt_udp.o: types.h xdr.h auth.h clnt.h rpc_msg.h pmap_clnt.h
pmap_clnt.o: types.h xdr.h auth.h clnt.h rpc_msg.h pmap_prot.h pmap_clnt.h
pmap_getmaps.o: types.h xdr.h auth.h clnt.h rpc_msg.h pmap_prot.h pmap_clnt.h
pmap_getport.o: types.h xdr.h auth.h clnt.h rpc_msg.h pmap_prot.h pmap_clnt.h
pmap_prot.o: types.h xdr.h pmap_prot.h
pmap_rmt.o: types.h xdr.h auth.h clnt.h rpc_msg.h pmap_prot.h pmap_clnt.h
svc_raw.o: types.h xdr.h auth.h clnt.h rpc_msg.h svc.h
svc_tcp.o: types.h xdr.h auth.h clnt.h rpc_msg.h svc.h
svc_udp.o: types.h xdr.h auth.h clnt.h rpc_msg.h svc.h
xdr_rec.o: types.h xdr.h
xdr_stdio.o: types.h xdr.h
