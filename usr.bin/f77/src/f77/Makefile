#
# Makefile for f77 shell, f77 compiler 4.2 BSD.
#
# University of Utah modification history:
#
# $Log:	Makefile,v $
# Revision 1.1  86/08/26  20:51:37  root
# Original BSD4.3 Source, as Distributed
# 
# Revision 1.2  84/08/05  23:31:35  donn
# Took out '-n' load option, added OFLAG to make DBX compiles easier.
# 
#

OFLAG         = -O
CFLAGS	      = -DHERE=VAX -DTARGET=VAX -DFAMILY=PCC -DUCBVAXASM -DUCBPASS2 \
		$(OFLAG) -I../../include

DEST	      = $(DESTDIR)/usr/bin

EXTHDRS	      = ../../include/defines.h \
		../../include/machdefs.h \
		../../include/version.h \
		/usr/include/ctype.h \
		/usr/include/errno.h \
		/usr/include/signal.h \
		/usr/include/stdio.h \
		/usr/include/sys/stat.h \
		/usr/include/sys/types.h

HDRS	      = drivedefs.h

LDFLAGS	      =

LIBS	      =

LINKER	      = cc

MAKEFILE      = Makefile

OBJS	      = f77.o

PRINT	      = pr

PROGRAM	      = f77

SRCS	      = f77.c

all:		$(PROGRAM)

$(PROGRAM):     $(OBJS) $(LIBS)
		@echo -n "Loading $(PROGRAM) ... "
		@$(LINKER) $(OFLAG) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
		@echo "done"

clean:;		@rm -f $(OBJS)

depend:;	@mkmf -f $(MAKEFILE) PROGRAM=$(PROGRAM) DEST=$(DEST)

index:;		@ctags -wx $(HDRS) $(SRCS)

install:	$(PROGRAM)
		@echo Installing $(PROGRAM) in $(DEST)
		@install -s $(PROGRAM) $(DEST)

print:;		@$(PRINT) $(HDRS) $(SRCS)

program:        $(PROGRAM)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:		$(DEST)/$(PROGRAM)

$(DEST)/$(PROGRAM): $(SRCS) $(LIBS) $(HDRS) $(EXTHDRS)
		@make -f $(MAKEFILE) DEST=$(DEST) install
###
f77.o: /usr/include/stdio.h /usr/include/ctype.h ../../include/defines.h \
	../../include/machdefs.h drivedefs.h ../../include/version.h \
	/usr/include/signal.h /usr/include/errno.h /usr/include/sys/types.h \
	/usr/include/sys/stat.h
