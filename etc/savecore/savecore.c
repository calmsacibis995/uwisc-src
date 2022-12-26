/*
 * Copyright (c) 1980,1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980,1986 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)savecore.c	5.8 (Berkeley) 5/26/86";
#endif not lint

/*
 * savecore
 */

#include <stdio.h>
#include <nlist.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/stat.h>
#ifdef VFS
#include <ufs/fs.h>
#else
#include <sys/fs.h>
#endif VFS
#include <sys/time.h>
#include <sys/file.h>
#include <sys/syslog.h>

#define	DAY	(60L*60L*24L)
#define	LEEWAY	(3*DAY)

#define eq(a,b) (!strcmp(a,b))
#ifdef vax
#define ok(number) ((number)&0x7fffffff)
#else
#define ok(number) (number)
#endif

struct nlist current_nl[] = {	/* namelist for currently running system */
#define X_DUMPDEV	0
	{ "_dumpdev" },
#define X_DUMPLO	1
	{ "_dumplo" },
#define X_TIME		2
	{ "_time" },
#define	X_DUMPSIZE	3
	{ "_dumpsize" },
#define X_VERSION	4
	{ "_version" },
#define X_PANICSTR	5
	{ "_panicstr" },
#define	X_DUMPMAG	6
	{ "_dumpmag" },
	{ "" },
};

struct nlist dump_nl[] = {	/* name list for dumped system */
	{ "_dumpdev" },		/* entries MUST be the same as */
	{ "_dumplo" },		/*	those in current_nl[]  */
	{ "_time" },
	{ "_dumpsize" },
	{ "_version" },
	{ "_panicstr" },
	{ "_dumpmag" },
	{ "" },
};

char	*system;
char	*dirname;			/* directory to save dumps in */
char	*ddname;			/* name of dump device */
char	*find_dev();
dev_t	dumpdev;			/* dump device */
time_t	dumptime;			/* time the dump was taken */
int	dumplo;				/* where dump starts on dumpdev */
int	dumpsize;			/* amount of memory dumped */
int	dumpmag;			/* magic number in dump */
time_t	now;				/* current date */
char	*path();
char	*malloc();
char	*ctime();
char	vers[80];
char	core_vers[80];
char	panic_mesg[80];
int	panicstr;
off_t	lseek();
off_t	Lseek();
int	Verbose;
extern	int errno;

main(argc, argv)
	char **argv;
	int argc;
{
	char *cp;

	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		for (cp = &argv[0][1]; *cp; cp++) switch (*cp) {

		case 'v':
			Verbose++;
			break;

		default:
		usage:
			fprintf(stderr,
			    "usage: savecore [-v] dirname [ system ]\n");
			exit(1);
		}
		argc--, argv++;
	}
	if (argc != 1 && argc != 2)
		goto usage;
	dirname = argv[0];
	if (argc == 2)
		system = argv[1];
	openlog("savecore", LOG_ODELAY, LOG_AUTH);
	if (access(dirname, W_OK) < 0) {
		int oerrno = errno;

		perror(dirname);
		errno = oerrno;
		syslog(LOG_ERR, "%s: %m", dirname);
		exit(1);
	}
	read_kmem();
	if (!dump_exists()) {
		if (Verbose)
			fprintf(stderr, "savecore: No dump exists.\n");
		exit(0);
	}
	(void) time(&now);
	check_kmem();
	if (panicstr)
		syslog(LOG_CRIT, "reboot after panic: %s", panic_mesg);
	else
		syslog(LOG_CRIT, "reboot");
	if (!get_crashtime() || !check_space())
		exit(1);
	save_core();
	clear_dump();
	exit(0);
}

