/*
 * fsleep( d ) -- Sleep for d seconds, where d is a double.  Like sleep()
 *	but with a finer grain.  Uses the 4.2 BSD interval timer.
 *
 * Author:	Donn Seeley, UCSD Chemistry Dept., sdchema!donn
 * Date:	Wed Apr 25 03:28:14 PST 1984
 * Remarks:
 *	If an alarm clock has gone off for the caller and alarms are being
 *		blocked, the alarm is lost.  You'd think with all those new
 *		system calls, there would be a way to find out what signals
 *		have arrived without actually taking them...
 *	You must define SLEEPBUG when compiling to get the 'feature' of
 *		sleep() that signals other than alarms won't terminate
 *		the sleep.
 */

# include	<signal.h>
# include	<sys/time.h>

# define	mask(s)		(1 << ((s) - 1))

# ifdef	SLEEPBUG
static int	beep;
# endif	SLEEPBUG

fsleep( d )
	double			d;
{
	struct itimerval	oldalarm, newalarm;
	struct sigvec		oldvec, newvec;
	int			_fsleep_handler();
	int			savemask;
	int			wasblocked	= 0;
	int			alarmpending	= 0;

	/*
	 * Sanity check.
	 */
	if ( d <= 0.0 )
		return;

	/*
	 * Block the alarm signal.
	 */
	savemask	= sigblock( mask( SIGALRM ) );
	wasblocked	= savemask & mask( SIGALRM );

	/*
	 * Get the old alarm time, set up the new one.
	 */
	newalarm.it_value.tv_sec	= (long) d;
	newalarm.it_value.tv_usec	=
		(long) ((d - (double) newalarm.it_value.tv_sec) * 1000000.0);
	timerclear( &newalarm.it_interval );
	if ( setitimer( ITIMER_REAL, &newalarm, &oldalarm ) < 0 ) {
		/*
		 * Illegal sleep time.
		 */
		(void) setitimer( ITIMER_REAL, &oldalarm, (struct itimerval *) 0 );
		(void) sigsetmask( savemask );
		return;
	}

	/*
	 * Was there an alarm out there already?
	 */
	if ( timerisset( &oldalarm.it_value ) )
		++alarmpending;
	else if ( timerisset( &oldalarm.it_interval ) ) {
		/*
		 * Paranoia.
		 */
		++alarmpending;
		oldalarm.it_value.tv_usec	= 11000;
	}

	/*
	 * If an old alarm would come while we slept, shorten the sleep.
	 */
	if ( alarmpending &&
	     ! timercmp( &oldalarm.it_value, &newalarm.it_value, > ) ) {
		(void) setitimer( ITIMER_REAL, &oldalarm, (struct itimerval *) 0 );
		if ( ! wasblocked ) {
			/*
			 * Just take the old alarm and return.
			 */
			sigpause( savemask );
			(void) sigsetmask( savemask );
			return;
		}
		/*
		 * Alarms are being blocked by the caller -- arrange to queue
		 * an alarm for the caller when the sleep finishes.
		 */
		newalarm.it_value	= oldalarm.it_value;
		{
			struct timeval	t;

			timerclear( &t );
			t.tv_usec		= 11000;
			timevaladd( &oldalarm.it_value, &t );
		}
	}

	/*
	 * Get the old signal handler, set up the new one.
	 */
	newvec.sv_handler	= _fsleep_handler;
	newvec.sv_mask		= savemask;
	newvec.sv_onstack	= 0;
	(void) sigvec( SIGALRM, &newvec, &oldvec );

	/*
	 * Pause for the alarm.
	 */
# ifdef	SLEEPBUG
	/*
	 * Duplicate disgusting sleep() behavior of only breaking for alarms.
	 */
	beep			= 0;
	while ( ! beep )
# endif	SLEEPBUG
		sigpause( savemask & ~ mask( SIGALRM ) );

	/*
	 * Reset the old signal handler and alarm.
	 */
	sigvec( SIGALRM, &oldvec, (struct sigvec *) 0 );
	if ( alarmpending ) {
		timevalsub( &oldalarm.it_value, &newalarm.it_value );
		(void) setitimer( ITIMER_REAL, (struct itimerval *) 0, &newalarm );
		timevaladd( &oldalarm.it_value, &newalarm.it_value );
		(void) setitimer( ITIMER_REAL, &oldalarm, (struct itimerval *) 0 );
	}

	/*
	 * Reset the mask and return.  This unblocks the alarm if it was
	 *	unblocked previously, allowing any old alarm to arrive.
	 */
	(void) sigsetmask( savemask );
	return;
}



/*
 * _fsleep_handler() -- Dummy routine for handling alarms.
 */
int _fsleep_handler()
{
# ifdef	SLEEPBUG
	++beep;
# endif	SLEEPBUG
	return 0;
}



/*
 * Add and subtract routines for timevals.
 * Stolen from the kernel.
 *
 * N.B.: subtract routine doesn't deal with
 * results which are before the beginning,
 * it just gets very confused in this case.
 * Caveat emptor.
 */
static
timevaladd(t1, t2)
	struct timeval *t1, *t2;
{

	t1->tv_sec += t2->tv_sec;
	t1->tv_usec += t2->tv_usec;
	timevalfix(t1);
}

static
timevalsub(t1, t2)
	struct timeval *t1, *t2;
{

	t1->tv_sec -= t2->tv_sec;
	t1->tv_usec -= t2->tv_usec;
	timevalfix(t1);
}

static
timevalfix(t1)
	struct timeval *t1;
{

	if (t1->tv_usec < 0) {
		t1->tv_sec--;
		t1->tv_usec += 1000000;
	}
	if (t1->tv_usec >= 1000000) {
		t1->tv_sec++;
		t1->tv_usec -= 1000000;
	}
}

