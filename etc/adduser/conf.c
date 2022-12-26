char Glob_names[] =	"/usr/lib/aliases";	/* names on other machines */

#ifdef NOTDEF
char Passwd[] =		"/etc/passwd";		/* password file */
char Lockfile[] =	"/etc/ptmp";		/* password lock file */
char NewPasswd[] =	"/etc/passwd.new";	/* the new password file */
char OldPasswd[] =	"/etc/passwd.old";	/* the old password file */
char Group[] =		"/etc/group";		/* group file */
char NewGroup[] =	"/etc/group.new";	/* the new group file */
char OldGroup[] =	"/etc/group.old";	/* the old group file */
#else
char Passwd[50];
char Lockfile[50];
char NewPasswd[50];
char OldPasswd[50];
char Group[50];
char NewGroup[50];
char OldGroup[50];
#endif NOTDEF

char Skel[] =		"/usr/skel";		/* where the '.'-files are */
#ifdef pyr
char Universe[] = 	"/etc/u_universe";	/* where the def universes are */
char defuniverse[]=	"ucb";				/* put him in BSD (system V sucks */
#endif pyr

int nomail = 0;
int instructional = 0;

char User[] =	"/etc/user";		/* user file */
char IShell[] =	"/bin/csh";		/* default shell */
char RShell[] =	"/bin/mesh";		/* special research shell */

struct 	{
	int	start_id;
	char lpquota[6];
	char resquota[4];
	char	acct_name[10];
	} acct_types[6] = {
		{ 16,	"@100", "@0",	"staff" },
		{ 64,	"@100", "@0",	"TA" },
		{ 64,	"@100", "@0",	"COURSE" },
		{ 64,	"@0",	"@0",	"GRAD" },
		{ 128,	"@0",	"@0",	"UGRAD" },
		{ 1000,	"@300", "@2",	"" }
	};