dump_exists()
{
	register int dumpfd;
	int word;

	dumpfd = Open(ddname, O_RDONLY);
	Lseek(dumpfd, (off_t)(dumplo + ok(dump_nl[X_DUMPMAG].n_value)), L_SET);
	Read(dumpfd, (char *)&word, sizeof (word));
	close(dumpfd);
	if (Verbose && word != dumpmag) {
		printf("dumplo = %d (%d bytes)\n", dumplo/DEV_BSIZE, dumplo);
		printf("magic number mismatch: %x != %x\n", word, dumpmag);
	}
	return (word == dumpmag);
}

clear_dump()
{
	register int dumpfd;
	int zero = 0;

	dumpfd = Open(ddname, O_WRONLY);
	Lseek(dumpfd, (off_t)(dumplo + ok(dump_nl[X_DUMPMAG].n_value)), L_SET);
	Write(dumpfd, (char *)&zero, sizeof (zero));
	close(dumpfd);
}

char *
find_dev(dev, type)
	register dev_t dev;
	register int type;
{
	register DIR *dfd = opendir("/dev");
	struct direct *dir;
	struct stat statb;
	static char devname[MAXPATHLEN + 1];
	char *dp;

	strcpy(devname, "/dev/");
	while ((dir = readdir(dfd))) {
		strcpy(devname + 5, dir->d_name);
		if (stat(devname, &statb)) {
			perror(devname);
			continue;
		}
		if ((statb.st_mode&S_IFMT) != type)
			continue;
		if (dev == statb.st_rdev) {
			closedir(dfd);
			dp = malloc(strlen(devname)+1);
			strcpy(dp, devname);
			return (dp);
		}
	}
	closedir(dfd);
	fprintf(stderr, "Can't find device %d/%d\n", major(dev), minor(dev));
	syslog(LOG_ERR, "Can't find device %d/%d\n", major(dev), minor(dev));
	exit(1);
	/*NOTREACHED*/
}

int	cursyms[] =
    { X_DUMPDEV, X_DUMPLO, X_VERSION, X_DUMPMAG, -1 };
int	dumpsyms[] =
    { X_TIME, X_DUMPSIZE, X_VERSION, X_PANICSTR, X_DUMPMAG, -1 };
read_kmem()
{
	register char *cp;
	FILE *fp;
	char *dump_sys;
	int kmem, i;
	
	dump_sys = system ? system : "/vmunix";
	nlist("/vmunix", current_nl);
	nlist(dump_sys, dump_nl);
	/*
	 * Some names we need for the currently running system,
	 * others for the system that was running when the dump was made.
	 * The values obtained from the current system are used
	 * to look for things in /dev/kmem that cannot be found
	 * in the dump_sys namelist, but are presumed to be the same
	 * (since the disk partitions are probably the same!)
	 */
	for (i = 0; cursyms[i] != -1; i++)
		if (current_nl[cursyms[i]].n_value == 0) {
			fprintf(stderr, "/vmunix: %s not in namelist",
			    current_nl[cursyms[i]].n_name);
			syslog(LOG_ERR, "/vmunix: %s not in namelist",
			    current_nl[cursyms[i]].n_name);
			exit(1);
		}
	for (i = 0; dumpsyms[i] != -1; i++)
		if (dump_nl[dumpsyms[i]].n_value == 0) {
			fprintf(stderr, "%s: %s not in namelist", dump_sys,
			    dump_nl[dumpsyms[i]].n_name);
			syslog(LOG_ERR, "%s: %s not in namelist", dump_sys,
			    dump_nl[dumpsyms[i]].n_name);
			exit(1);
		}
	kmem = Open("/dev/kmem", O_RDONLY);
	Lseek(kmem, (long)current_nl[X_DUMPDEV].n_value, L_SET);
	Read(kmem, (char *)&dumpdev, sizeof (dumpdev));
	Lseek(kmem, (long)current_nl[X_DUMPLO].n_value, L_SET);
	Read(kmem, (char *)&dumplo, sizeof (dumplo));
	Lseek(kmem, (long)current_nl[X_DUMPMAG].n_value, L_SET);
	Read(kmem, (char *)&dumpmag, sizeof (dumpmag));
	dumplo *= DEV_BSIZE;
	ddname = find_dev(dumpdev, S_IFBLK);
	fp = fdopen(kmem, "r");
	if (fp == NULL) {
		syslog(LOG_ERR, "Couldn't fdopen kmem");
		exit(1);
	}
	if (system)
		return;
	fseek(fp, (long)current_nl[X_VERSION].n_value, L_SET);
	fgets(vers, sizeof (vers), fp);
	fclose(fp);
}

