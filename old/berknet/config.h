/*	@(#)config.h	4.4	(Berkeley)	1/5/83	*/

/*
	These are machine-configuration dependent
	tables.  To add a machine, be sure to update all
	these tables, add the "ifdef" entry in "mach.h",
	and add config? to gothru() in sub.c.
	For Berkeley, this file corresponds to the following network:

	The tables must be consistent.


	For RAND, these tables are:

		VAX (C) ---------GRAPHICS (A)------- TP (B)

	For NOSC, these tables are:

		   FCCMM ------ ATTS ------ MSSF ------ CCMM
				/ \
			       /   \
			      /     \
			     /       \
		OT34 ---- GATE40    ING70
			    |
			    |
			   PWB



*/
# ifdef RAND
/* GRAPHICS = A */
char configA[] = {		/* to get to i, config[i] */
	'a','b','c',000,000,		/* a,b,c,d,e */
	000,000,000,000,000,		/* f,g,h,i,j */
	000,000,000,000,000,		/* k,l,m,n,o */
	000,000,000,000,000,		/* p,q,r,s,t */
	000,000,000,000,000,		/* u,v,w,x,y */
	000,0				/* z */
	};
/* TP = B */
char configB[] = {		/* to get to i, config[i] */
	'a','b','a',000,000,		/* a,b,c,d,e */
	000,000,000,000,000,		/* f,g,h,i,j */
	000,000,000,000,000,		/* k,l,m,n,o */
	000,000,000,000,000,		/* p,q,r,s,t */
	000,000,000,000,000,		/* u,v,w,x,y */
	000,0				/* z */
	};
/* VAX = C */
char configC[] = {		/* to get to i, config[i] */
	'a','a','c',000,000,		/* a,b,c,d,e */
	000,000,000,000,000,		/* f,g,h,i,j */
	000,000,000,000,000,		/* k,l,m,n,o */
	000,000,000,000,000,		/* p,q,r,s,t */
	000,000,000,000,000,		/* u,v,w,x,y */
	000,0				/* z */
	};
/* if machtype is
	M_CC		netlpr will do lpr w/o an acct.
			Will pre-encrypt the password.
	M_INGRES	will allow higher file lengths.
	M_OTHER		will give no unusual effects.
(when in doubt, machtype should be M_OTHER)
*/
char machtype[]= {
	M_OTHER,M_OTHER,M_OTHER,0,0,	/* a,b,c,d,e */
	0, 0, 0, 0, 0, 			/* f,g,h,i,j */
	0, 0, 0, 0, 0,			/* k,l,m,n,o */
	0, 0, 0, 0, 0,			/* p,q,r,s,t */
	0, 0, 0, 0, 0,			/* u,v,w,x,y */
	0, 0};				/* z */

/* this is basically the default machine for each local machine */
char remtable[] = {
	'b','a','a',000,000,		/* a,b,c,d,e */
	000,000,000,000,000,		/* f,g,h,i,j */
	000,000,000,000,000,		/* k,l,m,n,o */
	000,000,000,000,000,		/* p,q,r,s,t */
	000,000,000,000,000,		/* u,v,w,x,y */
	000,0				/* z */
	};
/* bad login names */
struct bstruct btable[] = {
	0,0 };
/* this table shows the correspondence between
   machine names like 'Cory' and their internal
   names, like 'y' */
static struct tt {
	char *bigname;
	char lname;
	} table[] = {
	"Graphics",	'a',
	"TP",		'b',
	"VAX",		'c',
	0, 		0
	};
/* end of Rand definitions */

# endif RAND

# ifdef NOSC
/* Naval Ocean Systems Center */

/* atts (a) */
char configA[] = {		/* to get to i, config[i] */
	'a',000,'m',000,000,		/* a,b,c,d,e */
	'f','g',000,'i',000,		/* f,g,h,i,j */
	000,000,'m',000,'g',		/* k,l,m,n,o */
	'g',000,000,000,000,		/* p,q,r,s,t */
	000,000,000,000,000,		/* u,v,w,x,y */
	000,0				/* z */
	};

