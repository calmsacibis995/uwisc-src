#if defined(LIBC_RCS) && !defined(lint)
static char rcs_id[] =
	{"$Header: rcmd.c,v 1.4 86/11/05 14:15:47 tadl Exp $"};
#endif not lint
/*
 * RCS Info
 *	$Locker:  $
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)rcmd.c	5.11 (Berkeley) 5/6/86";
#endif LIBC_SCCS and not lint

#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>

#include <netdb.h>
#include <errno.h>

extern	errno;
char	*index(), *sprintf();

rcmd(ahost, rport, locuser, remuser, cmd, fd2p)
	char **ahost;
	u_short rport;
	char *locuser, *remuser, *cmd;
	int *fd2p;
{
	int s, timo = 1, pid, oldmask;
	struct sockaddr_in sin, sin2, from;
	char c;
	int lport = IPPORT_RESERVED - 1;
	struct hostent *hp;

	pid = getpid();
	hp = gethostbyname(*ahost);
	if (hp == 0) {
		fprintf(stderr, "%s: unknown host\n", *ahost);
		return (-1);
	}
	*ahost = hp->h_name;
	oldmask = sigblock(sigmask(SIGURG));
	for (;;) {
		s = rresvport(&lport);
		if (s < 0) {
			if (errno == EAGAIN)
				fprintf(stderr, "socket: All ports in use\n");
			else
				perror("rcmd: socket");
			sigsetmask(oldmask);
			return (-1);
		}
		fcntl(s, F_SETOWN, pid);
		sin.sin_family = hp->h_addrtype;
		bcopy(hp->h_addr_list[0], (caddr_t)&sin.sin_addr, hp->h_length);
		sin.sin_port = rport;
		if (connect(s, (caddr_t)&sin, sizeof (sin), 0) >= 0)
			break;
		(void) close(s);
		if (errno == EADDRINUSE) {
			lport--;
			continue;
		}
		if (errno == ECONNREFUSED && timo <= 16) {
			sleep(timo);
			timo *= 2;
			continue;
		}
		if (hp->h_addr_list[1] != NULL) {
			int oerrno = errno;

			fprintf(stderr,
			    "connect to address %s: ", inet_ntoa(sin.sin_addr));
			errno = oerrno;
			perror(0);
			hp->h_addr_list++;
			bcopy(hp->h_addr_list[0], (caddr_t)&sin.sin_addr,
			    hp->h_length);
			fprintf(stderr, "Trying %s...\n",
				inet_ntoa(sin.sin_addr));
			continue;
		}
		perror(hp->h_name);
		sigsetmask(oldmask);
		return (-1);
	}
	lport--;
	if (fd2p == 0) {
		write(s, "", 1);
		lport = 0;
	} else {
		char num[8];
		int s2 = rresvport(&lport), s3;
		int len = sizeof (from);

		if (s2 < 0)
			goto bad;
		listen(s2, 1);
		(void) sprintf(num, "%d", lport);
		if (write(s, num, strlen(num)+1) != strlen(num)+1) {
			perror("write: setting up stderr");
			(void) close(s2);
			goto bad;
		}
		s3 = accept(s2, &from, &len, 0);
		(void) close(s2);
		if (s3 < 0) {
			perror("accept");
			lport = 0;
			goto bad;
		}
		*fd2p = s3;
		from.sin_port = ntohs((u_short)from.sin_port);
		if (from.sin_family != AF_INET ||
		    from.sin_port >= IPPORT_RESERVED) {
			fprintf(stderr,
			    "socket: protocol failure in circuit setup.\n");
			goto bad2;
		}
	}
	(void) write(s, locuser, strlen(locuser)+1);
	(void) write(s, remuser, strlen(remuser)+1);
	(void) write(s, cmd, strlen(cmd)+1);
	if (read(s, &c, 1) != 1) {
		perror(*ahost);
		goto bad2;
	}
	if (c != 0) {
		while (read(s, &c, 1) == 1) {
			(void) write(2, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad2;
	}
	sigsetmask(oldmask);
	return (s);
bad2:
	if (lport)
		(void) close(*fd2p);
bad:
	(void) close(s);
	sigsetmask(oldmask);
	return (-1);
}

rresvport(alport)
	int *alport;
{
	struct sockaddr_in sin;
	int s;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return (-1);
	for (;;) {
		sin.sin_port = htons((u_short)*alport);
		if (bind(s, (caddr_t)&sin, sizeof (sin)) >= 0)
			return (s);
		if (errno != EADDRINUSE) {
			(void) close(s);
			return (-1);
		}
		(*alport)--;
		if (*alport == IPPORT_RESERVED/2) {
			(void) close(s);
			errno = EAGAIN;		/* close */
			return (-1);
		}
	}
}