check_kmem()
{
	FILE *fp;
	register char *cp;

	fp = fopen(ddname, "r");
	if (fp == NULL) {
		int oerrno = errno;

		perror(ddname);
		errno = oerrno;
		syslog(LOG_ERR, "%s: %m", ddname);
		exit(1);
	}
	fseek(fp, (off_t)(dumplo+ok(dump_nl[X_VERSION].n_value)), L_SET);
	fgets(core_vers, sizeof (core_vers), fp);
	fclose(fp);
	if (!eq(vers, core_vers) && system == 0)
		fprintf(stderr,
		   "Warning: vmunix version mismatch:\n\t%sand\n\t%s",
		   vers, core_vers);
		syslog(LOG_WARNING,
		   "Warning: vmunix version mismatch: %s and %s",
		   vers, core_vers);
	fp = fopen(ddname, "r");
	fseek(fp, (off_t)(dumplo + ok(dump_nl[X_PANICSTR].n_value)), L_SET);
	fread((char *)&panicstr, sizeof (panicstr), 1, fp);
	if (panicstr) {
		fseek(fp, dumplo + ok(panicstr), L_SET);
		cp = panic_mesg;
		do
			*cp = getc(fp);
		while (*cp++);
	}
	fclose(fp);
}

get_crashtime()
{
	int dumpfd;
	time_t clobber = (time_t)0;

	dumpfd = Open(ddname, O_RDONLY);
	Lseek(dumpfd, (off_t)(dumplo + ok(dump_nl[X_TIME].n_value)), L_SET);
	Read(dumpfd, (char *)&dumptime, sizeof dumptime);
	close(dumpfd);
	if (dumptime == 0) {
		if (Verbose)
			printf("Dump time not found.\n");
		return (0);
	}
	printf("System went down at %s", ctime(&dumptime));
	if (dumptime < now - LEEWAY || dumptime > now + LEEWAY) {
		printf("dump time is unreasonable\n");
		return (0);
	}
	return (1);
}

char *
path(file)
	char *file;
{
	register char *cp = malloc(strlen(file) + strlen(dirname) + 2);

	(void) strcpy(cp, dirname);
	(void) strcat(cp, "/");
	(void) strcat(cp, file);
	return (cp);
}

check_space()
{
	struct stat dsb;
	register char *ddev;
	int dfd, spacefree;
	struct fs fs;

	if (stat(dirname, &dsb) < 0) {
		int oerrno = errno;

		perror(dirname);
		errno = oerrno;
		syslog(LOG_ERR, "%s: %m", dirname);
		exit(1);
	}
	ddev = find_dev(dsb.st_dev, S_IFBLK);
	dfd = Open(ddev, O_RDONLY);
	Lseek(dfd, (long)(SBLOCK * DEV_BSIZE), L_SET);
	Read(dfd, (char *)&fs, sizeof (fs));
	close(dfd);
 	spacefree = freespace(&fs, fs.fs_minfree) * fs.fs_fsize / 1024;
 	if (spacefree < read_number("minfree")) {
		printf("Dump omitted, not enough space on device");
		syslog(LOG_WARNING, "Dump omitted, not enough space on device");
		return (0);
	}
	if (freespace(&fs, fs.fs_minfree) < 0) {
		printf("Dump performed, but free space threshold crossed");
		syslog(LOG_WARNING,
		    "Dump performed, but free space threshold crossed");
	}
	return (1);
}

