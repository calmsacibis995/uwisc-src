/*	dtdebug.h */

struct	dt_trace {
	u_char	dtd_event;
	u_char	dtd_arg;
	int	dtd_time;
	union {
		struct dt_node	dtd_Node; /* for when arg is a node */
		struct dt_softc dtd_Soft; /* for info */
		struct dt_head  dtd_Head; 
		struct dt_sock	dtd_Sock;
		struct dt_param dtd_Param; /* 96 bytes, bigger than the rest */
		struct {
			int dtdm_2;
			int dtdm_3;
			int dtdm_4;
			int dtdm_5;
			char dtd_Str[50];
		} dtdmisc;
	} dtd_stuff;
};
#define dtd_node dtd_stuff.dtd_Node
#define dtd_soft dtd_stuff.dtd_Soft
#define dtd_head dtd_stuff.dtd_Head
#define dtd_param dtd_stuff.dtd_Param
#define dtd_sock dtd_stuff.dtd_Sock
#define dtd_str dtd_stuff.dtdmisc.dtd_Str
#define dtd_m2 dtd_stuff.dtdmisc.dtdm_2
#define dtd_m3 dtd_stuff.dtdmisc.dtdm_3
#define dtd_m4 dtd_stuff.dtdmisc.dtdm_4
#define dtd_m5 dtd_stuff.dtdmisc.dtdm_5

#define DTPTsendack	1
#define DTPTgotack	2
#define DTPTwstop	3
#define DTPTrstop	4
#define DTPTretrans	5
#define DTPTdrecv	6
#define DTPTlrecv	7
#define DTPTlsnd	8
#define DTPTdsnd	9
#define DTPTrstate	10
#define DTPTsstate	11
#define DTPTreject	12
#define DTPTrecst	13
#define DTPTsndst	14
#define DTPTheadin	15
#define DTPTheadout	16
#define DTPTmisc	17
#define DTPTinput	18
#define DTPTheadin2	19

#define DTTRACEN 150
int dttracen = 0;
struct dt_trace dt_trace[DTTRACEN];
