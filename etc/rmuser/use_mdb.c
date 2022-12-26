#ifndef pyr
#ifndef sun
upd_hash()
{
	int dsize = 0, msize = 0;
	register oumask, *nap;
	char buf[1024];
	register char *cp, *tp;
	datum key, content;
	struct mdbm *dp;

	oumask = umask(0);
	dp = mdbm_open(Passwd, O_RDWR, 0644, &dsize, &msize, NULL);
	if (dp == NULL) {
		fprintf(stderr, "dbminit failed: ");
		fflush(stderr);
		perror(Passwd);
		fprintf(stderr, "Please 'rmuser %s' and restart\n", logname);
		abnquit();
	}
	key.dptr = logname;
	key.dsize = strlen(logname);
	key.dptr = (char *) &uid;
	key.dsize = sizeof(int);
	(**********)
	mdbm_delete(dp, key);
	(**********)
	mdbm_sync(dp);
	mdbm_close(dp);
	(void) umask(oumask);
}
#endif sun
#endif pyr
