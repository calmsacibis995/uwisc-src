#include <stdio.h>
#include <sys/file.h>
#include "rmuser.h"
#ifndef pyr
#ifndef sun

upd_hash(user,uid)
char *user;
int uid;
{
	int dsize = 0, msize = 0;
	register oumask, *nap;
	char buf[1024];
	register char *cp, *tp;
	datum key, content;
	DBM *dp;

	oumask = umask(0);
	dp = dbm_open(Passwd, O_RDWR, 0644, &dsize, &msize, NULL);
	if (dp==NULL) {
		fflush(stderr);
		perror(Passwd);
		fprintf(stderr, "Please 'rmuser %s' and restart\n", user);
		abnquit();
	}
	key.dptr = user;
	key.dsize = strlen(user);
	dbm_delete(dp, key);

	key.dptr = (char *) &uid;
	key.dsize = sizeof(int);
	dbm_delete(dp, key);

	dbm_close(dp);
	(void) umask(oumask);
}
#endif sun
#endif pyr

mail_aliases(user)
char *user;
{
FILE	*pipe, *popen();
#define HOSTLEN 20
	char	hostname[HOSTLEN];

	if((pipe = popen(ALIAS_CMD, "w")) == NULL) {
		printf("Error could not 'mail aliases'. Do it yourself!\n");
		quit(5);
	}
	fprintf(pipe, "To: aliases\n");
	fprintf(pipe, "Subject: Rmuser - %s\n\n", user);
	fprintf(pipe, "Please remove the mail alias for %s.\n", user);
	gethostname(hostname, HOSTLEN);
	fprintf(pipe, "Login name is %s@%s.\n", user, hostname);
	fprintf(pipe, "\t\t\tThe rmuser program\n.\n");
	pclose(pipe);
}

/* Tell 'em how to use this program */
usage()
{
	fprintf(stderr, "usage: rmuser [-c course -f filename -h] user\n");
	exit(2);
}

/* Removes the lock file and any other stuff if we are exiting abnormally */
abnquit()
{
	(void)unlink(Lockfile);
	exit(5);
}

quit(stat)	/* normal exit, kill lock and go away, status is stat */
	int	stat;
{
	(void)unlink(Lockfile);
	exit(stat);
}
