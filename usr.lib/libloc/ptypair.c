#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/ioctl.h>

/*
 * ptypair
 *   works like pipe() or socketpair(), but returns a master/slave
 *   pty pair.  The master is fd[0], the slave is fd[1].  The slave
 *   end is the end that actually looks like a tty.  The pty is
 *   returned in RAW/NO-PARITY mode and both ends are open read/write.
 *
 * returns 0 if it found a pty, -1 if not.
 */
int
ptypair(fd)
	int fd[2];
{
	register i;
	register char *c, *line;
	struct stat stb;
	struct sgttyb sb;
	static char ptymask[] ="/dev/ptyXX";
	extern int errno;
#define PTYMASK_LEN (sizeof(ptymask)-1)

	for (c = "pqrs"; *c != 0; c++) {
		line = ptymask;
		line[PTYMASK_LEN-2] = *c;
		line[PTYMASK_LEN-1] = '0';

		if (stat(line, &stb) < 0)	/* see if the block of ptys exists */
			break;

		for (i = 0; i < 16; i++) {
			line[PTYMASK_LEN-1] = "0123456789abcdef"[i];
			fd[0] = open(line, O_RDWR);
			if (fd[0] > 0) {
				line[PTYMASK_LEN-5] = 't';
				fd[1] = open(line, O_RDWR);
				if(fd[1] < 0) {	/* if tty open fails, try another */
					(void) close(fd[0]);
					continue;
				}

				/* now, make it sane */
				(void) ioctl(fd[1], (int) TIOCGETP, (char *) &sb);
				sb.sg_ispeed = EXTA;
				sb.sg_ospeed = EXTA;
				sb.sg_flags = RAW|ANYP;
				(void) ioctl(fd[1], (int) TIOCSETP, (char *) &sb);
				return 0;
			}
		}
	}

	errno = ENOSPC;		/* no space left on device -- it's close */
	return -1;
}
