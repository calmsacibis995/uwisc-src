/*
 * This software is Copyright (c) 1986 by Rick Adams.
 *
 * Permission is hereby granted to copy, reproduce, redistribute or
 * otherwise use this software as long as: there is no monetary
 * profit gained specifically from the use or reproduction or this
 * software, it is not sold, rented, traded or otherwise marketed, and
 * this copyright notice is included prominently in any copy
 * made.
 *
 * The author make no claims as to the fitness or correctness of
 * this software for any use whatsoever, and it is provided as is. 
 * Any use of this software is at the user's own risk.
 *
 * inews - insert, receive, and transmit news articles.
 *
 */

#ifdef SCCSID
static char	*SccsId = "@(#)inews.c	2.73	12/29/86";
#endif /* SCCSID */

#include "iparams.h"
#include <errno.h>

#ifdef BSD4_2
# include <sys/dir.h>
# include <sys/file.h>
#else	/* !BSD4_2 */
# include "ndir.h"
# ifdef USG
# include <fcntl.h>
# endif /* USG */
# ifdef LOCKF
# include <unistd.h>
# endif /* LOCKF */
#endif /* !BSD4_2 */
/* local defines for inews */

#define OPTION	0	/* pick up an option string */
#define STRING	1	/* pick up a string of arguments */

#define UNKNOWN 0001	/* possible modes for news program */
#define UNPROC	0002	/* Unprocessed input */
#define PROC	0004	/* Processed input */
#define	CONTROL	0010	/* Control Message */
#define	CREATENG 0020	/* Create a new newsgroup */

char	forgedname[NAMELEN];	/* A user specified -f option. */
int spool_news = 0;
extern char histline[];
/* Fake sys line in case they forget their own system */
struct srec dummy_srec = { "MEMEME", "", "all", "", "" };

char *Progname = "inews";	/* used by xerror to identify failing program */

struct {			/* options table. */
	char	optlet;		/* option character. */
	char	filchar;	/* if to pickup string, fill character. */
	int	flag;		/* TRUE if have seen this opt. */
	int	oldmode;	/* OR of legal input modes. */
	int	newmode;	/* output mode. */
	char	*buf;		/* string buffer */
} *optpt, options[] = { /*
optlet	filchar		flag	oldmode	newmode		buf	*/
't',	' ',		FALSE,	UNPROC,	UNKNOWN,	header.title,
'n',	NGDELIM,	FALSE,	UNPROC,	UNKNOWN,	header.nbuf,
'd',	'\0',		FALSE,	UNPROC,	UNKNOWN,	header.distribution,
'e',	' ',		FALSE,	UNPROC,	UNKNOWN,	header.expdate,
'p',	'\0',		FALSE,	UNKNOWN|PROC,	PROC,	filename,
'f',	'\0',		FALSE,	UNPROC,	UNKNOWN,	forgedname,
'F',	' ',		FALSE,	UNPROC,	UNKNOWN,	header.followid,
'c',	' ',		FALSE,	UNKNOWN,UNKNOWN,	header.ctlmsg,
'C',	' ',		FALSE,	UNKNOWN,CREATENG,	header.ctlmsg,
#define hflag	options[9].flag
'h',	'\0',		FALSE,	UNPROC,	UNKNOWN,	filename,
#define oflag	options[10].flag
'o',	'\0',		FALSE,	UNPROC, UNKNOWN,	header.organization,
#define Mflag	options[11].flag
'M',	'\0',		FALSE,	UNPROC, UNKNOWN,	filename,
'a',	'\0',		FALSE,	UNPROC, UNKNOWN,	header.approved,
'U',	'\0',		FALSE,	PROC, PROC,		filename,
'S',	'\0',		FALSE,	UNKNOWN|PROC, 	UNPROC,	filename,
'x',	'\0',		FALSE,	UNPROC, UNKNOWN,	not_here,
'r',	'\0',		FALSE,	UNPROC, UNKNOWN,	header.replyto,
'\0',	'\0',		0,	0,	0,		(char *)NULL
};

FILE *mailhdr();
extern int errno;

struct timeb Now;

/*
 *	Authors:
 *		Matt Glickman	glickman@ucbarpa.Berkeley.ARPA
 *		Mark Horton	mark@cbosgd.UUCP
 *		Stephen Daniels	swd@mcnc.UUCP
 *		Tom Truscott	trt@duke.UUCP
 *		Rick Adams	rick@seismo.CSS.GOV
 *	IHCC version adapted by:
 *		Larry Marek	larry@ihuxf.UUCP
 */
