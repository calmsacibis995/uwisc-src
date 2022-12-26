/*
 * dtmon.h
 * monitor code for debugging hardware
 */


struct dt_mon {
	u_char dtm_event;
	u_char dtm_time;
	unsigned dtm_csr;
	unsigned dtm_wcr;
	unsigned dtm_flytime;
	struct dt_head dtm_head;
};


extern struct dt_mon dtmon[];
extern int dtmontime;

#define DTNMON 75

/* events */
#define DTM_RINT 1
#define DTM_XINT 2
#define DTM_INIT 3

#define DTMONGEN 0x20 /* general */
#define DTMONOC  0x5f /* open/close */