ruserok(rhost, superuser, ruser, luser)
	char *rhost;
	int superuser;
	char *ruser, *luser;
{
	FILE *hostf;
	char fhost[MAXHOSTNAMELEN];
	int first = 1;
	register char *sp, *p;
	int baselen = -1;

	sp = rhost;
	p = fhost;
	while (*sp) {
		if (*sp == '.') {
			if (baselen == -1)
				baselen = sp - rhost;
			*p++ = *sp++;
		} else {
			*p++ = isupper(*sp) ? tolower(*sp++) : *sp++;
		}
	}
	*p = '\0';
	hostf = superuser ? (FILE *)0 : fopen("/etc/hosts.equiv", "r");
again:
	if (hostf) {
		if (!_validuser(hostf, fhost, luser, ruser, baselen)) {
			(void) fclose(hostf);
			return(0);
		}
		(void) fclose(hostf);
	}
	if (first == 1) {
		struct stat sbuf;
		struct passwd *pwd;
		char pbuf[MAXPATHLEN];

		first = 0;
		if ((pwd = getpwnam(luser)) == NULL)
			return(-1);
		(void)strcpy(pbuf, pwd->pw_dir);
		(void)strcat(pbuf, "/.rhosts");
		if ((hostf = fopen(pbuf, "r")) == NULL)
			return(-1);
		(void)fstat(fileno(hostf), &sbuf);
		if (sbuf.st_uid && sbuf.st_uid != pwd->pw_uid) {
			fclose(hostf);
			return(-1);
		}
		goto again;
	}
	return (-1);
}

_validuser(hostf, rhost, luser, ruser, baselen)
char *rhost, *luser, *ruser;
FILE *hostf;
int baselen;
{
	static char ldomain[MAXHOSTNAMELEN + 1];
	static char *domainp = NULL;
	static char ypdomain[256];
	register char *cp;
	char *user;
	char ahost[MAXHOSTNAMELEN];
	register char *p;
	int hostmatch = 0, usermatch = 0;

	if (getdomainname(ypdomain, sizeof(ypdomain)) < 0) {
		fprintf(stderr, "rcmd: getdomainname system call missing\n");
		exit(1);
	}

	while (fgets(ahost, sizeof (ahost), hostf)) {
		p = ahost;
		while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0') {
			*p = isupper(*p) ? tolower(*p) : *p;
			p++;
		}
		if (*p == ' ' || *p == '\t') {
			*p++ = '\0';
			while (*p == ' ' || *p == '\t')
				p++;
			user = p;
			while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0')
				p++;
		} else
			user = p;
		*p = '\0';

		/*
		 * If there is either no domain or the domains match check the host.
		 * Hosts can match (or not)  several different ways
		 * 	+ 			=> everybody is ok
		 * 	+@netgroup	=> everyone in netgroup is Ok
		 * 	-@netgroup	=> no one in netgroup is Ok
		 * 	-host		=> host is not ok
		 */

		if (ahost[0] == '+' && ahost[1] == 0) {
			hostmatch = 1;
		}
		else if (ahost[0] == '+' && ahost[1] == '@') {
			hostmatch = innetgr(ahost + 2, rhost, ruser, ypdomain);
		}
		else if (ahost[0] == '-' && ahost[1] == '@')  {
			if (innetgr(ahost + 2, rhost, ruser, ypdomain))
				break;
		}
		else {
			if (ahost[0] == '-') {
				if (_checkhost(rhost, ahost+1, baselen))
					break;
			}
			else {
				hostmatch = _checkhost(rhost, ahost, baselen);
			}
		}

		if (*user) {
			if (user[0] == '+' && user[1] == 0) {
				usermatch = 1;
			}
			else if (user[0] == '+' && user[1] == '@') {
				usermatch = innetgr(user+2, rhost, ruser, ypdomain);
			}
			else if (user[0] == '-' && user[1] == '@') {
				if (innetgr(user+2, rhost, ruser, ypdomain))
					break;
			}
			else if (user[0] == '-') {
				if (!strcmp(user+1, ruser))
					break;
			}
			else
				usermatch = !strcmp(user, ruser);
		}
		else
			usermatch = !strcmp(ruser, luser);

		if (hostmatch && usermatch) {
			return(0);
		}
	}
	return (-1);
}

_checkhost(rhost, lhost, len)
	char *rhost, *lhost;
	int len;
{
	static char ldomain[MAXHOSTNAMELEN + 1];
	static char *domainp = NULL;
	register char *cp;

	if (len == -1)
		return(!strcmp(rhost, lhost));
	if (strncmp(rhost, lhost, len))
		return(0);
	if (!strcmp(rhost, lhost))
		return(1);
	if (*(lhost + len) != '\0')
		return(0);
	if (!domainp) {
		if (gethostname(ldomain, sizeof(ldomain)) == -1) {
			domainp = (char *)1;
			return(0);
		}
		ldomain[MAXHOSTNAMELEN] = NULL;
		if ((domainp = index(ldomain, '.') + 1) == (char *)1)
			return(0);
		cp = domainp;
		while (*cp) {
			*cp = isupper(*cp) ? tolower(*cp) : *cp;
			cp++;
		}
	}
	if (domainp == (char *)1)
		return(0);
	return(!strcmp(domainp, rhost + len +1));
}
