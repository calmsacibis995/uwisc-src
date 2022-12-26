#ifndef lint
static  char sccsid[] = "@(#)mount.c 1.1 85/05/30 Copyr 1985 Sun Micro";
#endif

/*
 * Copyright (c) 1985 Sun Microsystems, Inc.
 */

/*
 * mount
 */
#include <sys/param.h>
#include <rpc/rpc.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <nfs/nfs.h>
#include <rpcsvc/mount.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <stdio.h>
#include <mntent.h>

int	ro = 0;
int	hard = 1;
int	quota = 0;
int	fake = 0;
int	freq = 1;
int	passno = 2;
int	all = 0;
int	verbose = 0;

#ifdef INST
#define MAXSLEEP 900  /* max sleep 15 minutes */
#else !INST
#define MAXSLEEP 15  /* in seconds */
#endif INST

extern int errno;

char	*index(), *rindex();
char	host[MNTMAXSTR];
char	name[MNTMAXSTR];
char	dir[MNTMAXSTR];
char	type[MNTMAXSTR];
char	opts[MNTMAXSTR];

main(argc, argv)
	int argc;
	char **argv;
{
	struct mntent mnt;
	struct mntent *mntp;
	FILE *mnttab;
	char *options;
	char *colon;

	if (argc == 1) {
		mnttab = setmntent(MOUNTED, "r");
		while ((mntp = getmntent(mnttab)) != NULL) {
			if (strcmp(mntp->mnt_type, MNTTYPE_IGNORE) == 0) {
				continue;
			}
			printent(mntp);
		}
		endmntent(mnttab);
		exit(0);
	}

	/*
	 * Set options
	 */
	while (argc > 1 && argv[1][0] == '-') {
		options = &argv[1][1];
		while (*options) {
			switch (*options) {
			case 'a':
				all++;
				break;
			case 'f':
				fake++;
				break;
			case 'o':
				if (argc < 3) {
					usage();
				}
				strcpy(opts, argv[2]);
				argv++;
				argc--;
				break;
			case 'p':
				if (argc != 2) {
					usage();
				}
				mnttab = setmntent(MOUNTED, "r");
				while ((mntp = getmntent(mnttab)) != NULL) {
					prtmntent(stdout, mntp);
				}
				endmntent(mnttab);
				exit(0);
			case 'q':
				quota++;
				break;
			case 'r':
				ro++;
				break;
			case 't':
				if (argc < 3) {
					usage();
				}
				strcpy(type, argv[2]);
				argv++;
				argc--;
				break;
			case 'v':
				verbose++;
				break;
			default:
				fprintf(stderr, "mount: unknown option: %c\n",
				    *options);
				usage();
			}
			options++;
		}
		argc--, argv++;
	}
	if (all) {
		if (argc != 1) {
			usage();
		}
		mnttab = setmntent(MNTTAB, "r");
		if (mnttab == NULL) {
			fprintf(stderr, "mount: ");
			perror(MNTTAB);
			exit(1);
		}
		while ((mntp = getmntent(mnttab)) != NULL) {
			if ((strcmp(mntp->mnt_type, MNTTYPE_IGNORE) == 0) ||
			    (strcmp(mntp->mnt_type, MNTTYPE_SWAP) == 0) ||
			    (strcmp(mntp->mnt_dir, "/") == 0) ) {
				continue;
			}
			if (type[0] != '\0' &&
			    strcmp(mntp->mnt_type, type) != 0) {
				continue;
			}
			mountfs(mntp);
		}
		exit(0);
	}

	/*
	 * Command looks like: mount <dev>|<dir>
	 * we walk through /etc/fstab til we match either fsname or dir.
	 */
	if (argc == 2) {
		mnttab = setmntent(MNTTAB, "r");
		if (mnttab == NULL) {
			fprintf(stderr, "mount: ");
			perror(MNTTAB);
			exit(1);
		}
		while ((mntp = getmntent(mnttab)) != NULL) {
			if ((strcmp(mntp->mnt_type, MNTTYPE_IGNORE) == 0) ||
			    (strcmp(mntp->mnt_type, MNTTYPE_SWAP) == 0) ) {
				continue;
			}
			if ((strcmp(mntp->mnt_fsname, argv[1]) == 0) ||
			    (strcmp(mntp->mnt_dir, argv[1]) == 0) ) {
				mountfs(mntp);
				exit(0);
			}
		}
		fprintf(stderr, "mount: %s not found in %s\n", argv[1], MNTTAB);
		exit(0);
	}

	if (argc != 3) {
		usage();
	}
	strcpy(dir, argv[2]);
	strcpy(name, argv[1]);

	/*
	 * Check for file system names of the form
	 *     host:path
	 * make these type nfs
	 */
	colon = index(name, ':');
	if (colon) {
		if (type[0] != '\0' && strcmp(type, "nfs") != 0) {
			fprintf(stderr,"%s: %s; must use type nfs\n",
			    "mount: remote file system", name);
			usage();
		}
		strcpy(type, MNTTYPE_NFS);
	}
	if (type[0] == '\0') {
		strcpy(type, MNTTYPE_42);		/* default type = 4.2 */
	}
	if (dir[0] != '/') {
		fprintf(stderr, "mount: directory path must begin with '/'\n");
		exit(1);
	}

	if (opts[0] == '\0') {
		strcpy(opts, ro ? MNTOPT_RO : MNTOPT_RW);
		strcat(opts, ",");
		strcat(opts, quota ? MNTOPT_QUOTA : MNTOPT_NOQUOTA);
		if (strcmp(type, MNTTYPE_NFS) == 0) {
			strcat(opts, ",");
			strcat(opts, hard ? MNTOPT_HARD : MNTOPT_SOFT);
		}
	}

	if (strcmp(type, MNTTYPE_NFS) == 0) {
		passno = 0;
		freq = 0;
	}

	mnt.mnt_fsname = name;
	mnt.mnt_dir = dir;
	mnt.mnt_type = type;
	mnt.mnt_opts = opts;
	mnt.mnt_freq = freq;
	mnt.mnt_passno = passno;
	mountfs(&mnt);
}