main(argc, argv)
int	argc;
register char **argv;
{
	int	state;		/* which type of argument to pick up	*/
	int	tlen, len;	/* temps for string processing routine	*/
	register char *ptr;	/* pointer to rest of buffer		*/
	int	filchar;	/* fill character (state = STRING)	*/
	char	*user = NULL, *home = NULL;	/* environment temps	*/
	struct passwd	*pw;	/* struct for pw lookup			*/
	struct group	*gp;	/* struct for group lookup		*/
	register int	i;
	FILE	*mfd;		/* mail file file-descriptor		*/
	char	cbuf[BUFLEN];	/* command buffer			*/

	/* uuxqt doesn't close all it's files */
	for (i = 3; !close(i); i++)
		;
	/* set up defaults and initialize. */
	mode = UNKNOWN;
	infp = stdin;
	pathinit();
	savmask = umask(N_UMASK);	/* set up mask */
	ptr = rindex(*argv, '/');
	if (!ptr)
		ptr = *argv - 1;
	actfp = xfopen(ACTIVE, "r+");
#ifdef BSD4_2
	if (flock(fileno(actfp), LOCK_SH|LOCK_NB) < 0 && errno == EWOULDBLOCK)
#else	/* !BSD4_2 */
#ifdef	LOCKF
	if (lockf(fileno(actfp), F_TLOCK, 0) < 0 &&
		(errno == EAGAIN || errno == EACCES))
#else	/* !LOCKF */
	sprintf(bfr, "%s.lock", ACTIVE);
	if (LINK(ACTIVE,bfr) < 0 && errno == EEXIST)
#endif /* V7 */
#endif	/* !BSD4_2 */
		spool_news = 2;
	else {
#ifdef SPOOLNEWS
		if (argc > 1 && !strcmp(*(argv+1), "-S")) {
			argc--;
			argv++;
		} else
			spool_news = 1;

#endif /* SPOOLNEWS */
	}
#ifdef BSD4_2
	flock(fileno(actfp), LOCK_UN);
#else	/* !BSD4_2 */
#ifdef	LOCKF
	lockf(fileno(actfp), F_ULOCK, 0);
#else	/* !LOCKF */
	UNLINK(bfr);
#endif /* V7 */
#endif	/* !BSD4_2 */
	if (argc > 1 && !strcmp(*(argv+1), "-U")) {
		if (spool_news > 1) /* can't unspool while things are locked */
			xxit(0);
		dounspool();
		/* NOT REACHED */
	}

	if (!strncmp(ptr+1, "rnews", 5)) {
		mode = PROC;
		if (spool_news) {
			dospool((char *)NULL, FALSE);
			/* NOT REACHED */
		}
#ifdef NICENESS
		if (nice(0) < NICENESS)
			(void) nice(NICENESS);
#endif /* NICENESS */
	} else
		if (argc < 2)
			goto usage;

	state = OPTION;
	header.title[0] = header.nbuf[0] = filename[0] = '\0';

	/* check for existence of special files */
	if (!rwaccess(ARTFILE)) {
		mfd = mailhdr((struct hbuf *)NULL, exists(ARTFILE) ? "Unwritable files!" : "Missing files!");
		if (mfd != NULL) {
#ifdef HIDDENNET
			fprintf(mfd,"System: %s.%s\n\nThere was a problem with %s!!\n", LOCALSYSNAME, FULLSYSNAME, ARTFILE);
#else /* !HIDDENNET */
			fprintf(mfd,"System: %s\n\nThere was a problem with %s!!\n", FULLSYSNAME, ARTFILE);
#endif /* !HIDDENNET */
			(void) sprintf(cbuf, "touch %s;chmod 666 %s", ARTFILE, ARTFILE);
			(void) system(cbuf);
			if (rwaccess(ARTFILE))
				fprintf(mfd, "The problem has been taken care of.\n");
			else
				fprintf(mfd, "Corrective action failed - check suid bits.\n");
			(void) mclose(mfd);
		}
	}
	if (!rwaccess(ACTIVE)) {
		mfd = mailhdr((struct hbuf *)NULL, exists(ACTIVE) ? "Unwritable files!" : "Missing files!");
		if (mfd != NULL) {
#ifdef HIDDENNET
			fprintf(mfd,"System: %s.%s\n\nThere was a problem with %s!!\n", LOCALSYSNAME, FULLSYSNAME, ARTFILE);
#else /* !HIDDENNET */
			fprintf(mfd, "System: %s\n\nThere was a problem with %s!!\n", FULLSYSNAME, ACTIVE);
#endif /* !HIDDENNET */
			(void) sprintf(cbuf, "touch %s;chmod 666 %s", ACTIVE, ACTIVE);
			(void) system(cbuf);
			if (rwaccess(ACTIVE))
				fprintf(mfd, "The problem has been taken care of.\n");
			else
				fprintf(mfd, "Corrective action failed - check suid bits.\n");
			(void) mclose(mfd);
		}
	}
	SigTrap = FALSE;	/* true if a signal has been caught */
	if (mode != PROC) {
		(void) signal(SIGHUP, onsig);
		(void) signal(SIGINT, onsig);
	}
	uid = getuid();
	gid = getgid();
	duid = geteuid();
	dgid = getegid();
	(void) ftime(&Now);
	if (uid == 0 && duid == 0) {
		/*
		 * Must go through with this kludge since
		 * some systems do not honor the setuid bit
		 * when root invokes a setuid program.
		 */
		if ((pw = getpwnam(NEWSUSR)) == NULL)
			xerror("Cannot get NEWSU pw entry");

		duid = pw->pw_uid;
		if ((gp = getgrnam(NEWSGRP)) == NULL)
			xerror("Cannot get NEWSG gr entry");
		dgid = gp->gr_gid;
		(void) setgid(dgid);
		(void) setuid(duid);
	}

#ifndef IHCC
	/*
	 * We force the use of 'getuser()' to prevent forgery of articles
	 * by just changing $LOGNAME
	 */
	if (isatty(fileno(stderr))) {
		if ((user = getenv("USER")) == NULL)
			user = getenv("LOGNAME");
		if ((home = getenv("HOME")) == NULL)
			home = getenv("LOGDIR");
	}
#endif
	if (user == NULL || home == NULL)
		getuser();
	else {
		if (username == NULL || username[0] == 0) {
			username = AllocCpy(user);
		}
		userhome = AllocCpy(home);
	}
	getuser();

	/* loop once per arg. */

	++argv;		/* skip first arg, which is prog name. */

	while (--argc) {
	    if (state == OPTION) {
		if (**argv != '-') {
			xerror("Bad option string \"%s\"", *argv);
		}
		while (*++*argv != '\0') {
			for (optpt = options; optpt->optlet != '\0'; ++optpt) {
				if (optpt->optlet == **argv)
					goto found;
			}
			/* unknown option letter */
usage:
			fprintf(stderr, "usage: inews -t title");
			fprintf(stderr, " [ -n newsgroups ]");
			fprintf(stderr, " [ -e expiration date ]\n");
			fprintf(stderr, "\t[ -f sender]\n\n");
			xxit(1);

		    found:;
			if (optpt->flag == TRUE || (mode != UNKNOWN &&
			    (mode&optpt->oldmode) == 0)) {
				xerror("Bad %c option", **argv);
			}
			if (mode == UNKNOWN)
				mode = optpt->newmode;
			filchar = optpt->filchar;
			optpt->flag = TRUE;
			state = STRING;
			ptr = optpt->buf;
			len = BUFLEN;
		}

		argv++;		/* done with this option arg. */

	    } else {

		/*
		 * Pick up a piece of a string and put it into
		 * the appropriate buffer.
		 */
		if (**argv == '-') {
			state = OPTION;
			argc++;	/* uncount this arg. */
			continue;
		}

		if ((tlen = strlen(*argv)) >= len)
			xerror("Argument string too long");
		(void) strcpy(ptr, *argv++);
		ptr += tlen;
		if (*(ptr-1) != filchar)
			*ptr++ = filchar;
		len -= tlen + 1;
		*ptr = '\0';
	    }
	}

	/*
	 * ALL of the command line has now been processed. (!)
	 */

	if (*filename) {
		infp = freopen(filename, "r", stdin);
		if (infp == NULL)
			xerror("freopen(%s): %s", filename, errmsg(errno));
	} else
		infp = stdin;

	tty = isatty(fileno(infp));

	if (mode == CREATENG)
		createng();

	if (header.ctlmsg[0] != '\0' && header.title[0] == '\0')
		(void) strcpy(header.title, header.ctlmsg);

	if (*header.nbuf) {
		lcase(header.nbuf);
		ptr = index(header.nbuf, '\0');
		if (ptr[-1] == NGDELIM)
			*--ptr = '\0';
	}
	(void) nstrip(header.title);
	(void) nstrip(header.expdate);
	(void) nstrip(header.followid);
	if (mode != PROC) {
		if (hflag) {
			header.path[0] = '\0';
			(void) hread(&header, infp, FALSE);
			/* there are certain fields we won't let him specify. */
			if (header.from[0])
				(void) strcpy(forgedname, header.from);
			if (!header.approved[0])
				Mflag = FALSE;
			header.from[0] = '\0';
			header.sender[0] = '\0';
			if (header.subdate[0] && cgtdate(header.subdate) < 0)
				header.subdate[0] = '\0';
		}

		if (header.ident[0] == '\0')
			getident(&header);

		if (forgedname[0]) {
			register char *p1;
			if (Mflag)
				sprintf(header.path, "%s!%s",
					FULLSYSNAME, username);
			else if (!header.path[0]) {
				(void) strcpy(header.path, forgedname);

				if ((p1 = strpbrk(header.path, "@ (<")) != NULL)
					*p1 = '\0';
			}
			if (!Mflag && !strpbrk(forgedname, "@ (<"))
#ifdef UW
				(void) sprintf(header.from,"%s@%s",
					forgedname, DOMAINNAME);
#else
				(void) sprintf(header.from,"%s@%s%s",
					forgedname, FULLSYSNAME, MYDOMAIN);
#endif UW
			else
				(void) strncpy(header.from, forgedname, BUFLEN);

#ifdef UW
			(void) sprintf(header.sender, "%s@%s",
				username, DOMAINNAME);
#else
			(void) sprintf(header.sender, "%s@%s%s",
				username, FULLSYSNAME, MYDOMAIN);
#endif UW
		} else {
			gensender(&header, username);
		}
#ifdef MYORG
		if (header.organization[0] == '\0' && !Mflag &&
			header.sender[0] == '\0') {
			strncpy(header.organization, MYORG, BUFLEN);
			if (strncmp(header.organization, "Frobozz", 7) == 0)
				header.organization[0] = '\0';
			if (ptr = getenv("ORGANIZATION"))
				strncpy(header.organization, ptr, BUFLEN);
			/*
			 * Note that the organization can also be turned off by
			 * setting it to the null string, either in MYORG or
			 * $ORGANIZATION in the environment.
			 */
			if (header.organization[0] == '/') {
				mfd = fopen(header.organization, "r");
				if (mfd) {
					(void) fgets(header.organization, sizeof header.organization, mfd);
					(void) fclose(mfd);
				} else {
					header.organization[0] = '\0';
					logerr("Couldn't open %s",
						header.organization);
				}
				ptr = index(header.organization, '\n');
				if (ptr)
					*ptr = '\0';
			}
		}
#endif /* MYORG */
	}

	/* Authorize newsgroups. */
	if (mode == PROC) {
#ifdef BATCH
		checkbatch();
#endif /* BATCH */
		(void) signal(SIGHUP, SIG_IGN);
		(void) signal(SIGINT, SIG_IGN);
		(void) signal(SIGQUIT, SIG_IGN);
		header.ident[0] = '\0';
		if (hread(&header, infp, TRUE) == NULL)
			error("Inbound news is garbled");
		input();
	}
	/* always check history */

	if (history(&header)) {
		log("Duplicate article %s rejected. Path: %s",
			header.ident, header.path);
		xxit(0);
	}

	/* Easy way to make control messages, since all.all.ctl is unblessed */
	if (mode != PROC && prefix(header.title, "cmsg ") && header.ctlmsg[0] == 0)
		(void) strcpy(header.ctlmsg, &header.title[5]);
	is_ctl = mode != CREATENG &&
		(ngmatch(header.nbuf, "all.all.ctl,") || header.ctlmsg[0]);
#ifdef DEBUG
	fprintf(stderr,"is_ctl set to %d\n", is_ctl);
#endif

	if (mode != CREATENG) {
		if (!*header.title)
			error("No title, ng %s from %s", header.nbuf,
				header.from);
		if (!*header.nbuf)
			(void) strcpy(header.nbuf, DFLTNG);
	}

	if (mode <= UNPROC) {
#ifdef FASCIST
		if (uid && uid != ROOTID && fascist(user, header.nbuf))
			xerror("User %s is not authorized to post to newsgroup %s",
				user, header.nbuf);
#endif /* FASCIST */
		ctlcheck();
	}

	if (mode == CREATENG)
		createng();

	/* Determine input. */
	if (mode != PROC)
		input();
	if (header.intnumlines == 0 && !is_ctl)
		error("%s rejected: no text lines", header.ident);

	dates(&header);

	/* Do the actual insertion. */
	insert();
}

