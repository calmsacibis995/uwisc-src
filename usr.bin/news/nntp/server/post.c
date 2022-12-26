#ifndef lint
static char	*sccsid = "@(#)post.c	1.7	(Berkeley) 6/16/86";
#endif

#include "common.h"

/*
 * POST
 *
 * Post an article to a set of newsgroups.
 */

post(argc, argv)
int	argc;
char	*argv[];
{
	int	retcode;

	if (!canpost) {
		printf("%d Sorry, you're not allowed to post.\r\n",
			ERR_NOPOST);
#ifdef LOG
			syslog(LOG_INFO, "%s post rejected", hostname);
#endif
		(void) fflush(stdout);
		return;
	}

#ifdef POSTER
	if (uid_poster == 0) {
		printf("%d User %s does not exist!  Can't post.\r\n",
			ERR_POSTFAIL, POSTER);
#ifdef SYSLOG
		syslog(LOG_ERR, "post: User %s does not exist.", POSTER);
#endif
		(void) fflush(stdout);
		return;
	}
#endif

	retcode = spawn(INEWS, "inews", "-h", CONT_POST, ERR_POSTFAIL);
	if (retcode <= 0)
		printf("%d Posting failed.\r\n", ERR_POSTFAIL);
	else if (retcode > 0)
		printf("%d Article posted successfully.\r\n", OK_POSTED);
	(void) fflush(stdout);

#ifdef LOG
	syslog(LOG_INFO, "%s post %s", hostname,
			retcode == 1 ? "succeeded" : "failed");
#endif

}