mountfs(mnt)
	struct mntent *mnt;
{
	int error;

	if (mounted(mnt)) {
		fprintf(stderr, "mount: %s already mounted\n",
		    mnt->mnt_fsname);
		return;
	}
	if (fake) {
		addtomtab(mnt);
		return;
	}
	if (strcmp(mnt->mnt_type, MNTTYPE_42) == 0) {
		error = mount_42(mnt);
	} else if (strcmp(mnt->mnt_type, MNTTYPE_NFS) == 0) {
		error = mount_nfs(mnt);
#ifdef PCFS
	} else if (strcmp(mnt->mnt_type, MNTTYPE_PC) == 0) {
		error = mount_pc(mnt);
#endif
	} else {
		fprintf(stderr,
		    "mount: unknown file system type %s\n",
		    mnt->mnt_type);
	}

	if (error) {
		fprintf(stderr, "mount: %s on ", mnt->mnt_fsname);
		perror(mnt->mnt_dir);
	}
}

mount_42(mnt)
	struct mntent *mnt;
{
	int ro;

	ro = (hasmntopt(mnt, MNTOPT_RO) == NULL) ? 0 : 1;
	if (mount(mnt->mnt_fsname, mnt->mnt_dir, ro) < 0) {
		return (1);
	}
	addtomtab(mnt);
	return (0);
}

mount_nfs(mnt)
	struct mntent *mnt;
{
	int ro;
	int hard;
	struct sockaddr_in sin;
	struct hostent *hp;
	struct fhstatus fhs;
	int err;
	char *cp;
	char *hostp = host;
	char *path;
	int s = -1;
	struct timeval timeout;
	CLIENT *client;
	enum clnt_stat rpc_stat;
	struct stat sb;
	int printed1 = 0;
	int printed2 = 0;
	int bkg = 0;
#ifdef INST
	unsigned winks = 60;  /* sleep for a minute. */
#else !INST
	unsigned winks = 0;  /* seconds of sleep time */
#endif INST

