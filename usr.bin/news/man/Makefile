#
# Makefile for news manual pages
#
# If you install news documentation in an appropriately structured
#	man tree (e.g. /usr/local/man) then you can do this better
#

DESTDIR =
MANDIR = /usr/man/mann

MANFILES = checknews.1 expire.8 getdate.3 inews.8 news.5 newsrc.5 postnews.1 \
	readnews.1 recnews.8 sendbatch.8 sendnews.8 uurec.8 vnews.1
NMANFILES = checknews.n expire.n getdate.n inews.n news.n newsrc.n postnews.n \
	readnews.n recnews.n sendbatch.n sendnews.n uurec.n vnews.n

.SUFFIXES: .n .1 .3 .5 .8

.1.n .3.n .5.n .8.n:
	cp $< $@

all: $(NMANFILES)

install: $(NMANFILES)
	-for i in *.1; do (install -c -m 0444 $$i $(DESTDIR)/usr/man/man1); done
	-for i in *.3; do (install -c -m 0444 $$i $(DESTDIR)/usr/man/man3); done
	-for i in *.5; do (install -c -m 0444 $$i $(DESTDIR)/usr/man/man5); done
	-for i in *.8; do (install -c -m 0444 $$i $(DESTDIR)/usr/man/man8); done
#	-for i in $(NMANFILES); do (install -c -m 0444 $$i $(DESTDIR)$(MANDIR)); done

clean:
	rm -f $(NMANFILES)