/* ccmm (c) */
char configC[] = {		/* to get to i, config[i] */
	'm',000,'c',000,000,		/* a,b,c,d,e */
	'm','m',000,'m',000,		/* f,g,h,i,j */
	000,000,'m',000,'m',		/* k,l,m,n,o */
	'm',000,000,000,000,		/* p,q,r,s,t */
	000,000,000,000,000,		/* u,v,w,x,y */
	000,0				/* z */
	};

/* ccmm (f) */
char configF[] = {		/* to get to i, config[i] */
	'a',000,'c',000,000,		/* a,b,c,d,e */
	'f','a',000,'a',000,		/* f,g,h,i,j */
	000,000,'a',000,'a',		/* k,l,a,n,o */
	'a',000,000,000,000,		/* p,q,r,s,t */
	000,000,000,000,000,		/* u,v,w,x,y */
	000,0				/* z */
	};

/* mssf (m) */
char configM[] = {		/* to get to i, config[i] */
	'a',000,'c',000,000,		/* a,b,c,d,e */
	'a','a',000,'a',000,		/* f,g,h,i,j */
	000,000,'m',000,'a',		/* k,l,m,n,o */
	'a',000,000,000,000,		/* p,q,r,s,t */
	000,000,000,000,000,		/* u,v,w,x,y */
	000,0				/* z */
	};

/* ingres (i) proposed */
char configI[] = {		/* to get to i, config[i] */
	'a',000,'a',000,000,		/* a,b,c,d,e */
	'a','a',000,'i',000,		/* f,g,h,i,j */
	000,000,'a',000,'a',		/* k,l,m,n,o */
	'a',000,000,000,000,		/* p,q,r,s,t */
	000,000,000,000,000,		/* u,v,w,x,y */
	000,0				/* z */
	};

/* nosc-cc gateway 40 (g) */
char configG[] = {		/* to get to i, config[i] */
	'a',000,'a',000,000,		/* a,b,c,d,e */
	'a','g',000,'a',000,		/* f,g,h,i,j */
	000,000,'a',000,'g',		/* k,l,m,n,o */
	'p',000,000,000,000,		/* p,q,r,s,t */
	000,000,000,000,000,		/* u,v,w,x,y */
	000,0				/* z */
	};

/* ocean tech 34 (o) */
char configO[] = {		/* to get to i, config[i] */
	'g',000,'g',000,000,		/* a,b,c,d,e */
	'g','g',000,'g',000,		/* f,g,h,i,j */
	000,000,'g',000,'o',		/* k,l,m,n,o */
	'g',000,000,000,000,		/* p,q,r,s,t */
	000,000,000,000,000,		/* u,v,w,x,y */
	000,0				/* z */
	};

/* pwb at nosc (p) */
char configP[] = {		/* to get to i, config[i] */
	'g',000,'g',000,000,		/* a,b,c,d,e */
	'g','g',000,'g',000,		/* f,g,h,i,j */
	000,000,'g',000,'g',		/* k,l,m,n,o */
	'p',000,000,000,000,		/* p,q,r,s,t */
	000,000,000,000,000,		/* u,v,w,x,y */
	000,0				/* z */
	};

/* this table is used by netlpr to do lpr w/o an acct
   and by net and netdaemon to do pre-emption */
/* sub.c uses the table in initdaemon to check machine
   type - errormsg may be ignored */
char machtype[]= {
	M_CC, 0,M_OTHER, 0, 0,	   	/* a,b,c,d,e */
	M_OTHER,M_OTHER, 0,M_INGRES, 0,	/* f,g,h,i,j */
	0, 0,M_CC, 0,M_OTHER,		/* k,l,m,n,o */
	M_OTHER, 0, 0, 0, 0,		/* p,q,r,s,t */
	0, 0, 0, 0, 0,			/* u,v,w,x,y */
	0};				/* z */
/* this is basically the default machine for each local machine */
char remtable[] = {
	'm',000,'m',000,000,		/* a,b,c,d,e */
	'a','a',000,'a',000,		/* f,g,h,i,j */
	000,000,'a',000,'g',		/* k,l,m,n,o */
	'g',000,000,000,000,		/* p,q,r,s,t */
	000,000,000,000,000,		/* u,v,w,x,y */
	000,0				/* z */
	};