dospool(batchcmd, dolhwrite)
char *batchcmd;
int dolhwrite;
{
	register int c;
	register FILE *sp;
	register struct tm *tp;
	time_t t;
	char buf[BUFLEN];
	extern struct tm *gmtime();

	(void) time(&t);
	tp = gmtime(&t);
	/* This file name "has to" be unique  (right?) */
#ifdef USG
	(void) sprintf(buf, "%s/.rnews/%2.2d%2.2d%2.2d%2.2d%2.2d%x",
#else
#ifdef VMS
	/* Eunice doesn't like dots in directory names */
	(void) sprintf(buf, "%s/+rnews/%02d%02d%02d%02d%02d%x",
#else /* V7 */
	(void) sprintf(buf, "%s/.rnews/%02d%02d%02d%02d%02d%x",
#endif /* V7 */
#endif /* VMS */
		SPOOL,
		tp->tm_year, tp->tm_mon+1, tp->tm_mday,
		tp->tm_hour, tp->tm_min, getpid());
	sp = fopen(buf, "w");
	if (sp == NULL) {
		char dbuf[BUFLEN];
#ifdef VMS
		sprintf(dbuf, "%s/+rnews", SPOOL);
#else /* !VMS */
		sprintf(dbuf, "%s/.rnews", SPOOL);
#endif /* !VMS */
		if (mkdir(dbuf, 0777&~N_UMASK) < 0)
			xerror("Cannot mkdir %s: %s", dbuf, errmsg(errno));
		sp = xfopen(buf, "w");
	}
	if (batchcmd != NULL)
		fprintf(sp, "%s\n", batchcmd);
	else
		if (not_here[0] != '\0')
			fprintf(sp, "#! inews -x %s -p\n", not_here);
	if (dolhwrite)
		lhwrite(&header, sp);
	while ((c = getc(infp)) != EOF)
		putc(c, sp);
	fclose(sp);
	xxit(0);
	/* NOTREACHED */
}

/*
 *	Create a newsgroup
 */
createng()
{
	register char *cp;

	/*
	 * Only certain users are allowed to create newsgroups
	 */
	if (uid != ROOTID && uid != duid && uid) {
		logerr("Please contact one of the local netnews people");
		xerror("to create group \"%s\" for you", header.ctlmsg);
	}
	if (header.distribution[0] == '\0')
#ifdef ORGDISTRIB
		strcpy(header.distribution, ORGDISTRIB);
#else /* !ORGDISTRIB */
		strcpy(header.distribution, "local");
#endif /* !ORGDISTRIB */

	(void) strcpy(header.nbuf, header.ctlmsg);
	if ((cp=index(header.nbuf, ' ')) != NULL)
		*cp = '\0';

	if (header.approved[0] == '\0')
#ifdef UW
		(void) sprintf(header.approved, "%s@%s",
				username, DOMAINNAME);
#else
		(void) sprintf(header.approved, "%s@%s%s",
				username, FULLSYSNAME, MYDOMAIN);
#endif UW
	(void) sprintf(bfr, "%s/inews -n %s.ctl -c newgroup %s -d %s -a \"%s\"",
		LIB, header.nbuf, header.ctlmsg, header.distribution,
		header.approved);
	if (tty) {
		printf("Please type in a paragraph describing the new newsgroup.\n");
		printf("End with control D as usual.\n");
	}
	printf("%s\n", bfr);
	(void) fflush(stdout);
	(void) system(bfr);
	exit(0);
	/*NOTREACHED*/
}

char firstbufname[BUFLEN];
/*
 *	Link ARTICLE into dir for ngname and update active file.
 */
long
localize(ngname)
char	*ngname;
{
	char afline[BUFLEN];
	long ngsize;
	long fpos;
	int e;
	char *cp;

	lock();
	(void) rewind(actfp); clearerr(actfp);

	for(;;) {
		fpos = ftell(actfp);
		if (fgets(afline, sizeof afline, actfp) == NULL) {
			unlock();
			logerr("Can't find \"%s\" in active file", ngname);
			return FALSE;		/* No such newsgroup locally */
		}
		if (prefix(afline, ngname)) {
			(void) sscanf(afline, "%s %ld", bfr, &ngsize);
			if (strcmp(bfr, ngname) == 0) {
				if (ngsize < 0 || ngsize > 99998) {
					logerr("found bad ngsize %ld ng %s, setting to 1", ngsize, bfr);
					ngsize = 1;
				}
				break;
			}
		}
	}
	for (;;) {
		cp = dirname(ngname);

		(void) sprintf(bfr, "%s/%ld", cp, ngsize+1);
#ifdef VMS
		/*
		 * The effect of this code is to store the article in the first
		 * newsgroup's directory and to put symbolic links elsewhere.
		 * If this is the first group, firstbufname is not yet filled
		 * in. It should be portable to other link-less systems.
		 * epimass!jbuck
		 */
		if (firstbufname[0]) {
			if (vmslink(firstbufname, bfr) == 0)
				break;
		} else if (rename(ARTICLE, bfr) == 0)
			break;
#else /* !VMS */
		if (link(ARTICLE, bfr) == 0)
			break;
#endif /* !VMS */
		if (!exists(cp))
			mknewsg(cp, ngname);
#ifdef VMS
		if (firstbufname[0]) {
			if (vmslink(firstbufname, bfr) == 0)
				break;
		} else if (rename(ARTICLE, bfr) == 0) 
			break;
#else /* !VMS */
		if (link(ARTICLE, bfr) == 0)
			break;
#endif /* !VMS */
		e = errno;	/* keep log from clobbering it */
		log("Cannot install article as %s: %s", bfr, errmsg(errno));
		if (e != EEXIST) {
			logerr("Link into %s failed (%s); check dir permissions.",
			    bfr, errmsg(e));
			unlock();
			return FALSE;
		}
		ngsize++;
	}

	/*
	 * This works around a bug in the 4.1bsd stdio
	 * on fseeks to non even offsets in r+w files
	 */
	if (fpos&1)
		(void) rewind(actfp);

	(void) fseek(actfp, fpos, 0);
	/*
	 * Has to be same size as old because of %05d.
	 * This will overflow with 99999 articles.
	 */
	fprintf(actfp, "%s %05ld", ngname, ngsize+1);
#if defined(USG) || defined(MG1)
	/*
	 * U G L Y   K L U D G E
	 * This utter piece of tripe is the only way I know of to get
	 * around the fact that ATT BROKE standard IO in System 5.2.
	 * Basically, you can't open a file for "r+" and then try and
	 * write to it. This works on all "real" USGUnix systems, It will
	 * probably break on some obscure look alike that doesnt use the
	 * real ATT stdio.h
	 * Don't blame me, blame ATT. stdio should have already done the
	 * following line for us, but it doesn't
	 * also broken in WCW MG-1 42nix 2.0
	 */
	 actfp->_flag |= _IOWRT;
#endif /* USG */
	(void) fflush(actfp);
	if (ferror(actfp))
		xerror("Active file write failed");
	unlock();
	if (firstbufname[0] == '\0')
		(void) strcpy(firstbufname, bfr);
	(void) sprintf(bfr, "%s/%ld ", ngname, ngsize+1);
	addhist(bfr);
	return ngsize+1;
}

/*
 *	Localize for each newsgroup and broadcast.
 */
insert()
{
	register char *ptr;
	register FILE *tfp;
	register int c;
	struct srec srec;	/* struct for sys file lookup	*/
	struct tm *tm, *gmtime();
	int is_invalid = FALSE;
	int exitcode = 0;
	long now;
#ifdef DOXREFS
	register char *nextref = header.xref;
#endif /* DOXREFS */

	/* Clean up Newsgroups: line */
	if (!is_ctl && mode != CREATENG)
		is_invalid = ngfcheck(mode == PROC);

	(void) time(&now);
	tm = gmtime(&now);
	if (header.expdate[0])
		addhist(" ");
#ifdef USG
	sprintf(bfr,"%2.2d/%2.2d/%d %2.2d:%2.2d\t",
#else /* !USG */
	sprintf(bfr,"%02d/%02d/%d %02d:%02d\t",
#endif /* !USG */
		tm->tm_mon+1, tm->tm_mday, tm->tm_year,tm->tm_hour, tm->tm_min);
	addhist(bfr);
	log("%s %s ng %s subj '%s' from %s",
		spool_news ? "queued" : (mode==PROC ? "received" : "posted"),
		header.ident, header.nbuf, header.title, header.from);

	/* Write article to temp file. */
	tfp = xfopen(mktemp(ARTICLE), "w");

	if (is_invalid) {
		logerr("No valid newsgroups found, moved to junk");
		if (localize("junk"))
			savehist(histline);
		exitcode = 1;
		goto writeout;
	}

#ifdef ZAPNOTES
	if (strncmp(header.title, "Re: Orphaned Response", 21) == 0) {
		logerr("Orphaned Response, moved to junk");
		if (localize("junk"))
			savehist(histline);
		exitcode = 1;
		goto writeout;
	}
#endif	/* ZAPNOTES */

	if (time((time_t *)0) > (cgtdate(header.subdate) + HISTEXP) ){
		logerr("Article too old, moved to junk");
		if (localize("junk"))
			savehist(histline);
		exitcode = 1;
		goto writeout;
	}

	if (is_mod[0] != '\0' 	/* one of the groups is moderated */
		&& header.approved[0] == '\0') { /* and unapproved */
		struct hbuf mhdr;
		FILE *mfd;
		register char *p;
		char modadd[BUFLEN];
#ifdef DONTFOWARD
		if(mode == PROC) {
			logerr("Unapproved article in moderated group %s",
				is_mod);
			if (localize("junk"))
				savehist(histline);
			goto writeout;
		}
#endif /* DONTFORWARD */
		fprintf(stderr,"%s is moderated and may not be posted to",
			is_mod);
		fprintf(stderr," directly.\nYour article is being mailed to");
		fprintf(stderr," the moderator who will post it for you.\n");
		/* Let's find a path to the backbone */
		sprintf(bfr, "%s/mailpaths", LIB);
		mfd = xfopen(bfr, "r");
		do {
			if (fscanf(mfd, "%s %s", bfr, modadd) != 2)
				xerror("Can't find backbone in %s/mailpaths",
					LIB);
		} while (strcmp(bfr, "backbone") != 0 && !ngmatch(is_mod, bfr));
		(void) fclose(mfd);
		/* fake a header for mailhdr */
		mhdr.from[0] = '\0';
		mhdr.replyto[0] = '\0';
		p = is_mod;
		while (*++p)
			if (*p == '.')
				*p = '-';
		sprintf(bfr, "Submission for %s", is_mod);
		sprintf(mhdr.path, modadd, is_mod);
		mfd = mailhdr(&mhdr, bfr);
		if (mfd == NULL)
			xerror("Can't send mail to %s", mhdr.path);
		lhwrite(&header, mfd);
		while ((c = getc(infp)) != EOF)
			putc(c, mfd);
		mclose(mfd);
		log("Article mailed to %s", mhdr.path);
		xxit(0);
	}

	if (spool_news && mode != PROC) {
		fprintf(stderr,"Your article has been spooled for later processing.\n");
		dospool("#! inews -S -h", TRUE);
		/* NOT REACHED */
	}

	if (is_ctl) {
		exitcode = control(&header);
		if (localize("control") && exitcode != 0)
			savehist(histline);
	} else {
		if (s_find(&srec, FULLSYSNAME) == FALSE) {
			logerr("Cannot find my name '%s' in %s", FULLSYSNAME, SUBFILE);
			srec = dummy_srec;
		}
#ifdef DOXREFS
		(void) strncpy(nextref, FULLSYSNAME, BUFLEN);
#endif /* DOXREFS */
		for (ptr = nbuf; *ptr;) {
			if (ngmatch(ptr,srec.s_nbuf) || index(ptr,'.') == NULL){
#ifdef DOXREFS
				while (*nextref++)
					;
				(void) sprintf(--nextref, " %s:%ld", ptr, localize(ptr));
#else /* !DOXREFS */
				(void) localize(ptr);
#endif /* !DOXREFS */
			}
			while (*ptr++)
				;
		}
		if (firstbufname[0] == '\0') {
			logerr("Newsgroups in active, but not sys");
			(void) localize("junk");
		}
	}
#ifdef DOXREFS
	if (index(header.nbuf, NGDELIM) == NULL)
		header.xref[0] = '\0';
#endif /* DOXREFS */

writeout:
	/* Part 1 of kludge to get around article truncation problem */
	if ( (c=getc(infp)) != EOF) {
		ungetc(c, infp);
		if (c == ' ' || c == '\t') {
			header.intnumlines++;
			(void) sprintf(header.numlines, "%d",
				header.intnumlines);
		}
	}
	/* End of part 1 */
	if (header.expdate[0] != '\0' && mode != PROC) {
		/* Make sure it's fully qualified */
		long t = cgtdate(header.expdate);
		strcpy(header.expdate, arpadate(&t));
	}

	lhwrite(&header, tfp);
	if ((c = getc(infp)) != EOF) {
		/* Part 2 of kludge to get around article truncation problem */
		if (c == ' ' || c == '\t' )
			putc('\n', tfp);
		/* End of part 2 */
		ungetc(c, infp);
		while (fgets(bfr, BUFLEN, infp) != NULL)
			fputs(bfr, tfp);
		if (bfr[strlen(bfr)-1] != '\n')
			putc('\n',tfp);
	}
	if (ferror(tfp))
		xerror("Write failed for temp file");
	(void) fclose(tfp);
	(void) fclose(infp);
	if(exitcode == 0) {
		/* article has passed all the checks, so work in background */
		if (mode != PROC) {
			int pid;
			if ((pid=fork()) < 0)
				xerror("Can't fork");
			else if (pid > 0)
				_exit(0);
		}
#ifdef SIGTTOU
		signal(SIGTTOU, SIG_IGN);
#endif /* SIGTTOU */
		savehist(histline);
		broadcast(mode==PROC);
	}
	xxit((mode == PROC && filename[0] == '\0') ? 0 : exitcode);
}

input()
{
	register char *cp;
	register int c;
	register int empty = TRUE;
	FILE *tmpfp;
	int consec_newlines = 0;
	int linecount = 0;
	int linserted = 0;

	tmpfp = xfopen(mktemp(INFILE), "w");
	while (!SigTrap && fgets(bfr, BUFLEN, infp) != NULL) {
 		if (mode == PROC) {	/* zap trailing empty lines */
#ifdef ZAPNOTES
			if (empty && bfr[0] == '#' && bfr[2] == ':'
				&& header.nf_id[0] == '\0'
				&& header.nf_from[0] == '\0' ) {
				(void) strcpy(header.nf_id, bfr);
				(void) nstrip(header.nf_id);
				(void) fgets(bfr, BUFLEN, infp);
				(void) strcpy(header.nf_from, bfr);
				(void) nstrip(header.nf_from);
				(void) fgets(bfr, BUFLEN, infp);

				if (header.numlines[0]) {
					header.intnumlines -= 2;
					(void) sprintf(header.numlines, "%d", header.intnumlines);
				}

				/* Strip trailing " - (nf)" */
				if ((cp = rindex(header.title, '-')) != NULL
				    && !strcmp(--cp, " - (nf)"))
					*cp = '\0';
				log("Stripped notes header on %s", header.ident);
				continue;
			}
#endif /* ZAPNOTES */
 			if (bfr[0] == '\n' ||
				/* Bandage for older versions of inews */
				bfr[1] == '\n' && !isascii(bfr[0])) {
 				consec_newlines++;	/* count it, in case */
 				continue;		/* but don't write it*/
 			}
 			/* foo! a non-empty line. write out all saved lines. */
 			while (consec_newlines > 0) {
				putc('\n', tmpfp);
				consec_newlines--;
				linecount++;
			}
 		}
		if (mode != PROC && tty && strcmp(bfr, ".\n") == 0)
			break;
		for (cp = bfr; c = toascii(*cp); cp++) {
			if (isprint(c) || isspace(c) || c == '\b')
				putc(c, tmpfp);
			if (c == '\n')
				linecount++;
		}
		if (bfr[0] == '>')
			linserted++;
		if (bfr[0] == '<') /* kludge to allow diff's to be posted */
			linserted--;
		empty = FALSE;
	}
	if (*filename)
		(void) fclose(infp);
	if (mode != PROC &&
		linecount > LNCNT && linserted > (linecount-linserted))
		error("Article rejected: %s included more text than new text",
			username);

	if (mode != PROC && !is_ctl && header.sender[0] == '\0') {
		int siglines = 0;
		char sbuf[BUFLEN];
		(void) sprintf(bfr, "%s/%s", userhome, ".signature");
		if (access(bfr, 4) == 0) {
			if ((infp = fopen(bfr, "r")) == NULL) {
				(void) fprintf(stderr,
    "inews: \"%s\" left off (must be readable by \"inews\" owner)\n", bfr);
				goto finish;
			}

			while (fgets(sbuf, sizeof sbuf, infp) != NULL)
				if (++siglines > 4)
					break;
			if (siglines > 4)
				fprintf(stderr,".signature not included (> 4 lines)\n");
			else {
				rewind(infp);
				fprintf(tmpfp, "-- \n");	/* To separate */
				linecount++;
				while ((c = getc(infp)) != EOF) {
					putc(c, tmpfp);
					if (c == '\n')
						linecount++;
				}
			}
			(void) fclose(infp);
		}
	}

finish:
	if (ferror(tmpfp))
		xerror("write failed to temp file");
	(void) fclose(tmpfp);
	if (SigTrap) {
		if (tty)
			fprintf(stderr, "Interrupt\n");
		if (tty && !empty)
			fwait(fsubr(newssave, (char *) NULL, (char *) NULL));
		if (!tty)
			log("Blown away by an interrupt %d", SigTrap);
		xxit(1);
	}
	if (tty)
		fprintf(stderr, "EOT\n");
	fflush(stdout);
	infp = fopen(INFILE, "r");
	if (header.numlines[0]) {
		/*
		 * Check line count if there's already one attached to
		 * the article.  Could make this a fatal error -
		 * throwing it away if it got chopped, in hopes that
		 * another copy will come in later with a correct
		 * line count.  But that seems a bit much for now.
		 */
		if (linecount != header.intnumlines) {
			if (linecount == 0)
				error("%s rejected. linecount expected %d, got 0", header.ident, header.intnumlines);
#ifdef UW
			if (linecount+consec_newlines+10 <= header.intnumlines)
				xerror("%s rejected. linecount expected %d, got %d", header.ident, header.intnumlines, linecount+consec_newlines);
#endif UW
			if (linecount > header.intnumlines ||
			    linecount+consec_newlines < header.intnumlines)
				log("linecount expected %d, got %d", header.intnumlines, linecount+consec_newlines);
		}
		/* adjust count for blank lines we stripped off */
		if (consec_newlines) {
			header.intnumlines -= consec_newlines;
			if (header.intnumlines < 0 )
				header.intnumlines = 0; /* paranoia */
			(void) sprintf(header.numlines, "%d", header.intnumlines);
		}

	} else {
		/* Attach a line count to the article. */
		header.intnumlines = linecount;
		(void) sprintf(header.numlines, "%d", linecount);
	}
}

/*
 * Make the directory for a new newsgroup.  ngname should be the
 * full pathname of the directory.  Do the other stuff too.
 * The various games with setuid and chown are to try to make sure
 * the directory is owned by NEWSUSR and NEWSGRP, which is tough to
 * do if you aren't root.  This will work on a UCB system (which allows
 * setuid(geteuid()) or a USG system (which allows you to give away files
 * you own with chown), otherwise you have to change your kernel to allow
 * one of these things or run with your dirs 777 so that it doesn't matter
 * who owns them.
 */
mknewsg(fulldir, ngname)
char	*fulldir;
char	*ngname;
{
	if (ngname == NULL || !isalpha(ngname[0]))
		xerror("Tried to make illegal newsgroup %s", ngname);

	/* Create the directory */
	mkparents(fulldir);

	if (mkdir(fulldir, 0777) < 0)
		xerror("Cannot mkdir %s: %s", fulldir, errmsg(errno));

	log("make newsgroup %s in dir %s", ngname, fulldir);
}

/*
 * If any parent directories of this dir don't exist, create them.
 */
mkparents(dname)
char *dname;
{
	char buf[200];
	register char *p;

	(void) strcpy(buf, dname);
	p = rindex(buf, '/');
	if (p)
		*p = '\0';
	if (exists(buf))
		return;
	mkparents(buf);
	if (mkdir(buf, 0777) < 0)
		xerror("Can not mkdir %s: %s", buf, errmsg(errno));
}

cancel()
{
	register FILE *fp;

	log("cancel article %s", filename);
	fp = fopen(filename, "r");
	if (fp == NULL) {
		log("article %s not found", filename);
		return;
	}
	if (hread(&header, fp, TRUE) == NULL)
		error("Article is garbled.");
	(void) fclose(fp);
	(void) unlink(filename);
}

dounspool()
{
	register DIR	*dirp;
	register struct direct *dir;
	register int foundsome;
	int pid, status, ret;
#ifdef VMS
	sprintf(bfr, "%s/+rnews", SPOOL);
#else /* !VMS */
	sprintf(bfr, "%s/.rnews", SPOOL);
#endif /* !VMS */

	if (chdir(bfr) < 0)
		xerror("chdir(%s):%s", bfr, errmsg(errno));

	do {
		foundsome = 0;
		dirp = opendir(".");
		if (dirp == NULL)	/* Boy are things screwed up */
			xerror("opendir can't open .:%s", errmsg(errno));

		while ((dir=readdir(dirp)) != NULL) {
			if (dir->d_name[0] == '.')
				continue;
			if ((pid=vfork()) == -1)
				xerror("Can't fork: %s", errmsg(errno));
			if (pid == 0) {
				execl(RNEWS, "rnews", "-S", "-p", dir->d_name,
					(char *) NULL);
				_exit(1);
			}
			
			while ((ret=wait(&status)) != pid && ret != -1)
				/* continue */;

			if (status != 0) {
				sprintf(bfr, "../%s", dir->d_name);
				(void) LINK(dir->d_name, bfr);
				logerr("rnews failed, status %d. Batch saved in %s/%s",
					status, SPOOL, dir->d_name);
			}
			(void) unlink(dir->d_name);
			foundsome++;
		}
		closedir(dirp);
	} while (foundsome); /* keep rereading the directory until it's empty */

	xxit(0);
}