read_number(fn)
	char *fn;
{
	char lin[80];
	register FILE *fp;

	fp = fopen(path(fn), "r");
	if (fp == NULL)
		return (0);
	if (fgets(lin, 80, fp) == NULL) {
		fclose(fp);
		return (0);
	}
	fclose(fp);
	return (atoi(lin));
}

#define	BUFPAGES	(256*1024/NBPG)		/* 1/4 Mb */

save_core()
{
	register int n;
	register char *cp;
	register int ifd, ofd, bounds;
	register FILE *fp;

	cp = malloc(BUFPAGES*NBPG);
	if (cp == 0) {
		fprintf(stderr, "savecore: Can't allocate i/o buffer.\n");
		return;
	}
	bounds = read_number("bounds");
	ifd = Open(system?system:"/vmunix", O_RDONLY);
	sprintf(cp, "vmunix.%d", bounds);
	ofd = Create(path(cp), 0644);
	while((n = Read(ifd, cp, BUFSIZ)) > 0)
		Write(ofd, cp, n);
	close(ifd);
	close(ofd);
	ifd = Open(ddname, O_RDONLY);
	Lseek(ifd, (off_t)(dumplo + ok(dump_nl[X_DUMPSIZE].n_value)), L_SET);
	Read(ifd, (char *)&dumpsize, sizeof (dumpsize));
	sprintf(cp, "vmcore.%d", bounds);
	ofd = Create(path(cp), 0644);
	Lseek(ifd, (off_t)dumplo, L_SET);
	printf("Saving %d bytes of image in vmcore.%d\n", NBPG*dumpsize,
		bounds);
	syslog(LOG_NOTICE, "Saving %d bytes of image in vmcore.%d\n",
		NBPG*dumpsize, bounds);
	while (dumpsize > 0) {
		n = Read(ifd, cp,
		    (dumpsize > BUFPAGES ? BUFPAGES : dumpsize) * NBPG);
		if (n == 0) {
			syslog(LOG_WARNING,
			    "WARNING: vmcore may be incomplete\n");
			printf("WARNING: vmcore may be incomplete\n");
			break;
		}
		Write(ofd, cp, n);
		dumpsize -= n/NBPG;
	}
	close(ifd);
	close(ofd);
	fp = fopen(path("bounds"), "w");
	fprintf(fp, "%d\n", bounds+1);
	fclose(fp);
	free(cp);
}

/*
 * Versions of std routines that exit on error.
 */
Open(name, rw)
	char *name;
	int rw;
{
	int fd;

	fd = open(name, rw);
	if (fd < 0) {
		int oerrno = errno;

		perror(name);
		errno = oerrno;
		syslog(LOG_ERR, "%s: %m", name);
		exit(1);
	}
	return (fd);
}

Read(fd, buff, size)
	int fd, size;
	char *buff;
{
	int ret;

	ret = read(fd, buff, size);
	if (ret < 0) {
		int oerrno = errno;

		perror("read");
		errno = oerrno;
		syslog(LOG_ERR, "read: %m");
		exit(1);
	}
	return (ret);
}

off_t
Lseek(fd, off, flag)
	int fd, flag;
	long off;
{
	long ret;

	ret = lseek(fd, off, flag);
	if (ret == -1) {
		int oerrno = errno;

		perror("lseek");
		errno = oerrno;
		syslog(LOG_ERR, "lseek: %m");
		exit(1);
	}
	return (ret);
}

Create(file, mode)
	char *file;
	int mode;
{
	register int fd;

	fd = creat(file, mode);
	if (fd < 0) {
		int oerrno = errno;

		perror(file);
		errno = oerrno;
		syslog(LOG_ERR, "%s: %m", file);
		exit(1);
	}
	return (fd);
}

Write(fd, buf, size)
	int fd, size;
	char *buf;
{

	if (write(fd, buf, size) < size) {
		int oerrno = errno;

		perror("write");
		errno = oerrno;
		syslog(LOG_ERR, "write: %m");
		exit(1);
	}
}