/* bad login names */
struct bstruct btable[] = {
	"op", 'a',
	0,0 };
/* this table shows the correspondence between
   machine names like 'Cory' and their internal
   names, like 'y' */
static struct tt {
	char *bigname;
	char lname;
	} table[] = {
	"ATTS",		'a',
	"CCMM",		'c',
	"FCCMM",	'f',
	"MSSF",		'm',
	"INGRES",	'i',
	"GATEWAY",	'g',
	"OT34",		'o',
	"PWB",		'p',
	0, 0 };

# endif NOSC

# ifdef BERKELEY
/* Berkeley definitions */

/* Computer Center A Machine (A) */
char configA[] = {		/* to get to i, config[i] */
	'a','c','c','c','c',		/* a,b,c,d,e */
	'c','c','c','c','c',		/* f,g,h,i,j */
	'c','c','c','c','c',		/* k,l,m,n,o */
	'c',000,'c','c','c',		/* p,q,r,s,t */
	'c','c','c','c','c',		/* u,v,w,x,y */
	'c',0				/* z */
	};
/* Computer Center B Machine (B) */
char configB[] = {		/* to get to i, config[i] */
	'e','b','e','e','e',		/* a,b,c,d,e */
	'e','e','e','e','e',		/* f,g,h,i,j */
	'e','e','e','e','e',		/* k,l,m,n,o */
	'e',000,'e','e','e',		/* p,q,r,s,t */
	'e','e','e','e','e',		/* u,v,w,x,y */
	'e',0				/* z */
	};
/* Computer Center C Machine (C) */
char configC[] = {		/* to get to i, config[i] */
	'a','e','c','g','e',		/* a,b,c,d,e */
	'g','g','g','g','g',		/* f,g,h,i,j */
	'g','g','g','g','g',		/* k,l,m,n,o */
	'g',000,'g','e','g',		/* p,q,r,s,t */
	'g','g','g','g','g',		/* u,v,w,x,y */
	'g',0				/* z */
	};
/* Computer Center D Machine (D) */
char configD[] = {		/* to get to i, config[i] */
	'g','g','g','d','g',		/* a,b,c,d,e */
	'g','g','g','g','g',		/* f,g,h,i,j */
	'g','g','g','g','g',		/* k,l,m,n,o */
	'g',000,'g','g','g',		/* p,q,r,s,t */
	'g','g','g','g','g',		/* u,v,w,x,y */
	'g',0				/* z */
	};
/* Computer Center E Machine (E) */
char configE[] = {		/* to get to i, config[i] */
	'c','b','c','c','e',		/* a,b,c,d,e */
	'c','c','c','c','c',		/* f,g,h,i,j */
	'c','c','c','c','c',		/* k,l,m,n,o */
	'c',000,'c','s','c',		/* p,q,r,s,t */
	'c','c','c','c','c',		/* u,v,w,x,y */
	'c',0				/* z */
	};
/* Computer Center F Machine (F) */
char configF[] = {		/* to get to i, config[i] */
	'g','g','g','g','g',		/* a,b,c,d,e */
	'f','g','g','g','g',		/* f,g,h,i,j */
	'g','g','g','g','g',		/* k,l,m,n,o */
	'g',000,'g','g','g',		/* p,q,r,s,t */
	'g','g','g','g','g',		/* u,v,w,x,y */
	'g',0				/* z */
	};
/* Computer Center G Machine (G) */
char configG[] = {		/* to get to i, config[i] */
	'c','c','c','d','c',		/* a,b,c,d,e */
	'f','g','h','k','k',		/* f,g,h,i,j */
	'k','k','k','k','k',		/* k,l,m,n,o */
	'k',000,'k','c','k',		/* p,q,r,s,t */
	'k','k','k','k','k',		/* u,v,w,x,y */
	'k',0				/* z */
	};
