#ifndef lint
/* @(#)spray.c	2.1 86/04/16 NFSSRC */
static  char sccsid[] = "@(#)spray.c 1.1 86/02/05 Copyr 1985 Sun Micro";
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <ctype.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <rpcsvc/spray.h>

#define DEFBYTES 100000		/* default numbers of bytes to send */
#define MAXPACKETLEN 1514

char *adrtostr();
char *host;
int adr;

main(argc, argv)
	char **argv;
{
	int err, cnt, i, rcved, lnth;
	int psec, bsec;
	int buf[SPRAYMAX/4];
	struct hostent *hp;
	struct sprayarr arr;	
	struct spraycumul cumul;
	
	if (argc < 2)
		usage();
	
	cnt = -1;
	lnth = SPRAYOVERHEAD;
	while (argc > 1) {
		if (argv[1][0] == '-') {
			switch(argv[1][1]) {
				case 'c':
					cnt = atoi(argv[2]);
					argc--;
					argv++;
					break;
				case 'l':
					lnth = atoi(argv[2]);
					argc--;
					argv++;
					break;
				default:
					usage();
			}
		}
		else {
			if (host)
				usage();
			else
				host = argv[1];
		}
		argc--;
		argv++;
	}
	if (host == NULL)
		usage();
	if (isdigit(host[0])) {
		adr = inet_addr(host);
		host = adrtostr(adr);
	}
	else {
		if ((hp = gethostbyname(host)) == NULL) {
			fprintf(stderr, "%s is unknown host name\n");
			exit(1);
		}
		adr = *((int *)hp->h_addr);
	}
	if (cnt == -1)
		cnt = DEFBYTES/lnth;
	if (lnth < SPRAYOVERHEAD)
		lnth = SPRAYOVERHEAD;
	else if (lnth >= SPRAYMAX)
		lnth = SPRAYMAX;
	if (lnth <= MAXPACKETLEN && lnth % 4 != 2)
		lnth = ((lnth+5)/4)*4 - 2;
	arr.lnth = lnth - SPRAYOVERHEAD;
	arr.data = buf;
	printf("sending %d packets of lnth %d to %s ...", cnt, lnth, host);
	fflush(stdout);
	
	if (err = mycallrpc(adr, SPRAYPROG, SPRAYVERS, SPRAYPROC_CLEAR,
	    xdr_void, NULL, xdr_void, NULL)) {
		fprintf(stderr, "SPRAYPROC_CLEAR ");
		clnt_perrno(err);
		fprintf(stderr, "\n");
		return;
	}
	for (i = 0; i < cnt; i++)
		callrpcnowait(adr, SPRAYPROG, SPRAYVERS, SPRAYPROC_SPRAY,
		    xdr_sprayarr, &arr, xdr_void, NULL);
	if (err = mycallrpc(adr, SPRAYPROG, SPRAYVERS, SPRAYPROC_GET,
	    xdr_void, NULL, xdr_spraycumul, &cumul)) {
		fprintf(stderr, "SPRAYPROC_GET ");
		fprintf(stderr, "%s ", host);
		clnt_perrno(err);
		fprintf(stderr, "\n");
		return;
	}
	if (cumul.counter < cnt)
		printf("\n\t%d packets (%.3f%%) dropped by %s\n",
		    cnt - cumul.counter,
		    100.0*(cnt - cumul.counter)/cnt, host);
	else
		printf("\n\tno packets dropped by %s\n", host);
	psec = (1000000.0 * cumul.counter)
	    / (1000000.0 * cumul.clock.tv_sec + cumul.clock.tv_usec);
	bsec = (lnth * 1000000.0 * cumul.counter)/
	    (1000000.0 * cumul.clock.tv_sec + cumul.clock.tv_usec);
	printf("\t%d packets/sec, %d bytes/sec\n", psec, bsec);
}

/* 
 * like callrpc, but with addr instead of host name
 */
mycallrpc(addr, prognum, versnum, procnum, inproc, in, outproc, out)
	xdrproc_t inproc, outproc;
	char *in, *out;
{
	struct sockaddr_in server_addr;
	enum clnt_stat clnt_stat;
	struct timeval timeout, tottimeout;

	static CLIENT *client;
	static int socket = RPC_ANYSOCK;
	static int oldprognum, oldversnum, valid;
	static int oldadr;

	if (valid && oldprognum == prognum && oldversnum == versnum
		&& adr == oldadr) {
		/* reuse old client */		
	}
	else {
		close(socket);
		socket = RPC_ANYSOCK;
		if (client) {
			clnt_destroy(client);
			client = NULL;
		}
		timeout.tv_usec = 0;
		timeout.tv_sec = 10;
		bcopy(&adr, &server_addr.sin_addr, sizeof(adr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port =  0;
		if ((client = clntudp_create(&server_addr, prognum,
		    versnum, timeout, &socket)) == NULL)
			return ((int) rpc_createerr.cf_stat);
		valid = 1;
		oldprognum = prognum;
		oldversnum = versnum;
		oldadr = adr;
	}
	tottimeout.tv_sec = 25;
	tottimeout.tv_usec = 0;
	clnt_stat = clnt_call(client, procnum, inproc, in,
	    outproc, out, tottimeout);
	/* 
	 * if call failed, empty cache
	 */
	if (clnt_stat != RPC_SUCCESS)
		valid = 0;
	return ((int) clnt_stat);
}

callrpcnowait(adr, prognum, versnum, procnum, inproc, in, outproc, out)
	xdrproc_t inproc, outproc;
	char *in, *out;
{
	struct sockaddr_in server_addr;
	enum clnt_stat clnt_stat;
	struct timeval timeout, tottimeout;

	static CLIENT *client;
	static int socket = RPC_ANYSOCK;
	static int oldprognum, oldversnum, valid;
	static int oldadr;

	if (valid && oldprognum == prognum && oldversnum == versnum
		&& oldadr == adr) {
		/* reuse old client */		
	}
	else {
		close(socket);
		socket = RPC_ANYSOCK;
		if (client) {
			clnt_destroy(client);
			client = NULL;
		}
		timeout.tv_usec = 0;
		timeout.tv_sec = 0;
		bcopy(&adr, &server_addr.sin_addr, sizeof(adr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port =  0;
		if ((client = clntudp_create(&server_addr, prognum,
		    versnum, timeout, &socket)) == NULL)
			return ((int) rpc_createerr.cf_stat);
		valid = 1;
		oldprognum = prognum;
		oldversnum = versnum;
		oldadr = adr;
	}
	tottimeout.tv_sec = 0;
	tottimeout.tv_usec = 0;
	clnt_stat = clnt_call(client, procnum, inproc, in,
	    outproc, out, tottimeout);
	/* 
	 * if call failed, empty cache
	 * since timeout is zero, normal return value is RPC_TIMEDOUT
	 */
	if (clnt_stat != RPC_SUCCESS && clnt_stat != RPC_TIMEDOUT)
		valid = 0;
	return ((int) clnt_stat);
}

usage()
{
	fprintf(stderr, "Usage: spray host [-c cnt] [-l lnth]\n");
	exit(1);
}

char *
adrtostr(adr)
int adr;
{
	struct hostent *hp;
	char buf[100];		/* hope this is long enough */
	
	hp = gethostbyaddr(&adr, sizeof(adr), AF_INET);
	if (hp == NULL) {
	    	sprintf(buf, "0x%x", adr);
		return buf;
	}
	else
		return hp->h_name;
}