	cp = mnt->mnt_fsname;
	while ((*hostp = *cp) != ':') {
		if (*cp == '\0') {
			fprintf(stderr,
			    "mount: nfs file system; use host:path\n");
			return (1);
		}
		hostp++;
		cp++;
	}
	*hostp = '\0';
	path = ++cp;
	/*
	 * Get server's address
	 */
	if ((hp = gethostbyname(host)) == NULL) {
		/*
		 * XXX
		 * Failure may be due to yellow pages, try again
		 */
		if ((hp = gethostbyname(host)) == NULL) {
			fprintf(stderr,
			    "mount: %s not in hosts database\n", host);
			return (1);
		}
	}

	ro = (hasmntopt(mnt, MNTOPT_RO) == NULL) ? 0 : 1;
	hard = (hasmntopt(mnt, MNTOPT_HARD) == NULL) ? 0 : 1;

	/*
	 * get fhandle of remote path from server's mountd
	 */
	do {
		bzero(&sin, sizeof(sin));
		bcopy(hp->h_addr, (char *) & sin.sin_addr, hp->h_length);
		sin.sin_family = AF_INET;
#ifdef INST
		/*
		 *  wait 60 seconds for reply.
		 */
		timeout.tv_usec = 0;
		timeout.tv_sec = 60;
#else !INST
		timeout.tv_usec = 0;
		timeout.tv_sec = 10;
#endif INST
		s = -1;
		do {
			if ((client = clntudp_create(&sin, MOUNTPROG, MOUNTVERS,
			    timeout, &s)) == NULL) {
				sleep(winks);
				if (winks < MAXSLEEP)
#ifdef INST
					/*
					 *  add another minute to sleep time.
					 */
					winks += 60;
#else !INST
					winks++;
#endif INST
				if (!printed1++) {
					fprintf(stderr,
					    "mount: %s server not responding",
					    mnt->mnt_fsname);
					clnt_pcreateerror("");
				}
				if (!hard && !bkg) {
					/*
					 * doing a soft mount, print a message
					 * and try forever in the background.
					 */
					fprintf(stderr,"mount: %s\n",
					  "retrying soft mount in background");
					switch (fork()) {
					case -1:
						perror("mount: cannot fork");
						return (1);
					case 0:
						/*
						 * CHILD: retry mount until it
						 * works
						 */
						bkg++;
						continue;
					default:
						/*
						 * PARENT: we are done, return
						 * ok status but don't update
						 * mtab.
						 */
						return (0);
					}
				}
			}
		} while (client == NULL);
		client->cl_auth = authunix_create_default();
#ifdef INST
		/*
		 *  wait two minutes for reply.
		 */
		timeout.tv_usec = 0;
		timeout.tv_sec = 120;
#else !INST
		timeout.tv_usec = 0;
		timeout.tv_sec = 25;
#endif INST
		rpc_stat = clnt_call(client, MOUNTPROC_MNT, xdr_path, &path,
		    xdr_fhstatus, &fhs, timeout);
		if (rpc_stat != RPC_SUCCESS) {
			if (!printed2++) {
				fprintf(stderr,
				    "mount: %s server not responding",
				    mnt->mnt_fsname);
				clnt_perror(client, "");
			}
			if (!hard && !bkg) {
				/*
				 * doing a soft mount, print a message
				 * and try forever in the background.
				 */
				fprintf(stderr,
				  "mount: retrying soft mount in background\n");
				switch (fork()) {
				case -1:
					perror("mount: cannot fork");
					return (1);
				case 0:
					/*
					 * CHILD: retry mount until it
					 * works
					 */
					bkg++;
					continue;
				default:
					/*
					 * PARENT: we are done, return
					 * ok status but don't update
					 * mtab.
					 */
					return (0);
				}
			}
		}
		close(s);
		clnt_destroy(client);
	} while (rpc_stat == RPC_TIMEDOUT || fhs.fhs_status == ETIMEDOUT);

	if (rpc_stat != RPC_SUCCESS || fhs.fhs_status) {
		errno = fhs.fhs_status;
		if (errno == EACCES) {
			fprintf(stderr, "mount: not in export list for %s:%s\n",
			    host, path);
		}
		return (1);
	}
	if (printed1 || printed2) {
		fprintf(stderr, "mount: %s server ok\n", mnt->mnt_fsname);
	}