/* Computer Center H Machine (H) */
char configH[] = {		/* to get to i, config[i] */
	'g','g','g','g','g',		/* a,b,c,d,e */
	'g','g','h','g','g',		/* f,g,h,i,j */
	'g','g','g','g','g',		/* k,l,m,n,o */
	'g',000,'g','g','g',		/* p,q,r,s,t */
	'g','g','g','g','g',		/* u,v,w,x,y */
	'g',0				/* z */
	};
/* Project INGRES 11/70 (Ing70) */
char configI[] = {		/* to get to i, config[i] */
	'j','j','j','j','j',		/* a,b,c,d,e */
	'j','j','j','i','j',		/* f,g,h,i,j */
	'j','j','j','j','j',		/* k,l,m,n,o */
	'j',000,'j','j','j',		/* p,q,r,s,t */
	'j','j','j','j','j',		/* u,v,w,x,y */
	'j',0				/* z */
	};
/* Project INGRES VAX (IngVAX) */
char configJ[] = {		/* to get to i, config[i] */
	'k','k','k','k','k',		/* a,b,c,d,e */
	'k','k','k','i','j',		/* f,g,h,i,j */
	'k','k','k','k','k',		/* k,l,m,n,o */
	'k',000,'k','k','k',		/* p,q,r,s,t */
	'k','k','k','k','k',		/* u,v,w,x,y */
	'k',0				/* z */
	};
/* UUCP gateway VAX (ucbvax) */
char configK[] = {		/* to get to i, config[i] */
	'g','g','g','g','g',		/* a,b,c,d,e */
	'g','g','g','j','j',		/* f,g,h,i,j */
	'k','p','p','n','p',		/* k,l,m,n,o */
	'p','g','r','g','t',		/* p,q,r,s,t */
	'u','v','w','x','y',		/* u,v,w,x,y */
	'p',0				/* z */
	};
/* Brodersen EECS VLSI VAX (VLSI) */
char configL[] = {		/* to get to i, config[i] */
	'm','m','m','m','m',		/* a,b,c,d,e */
	'm','m','m','m','m',		/* f,g,h,i,j */
	'm','l','m','m','m',		/* k,l,m,n,o */
	'm',000,'m','m','m',		/* p,q,r,s,t */
	'm','m','m','m','m',		/* u,v,w,x,y */
	'm',0				/* z */
	};
/* Sakrison's Image Project VAX (Image) */
char configM[] = {		/* to get to i, config[i] */
	'o','o','o','o','o',		/* a,b,c,d,e */
	'o','o','o','o','o',		/* f,g,h,i,j */
	'o','l','m','o','o',		/* k,l,m,n,o */
	'o',000,'o','o','o',		/* p,q,r,s,t */
	'o','o','o','o','o',		/* u,v,w,x,y */
	'o',0				/* z */
	};
/* Fatemans Applied Math VAX (Kim) */
char configN[] = {		/* to get to i, config[i] */
	'k','k','k','k','k',		/* a,b,c,d,e */
	'k','k','k','k','k',		/* f,g,h,i,j */
	'k','k','k','n','k',		/* k,l,m,n,o */
	'k',000,'k','k','k',		/* p,q,r,s,t */
	'k','k','k','k','k',		/* u,v,w,x,y */
	'k',0				/* z */
	};
/* Pfeister - Pollack - Sangiovanni Optimization VAX (ESVAX) */
char configO[] = {		/* to get to i, config[i] */
	'p','p','p','p','p',		/* a,b,c,d,e */
	'p','p','p','p','p',		/* f,g,h,i,j */
	'p','m','m','p','o',		/* k,l,m,n,o */
	'p',000,'p','p','p',		/* p,q,r,s,t */
	'p','p','p','p','p',		/* u,v,w,x,y */
	'z',0				/* z */
	};
/* Newton's CAD machine (VAX 11/780) */
char configP[] = {		/* to get to i, config[i] */
	'k','k','k','k','k',		/* a,b,c,d,e */
	'k','k','k','k','k',		/* f,g,h,i,j */
	'k','o','o','k','o',		/* k,l,m,n,o */
	'p',000,'k','k','k',		/* p,q,r,s,t */
	'k','k','k','k','k',		/* u,v,w,x,y */
	'o',0				/* z */
	};
