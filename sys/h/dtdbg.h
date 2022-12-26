
/* dtdbg.h  - debugger for hardware, protocol, selected parts of driver */

extern int dtdbgtime;
extern int dtdbgindx;
extern int dtdbgflag;

/*  debugging options */
#define DTMONF	0x01 /* general */
#define DTOCF	0x02 /* open/close */
#define DTPTF	0x04 /* protocol trace  -- not used now */


/* 
 * structures and constants for debugging output 
 * separated by purpose
 */


struct dt_mon {
	u_char dtm_event;
	u_char dtm_time;
	unsigned dtm_csr;
	unsigned dtm_wcr;
	unsigned dtm_flytime;
	struct dt_head dtm_head;
};

/* dtmon events */
#define DTM_RINT 1
#define DTM_XINT 2
#define DTM_INIT 3


struct dt_oc {
	int dtoc_time;
	int dtoc_event;
	int dtoc_sock;
	short dtoc_uid;
	short dtoc_pid;
	ino_t dtoc_cdir;
 	char dtoc_dirp[50+1];
};
#define DTOCopen 0
#define DTOCclose 1

/* 
 * THis will have to be bigger for the larger types of traces
 * and for when response is so bad that the user program can't
 * print fast enough
 */
#define DTNINDX 50 
				

union {
	struct dt_mon dtm;
	struct dt_oc dtoc;
} dtdbg[DTNINDX];
