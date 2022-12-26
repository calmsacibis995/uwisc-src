
shlock: shlock.c
	cc -O -o shlock shlock.c

install: shlock
	cp shlock $(DESTDIR)/usr/lib/news/shlock
	/etc/chown news.news $(DESTDIR)/usr/lib/news/shlock

clean:
	rm -f shlock *.o

#### above is so local stuff works
#### below gets added to news/src/Makefile
NNTPSRC = nntpxmit.c rfc822.c remote.c get_tcp_conn.c getdate.y llist.c
NNTPOBJ = nntpxmit.o rfc822.o remote.o get_tcp_conn.o getdate.o llist.o

get_tcp_conn.o: get_tcp_conn.c get_tcp_conn.h
 
remote.o: remote.c response_codes.h nntpxmit.h
 
nntpxmit.o: nntpxmit.c nntpxmit.h get_tcp_conn.h header.h

nntpxmit: $(NNTPOBJ)
	$(CC) $(LDFLAGS) $(NNTPOBJ) -o nntpxmit