/* Fabry's ARPA support VAX - ARPAVAX */
char configR[] = {		/* to get to i, config[i] */
	'k','k','k','k','k',		/* a,b,c,d,e */
	'k','k','k','k','k',		/* f,g,h,i,j */
	'k','k','k','k','k',		/* k,l,m,n,o */
	'k',000,'r','k','k',		/* p,q,r,s,t */
	'k','k','k','k','k',		/* u,v,w,x,y */
	'k',0				/* z */
	};
/* Survey Research Center 11/40 (SRC) */
char configS[] = {		/* to get to i, config[i] */
	'e','e','e','e','e',		/* a,b,c,d,e */
	'e','e','e','e','e',		/* f,g,h,i,j */
	'e','e','e','e','e',		/* k,l,m,n,o */
	'e',000,'e','s','e',		/* p,q,r,s,t */
	'e','e','e','e','e',		/* u,v,w,x,y */
	'e',0				/* z */
	};
/* Math-Stat Departement machine 11-45 (MathStat) */
char configT[] = {		/* to get to i, config[i] */
	'k','k','k','k','k',		/* a,b,c,d,e */
	'k','k','k','k','k',		/* f,g,h,i,j */
	'k','k','k','k','k',		/* k,l,m,n,o */
	'k',000,'k','k','t',		/* p,q,r,s,t */
	'k','k','k','k','k',		/* u,v,w,x,y */
	'k',0				/* z */
	};
/* ARPANET gateway (ucbc70) */
char configU[] = {		/* to get to i, config[i] */
	'k','k','k','k','k',		/* a,b,c,d,e */
	'k','k','k','k','k',		/* f,g,h,i,j */
	'k','k','k','k','k',		/* k,l,m,n,o */
	'k',000,'k','k','k',		/* p,q,r,s,t */
	'u','k','k','k','k',		/* u,v,w,x,p */
	'k',0				/* z */
	};
/* EECS Research (Fateman - Ernie) VAX (CSVAX) */
char configV[] = {		/* to get to i, config[i] */
	'k','k','k','k','k',		/* a,b,c,d,e */
	'k','k','k','k','k',		/* f,g,h,i,j */
	'k','k','k','k','k',		/* k,l,m,n,o */
	'k',000,'k','k','k',		/* p,q,r,s,t */
	'k','v','k','k','k',		/* u,v,w,x,p */
	'k',0				/* z */
	};
/* Statistics VAX 11/780 (ucbstatvax) */
char configW[] = {		/* to get to i, config[i] */
	'k','k','k','k','k',		/* a,b,c,d,e */
	'k','k','k','k','k',		/* f,g,h,i,j */
	'k','k','k','k','k',		/* k,l,m,n,o */
	'k',000,'k','k','k',		/* p,q,r,s,t */
	'k','k','w','k','k',		/* u,v,w,x,p */
	'k',0				/* z */
	};
/* Undergraduate Computing Facility Onyx Computer */
char configX[] = {		/* to get to i, config[i] */
	'k','k','k','k','k',		/* a,b,c,d,e */
	'k','k','k','k','k',		/* f,g,h,i,j */
	'k','k','k','k','k',		/* k,l,m,n,o */
	'k',000,'k','k','k',		/* p,q,r,s,t */
	'k','k','k','x','k',		/* u,v,w,x,y */
	'k',0				/* z */
	};
/* EECS Instructional 11/70 (199 Cory) (Cory) */
char configY[] = {		/* to get to i, config[i] */
	'k','k','k','k','k',		/* a,b,c,d,e */
	'k','k','k','k','k',		/* f,g,h,i,j */
	'k','k','k','k','k',		/* k,l,m,n,o */
	'k',000,'k','k','k',		/* p,q,r,s,t */
	'k','k','k','k','y',		/* u,v,w,x,y */
	'k',0				/* z */
	};
