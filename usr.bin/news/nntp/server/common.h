/*
 * Common declarations, includes, and other goodies.
 *
 * @(#)common.h	1.6	(Berkeley) 4/13/86
 */

#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <syslog.h>
#include <strings.h>
#include "../common/response_codes.h"
#include "../common/conf.h"
#ifdef DBM
#include <dbm.h>
#undef NULL			/* Such Joe Code ... */
#else
#include <ndbm.h>
#endif
#include <stdio.h>

/*
 * For "spew()"
 */

#define	ARTICLE	0
#define	HEAD	1
#define	BODY	2
#define	STAT	3

#define	valid_art(s)	(atoi(s) != 0)

extern	int	errno;

extern	char	*gets(), *fgets();
extern	char	*mktemp();
extern	FILE	*open_valid_art();
extern	FILE	*openartbyid();

extern	int	ahbs(), group(), help(), ihave();
extern	int	list(), newgroups(), newnews(), nextlast(), post();
extern	int	slave(), stat();

extern	char	*group_array[];
extern	int	num_groups;
extern	char	*homedir;
extern	int	ingroup;
extern	int	maxgroups;
extern	int	art_array[];
extern	int	art_ptr;
extern	FILE	*art_fp;
extern	int	num_arts;
extern	int	uid_poster, gid_poster;
extern	int	canread, canpost;

#ifdef LOG
extern	int	grps_acsd, arts_acsd;
extern	char	hostname[];
#endif