	/*
	 * remote mount the fhandle on the local path
	 */
	if (nfsmount(&sin, &fhs.fhs_fh, mnt->mnt_dir, ro, hard) < 0) {
		return (1);
	}
	addtomtab(mnt);
	if (bkg) {
		/*
		 * We are a forked off child left around to do this one
		 * mount, so now we are done, let's die.
		 */
		exit(0);
	}
	return (0);
}

#ifdef PCFS
mount_pc(mnt)
	struct mntent *mnt;
{
	int ro;

	ro = (hasmntopt(mnt, MNTOPT_RO) == NULL) ? 0 : 1;
	if (pcfs_mount(mnt->mnt_fsname, mnt->mnt_dir, ro) < 0) {
		return (1);
	}
	addtomtab(mnt);
	return (0);
}
#endif

printent(mnt)
	struct mntent *mnt;
{
	fprintf(stdout, "%s on %s type %s (%s)\n",
	    mnt->mnt_fsname, mnt->mnt_dir, mnt->mnt_type, mnt->mnt_opts);
}

prtmntent(mnttabp, mnt)
	FILE *mnttabp;
	register struct mntent *mnt;
{
	fprintf(mnttabp, "%-20.20s %-12.12s %-6.6s %-16.16s %d %d\n",
	    mnt->mnt_fsname,
	    mnt->mnt_dir,
	    mnt->mnt_type,
	    mnt->mnt_opts,
	    mnt->mnt_freq,
	    mnt->mnt_passno);
	return (0);
}

/*
 * Check to see if mntck is already mounted.
 * We have to be careful because getmntent modifies its static struct.
 */
mounted(mntck)
	struct mntent *mntck;
{
	int found = 0;
	struct mntent *mnt, mntsave;
	FILE *mnttab;

	mnttab = setmntent(MOUNTED, "r");
	if (mnttab == NULL) {
		fprintf(stderr, "mount: ");
		perror(MOUNTED);
		exit(1);
	}
	mntcp(mntck, &mntsave);
	while ((mnt = getmntent(mnttab)) != NULL) {
		if (strcmp(mnt->mnt_type, MNTTYPE_IGNORE) == 0) {
			continue;
		}
		if ((strcmp(mntsave.mnt_fsname, mnt->mnt_fsname) == 0) &&
		    (strcmp(mntsave.mnt_dir, mnt->mnt_dir) == 0) ) {
			found = 1;
			break;
		}
	}
	endmntent(mnttab);
	*mntck = mntsave;
	return (found);
}

mntcp(mnt1, mnt2)
	struct mntent *mnt1, *mnt2;
{
	static char fsname[128], dir[128], type[128], opts[128];

	mnt2->mnt_fsname = fsname;
	strcpy(fsname, mnt1->mnt_fsname);
	mnt2->mnt_dir = dir;
	strcpy(dir, mnt1->mnt_dir);
	mnt2->mnt_type = type;
	strcpy(type, mnt1->mnt_type);
	mnt2->mnt_opts = opts;
	strcpy(opts, mnt1->mnt_opts);
	mnt2->mnt_freq = mnt1->mnt_freq;
	mnt2->mnt_passno = mnt1->mnt_passno;
}

/*
 * update /etc/mtab
 */
addtomtab(mnt)
	struct mntent *mnt;
{
	FILE *mnted;

	mnted = setmntent(MOUNTED, "r+");
	if (mnted == NULL) {
		fprintf(stderr, "mount: ");
		perror(MOUNTED);
		exit(1);
	}
	if (addmntent(mnted, mnt)) {
		fprintf(stderr, "mount: ");
		perror(MOUNTED);
		exit(1);
	}
	endmntent(mnted);

	if (verbose) {
		fprintf(stdout, "%s mounted on %s\n",
		    mnt->mnt_fsname, mnt->mnt_dir);
	}
}

usage()
{
	fprintf(stderr,
	    "Usage: mount [-ravpfto [type|option]] ... [fsname] [dir]\n");
	exit(1);
}