/* EECS Departmental 11/40  (EECS40) */
char configZ[] = {		/* to get to i, config[i] */
	'o','o','o','o','o',		/* a,b,c,d,e */
	'o','o','o','o','o',		/* f,g,h,i,j */
	'o','o','o','o','o',		/* k,l,m,n,o */
	'o',000,'o','o','o',		/* p,q,r,s,t */
	'o','o','o','o','o',		/* u,v,w,x,y */
	'z',0				/* z */
	};
/* if machtype is
	M_CC		netlpr will do lpr w/o an acct.
			Will pre-encrypt the password.
	M_INGRES	will allow higher file lengths.
	M_OTHER		will give no unusual effects.
(when in doubt, machtype should be M_OTHER)
*/
char machtype[]= {
	M_CC, M_CC, M_CC, M_CC, M_CC,			/* a,b,c,d,e */
	M_CC, M_CC, M_CC, M_INGRES, M_INGRES,		/* f,g,h,i,j */
	M_OTHER, M_OTHER, M_OTHER, M_OTHER, M_OTHER,	/* k,l,m,n,o */
	M_OTHER, 0, M_OTHER, M_OTHER, M_OTHER,		/* p,q,r,s,t */
	M_OTHER, M_OTHER, M_OTHER, M_OTHER, M_OTHER,	/* u,v,w,x,y */
	M_OTHER, 0};					/* z */

/* this is basically the default machine for each local machine */
char remtable[] = {
	'c','e','g','g','c',		/* a,b,c,d,e */
	'g','k','g','j','k',		/* f,g,h,i,j */
	'u','m','o','k','p',		/* k,l,m,n,o */
	'k',000,'k','e','k',		/* p,q,r,s,t */
	'k','k','k','k','k',		/* u,v,w,x,y */
	'o',0				/* z */
	};
/* bad login names */
struct bstruct btable[] = {
	"op", 'a',
	0,0 };
/* this table shows the correspondence between
   machine names like 'Cory' and their internal names, like 'y' */
static struct tt {
	char *bigname;
	char lname;
	} table[] = {
	"A",		'a',
	"ucbcfo-a",	'a',
	"cfo-a",	'a',
	"B",		'b',
	"ucbcfo-b",	'b',
	"cfo-b",	'b',
	"C",		'c',
	"ucbcfo-c",	'c',
	"cfo-c",	'c',
	"D",		'd',
	"ucbcfo-d",	'd',
	"cfo-d",	'd',
	"E",		'e',
	"ucbcfo-e",	'e',
	"cfo-e",	'e',
	"F",		'f',
	"ucbcfo-f",	'f',
	"cfo-f",	'f',
	"G",		'g',
	"ucbcfo-g",	'g',
	"cfo-g",	'g',
	"H",		'h',
	"ucbcfo-h",	'h',
	"cfo-h",	'h',
	"ucbjade",	'h',
	"jade",		'h',
	"Ing70",	'i',
	"ucberl70",	'i',
	"erl70",	'i',
	"IngVAX",	'j',
	"ucbingres",	'j',
	"ingres",	'j',
	"ucbvax",	'k',
	"vax",		'k',
	"ucboz",	'l',
	"oz",		'l',
	"VLSI",		'l',
	"ucbmedea",	'm',
	"medea",	'm',
	"Image",	'm',
	"Kim",		'n',
	"ucbkim",	'n',
	"kim",		'n',
	"ESVAX",	'o',
	"ucbesvax",	'o',
	"ucbcad",	'p',
	"ucbcoke",	'p',
	"coke",		'p',
	"cad",		'p',
	"ARPAVAX",	'r',
	"ucbarpa",	'r',
	"arpa",		'r',
	"SRC",		's',
	"ucbsrc",	's',
	"MathStat",	't',
	"ucbmathstat",	't',
	"ucbc70",	'u',
	"c70",		'u',
	"CSVAX",	'v',
	"ucbernie",	'v',
	"ernie",	'v',
	"ucbstatvax",	'w',
	"statvax",	'w',
	"ucbonyx",	'x',
	"onyx",		'x',
	"Cory",		'y',
	"ucbcory",	'y',
	"ucbear",	'z',
	"ear",		'z',
	"earvax",	'z',
	"EECS40",	'z',
	0, 		0
	};
# endif
