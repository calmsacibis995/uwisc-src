/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)defs.h	5.3 (Berkeley) 1/10/86
 */

/*
 * defs.h
 *
 * Definitions for f77pass1
 *
 * University of Utah CS Dept modification history:
 *
 * $Log:	defs.h,v $
 * Revision 1.1  86/08/26  20:52:04  root
 * Original BSD4.3 Source, as Distributed
 * 
 * Revision 5.5  86/01/10  17:11:11  donn
 * Add a vparamval field to Nameblock struct -- fix up klugy sharing between
 * Paramblock and Nameblock structs.
 * 
 * Revision 5.4  85/11/25  00:23:47  donn
 * 4.3 beta
 * 
 * Revision 5.3  85/08/10  06:31:09  donn
 * Added missing definition for intrconv().
 * 
 * Revision 5.2  85/08/10  04:01:53  donn
 * Jerry Berkman's change to add a definition for 'dblflag' and to ifdef
 * the Fortran 66 compatibility flags.
 * 
 * Revision 5.1  85/08/10  03:44:01  donn
 * 4.3 alpha
 * 
 * Revision 2.3  85/06/07  21:06:48  root
 * Add copyright
 * 
 * Revision 2.2  85/01/11  21:02:58  donn
 * Added 'issaved' bit to addrblocks to help implement SAVE statements.
 * 
 * Revision 2.1  84/07/19  12:02:46  donn
 * Changed comment headers for UofU.
 * 
 * Revision 1.2  84/02/27  00:49:40  donn
 * Added external references to lists of argument temporaries, used in the
 * Berkeley changes to reduce offsets of data.
 * 
 */

#include <stdio.h>

#ifdef unix
#	include <ctype.h>
#endif

#include "ftypes.h"
#include "defines.h"
#include "machdefs.h"

#define VL 16

#define MAXDIM 20
#define MAXINCLUDES 10
#define MAXLITERALS 30
#define MAXCTL 20
#define MAXHASH 1009
#define MAXSTNO 401
#define MAXEXT 200
#define MAXEQUIV 150
#define MAXLABLIST 256
#define MAXDEBUGFLAG 20

typedef union Expression *expptr;
typedef union Taggedblock *tagptr;
typedef struct Chain *chainp;
typedef struct Addrblock *Addrp;
typedef struct Tempblock *Tempp;
typedef struct Constblock *Constp;
typedef struct Exprblock *Exprp;
typedef struct Nameblock *Namep;

extern FILEP infile;
extern FILEP diagfile;
extern FILEP textfile;
extern FILEP asmfile;
extern FILEP initfile;
extern long int headoffset;

extern char token [ ];
extern int toklen;
extern int lineno;
extern char *infname;
extern int needkwd;
extern struct Labelblock *thislabel;

extern int maxctl;
extern int maxequiv;
extern int maxstno;
extern int maxhash;
extern int maxext;

extern flag profileflag;
extern flag dblflag;
extern flag optimflag;
extern flag nowarnflag;
extern flag ftn66flag;
#ifdef ONLY66
extern flag no66flag;
extern flag noextflag;
#endif
extern flag shiftcase;
extern flag undeftype;
extern flag shortsubs;
extern flag onetripflag;
extern flag checksubs;
extern flag debugflag[];
extern flag equivdcl;
extern int nerr;
extern int nwarn;
extern int ndata;

extern int parstate;
extern flag headerdone;
extern int blklevel;
extern flag saveall;
extern flag substars;
extern int impltype[ ];
extern int implleng[ ];
extern int implstg[ ];

extern int tyint;
extern int tylogical;
extern ftnint typesize[];
extern int typealign[];
extern int procno;
extern int proctype;
extern char * procname;
extern int rtvlabel[ ];
extern int fudgelabel;	/* to confuse the pdp11 optimizer */
extern Addrp typeaddr;
extern Addrp retslot;
extern int cxslot;
extern int chslot;
extern int chlgslot;
extern int procclass;
extern ftnint procleng;
extern int nentry;
extern flag multitype;
extern int blklevel;
extern int lastlabno;
extern int lastvarno;
extern int lastargslot;
extern int argloc;
extern ftnint autoleng;
extern ftnint bssleng;
extern int retlabel;
extern int ret0label;
extern int dorange;
extern int regnum[ ];
extern Namep regnamep[ ];
extern int maxregvar;
extern int highregvar;
extern int nregvar;
extern ftnint lowbss;
extern ftnint highbss;
extern int bsslabel;
extern flag anyinits;
extern flag anylocals;

extern chainp templist;
extern chainp argtemplist;
extern chainp activearglist;
extern int maxdim;
extern chainp holdtemps;
extern struct Entrypoint *entries;
extern struct Rplblock *rpllist;
extern struct Chain *curdtp;
extern ftnint curdtelt;
extern flag toomanyinit;

extern flag inioctl;
extern int iostmt;
extern Addrp ioblkp;
extern int nioctl;
extern int nequiv;
extern int eqvstart;	/* offset to eqv number to guarantee uniqueness */
extern int nintnames;

#ifdef SDB
extern int dbglabel;
extern flag sdbflag;
#endif

extern int cdatafile;
extern int cchkfile;
extern int vdatafile;
extern int vchkfile;

extern char cdatafname[];
extern char cchkfname[];
extern char vdatafname[];
extern char vchkfname[];

extern long cdatahwm;
extern long vdatahwm;


struct Chain
	{
	chainp nextp;
	tagptr datap;
	};

extern chainp chains;

struct Headblock
	{
	field tag;
	field vtype;
	field vclass;
	field vstg;
	expptr vleng;
	} ;

struct Ctlframe
	{
	unsigned ctltype:8;
	unsigned dostepsign:8;
	int ctlabels[4];
	int dolabel;
	Namep donamep;
	expptr domax;
	expptr dostep;
	};
#define endlabel ctlabels[0]
#define elselabel ctlabels[1]
#define dobodylabel ctlabels[1]
#define doposlabel ctlabels[2]
#define doneglabel ctlabels[3]
extern struct Ctlframe *ctls;
extern struct Ctlframe *ctlstack;
extern struct Ctlframe *lastctl;

struct Extsym
	{
	char extname[XL+1];
	field extstg;
	unsigned extsave:1;
	unsigned extinit:1;
	chainp extp;
	ftnint extleng;
	ftnint maxleng;
	char init;
	long initoffset;
	};

extern struct Extsym *extsymtab;
extern struct Extsym *nextext;
extern struct Extsym *lastext;

struct Labelblock
	{
	int labelno;
	unsigned blklevel:8;
	unsigned labused:1;
	unsigned labinacc:1;
	unsigned labdefined:1;
	unsigned labtype:2;
	ftnint stateno;
	};

extern struct Labelblock *labeltab;
extern struct Labelblock *labtabend;
extern struct Labelblock *highlabtab;

struct Entrypoint
	{
	struct Entrypoint *entnextp;
	struct Extsym *entryname;
	chainp arglist;
	int entrylabel;
	int typelabel;
	Namep enamep;
	};

struct Primblock
	{
	field tag;
	field vtype;
	Namep namep;
	struct Listblock *argsp;
	expptr fcharp;
	expptr lcharp;
	};


struct Hashentry
	{
	int hashval;
	Namep varp;
	};
extern struct Hashentry *hashtab;
extern struct Hashentry *lasthash;

struct Intrpacked	/* bits for intrinsic function description */
	{
	unsigned f1:3;
	unsigned f2:4;
	unsigned f3:7;
	};

struct Nameblock
	{
	field tag;
	field vtype;
	field vclass;
	field vstg;
	expptr vleng;
	char varname[VL];
	expptr vparamval;
	unsigned vdovar:1;
	unsigned vdcldone:1;
	unsigned vadjdim:1;
	unsigned vsave:1;
	unsigned vprocclass:3;
	unsigned vregno:4;
	union	{
		int varno;
		struct Intrpacked intrdesc;	/* bits for intrinsic function*/
		} vardesc;
	struct Dimblock *vdim;
	ftnint voffset;
	union	{
		chainp namelist;	/* points to chain of names in */
		chainp vstfdesc;	/* points to (formals, expr) pair */
		} varxptr;
	char inlcomm;
	char init;
	long initoffset;
	ftnint varsize;
	};


struct Paramblock
	{
	field tag;
	field vtype;
	field vclass;
	field vstg;
	expptr vleng;
	char varname[VL];
	expptr paramval;
	} ;


struct Exprblock
	{
	field tag;
	field vtype;
	field vclass;
	field vstg;
	expptr vleng;
	unsigned opcode:6;
	expptr leftp;
	expptr rightp;
	};


union Constant
	{
	char *ccp;
	ftnint ci;
	double cd[2];
	};

struct Constblock
	{
	field tag;
	field vtype;
	field vclass;
	field vstg;
	expptr vleng;
	union Constant const;
	};


struct Listblock
	{
	field tag;
	field vtype;
	chainp listp;
	};



struct Addrblock
	{
	field tag;
	field vtype;
	field vclass;
	field vstg;
	expptr vleng;
	int memno;
	expptr memoffset;
	unsigned istemp:1;
        unsigned isarray:1;
	unsigned issaved:1;
	unsigned ntempelt:10;
	ftnint varleng;
	ftnint varsize;
	};



struct Tempblock
	{
	field tag;
	field vtype;
	field vclass;
	expptr vleng;
	Addrp memalloc;
	unsigned istemp:1;
	unsigned ntempelt:10;
	ftnint varleng;
	};



struct Errorblock
	{
	field tag;
	field vtype;
	};


union Expression
	{
	field tag;
	struct Headblock headblock;
	struct Exprblock exprblock;
	struct Addrblock addrblock;
	struct Tempblock tempblock;
	struct Constblock constblock;
	struct Errorblock errorblock;
	struct Listblock listblock;
	struct Primblock primblock;
	} ;



struct Dimblock
	{
	int ndim;
	expptr nelt;
	expptr baseoffset;
	expptr basexpr;
	struct
		{
		expptr dimsize;
		expptr dimexpr;
		expptr lb;
		expptr lbaddr;
		expptr ub;
		expptr ubaddr;
		} dims[1];
	};


struct Impldoblock
	{
	field tag;
	unsigned isactive:1;
	unsigned isbusy:1;
	Namep varnp;
	Constp varvp;
	chainp impdospec;
	expptr implb;
	expptr impub;
	expptr impstep;
	ftnint impdiff;
	ftnint implim;
	struct Chain *datalist;
	};


struct Rplblock	/* name replacement block */
	{
	struct Rplblock *rplnextp;
	Namep rplnp;
	expptr rplvp;
	expptr rplxp;
	int rpltag;
	};



struct Equivblock
	{
	struct Eqvchain *equivs;
	flag eqvinit;
	long int eqvtop;
	long int eqvbottom;
	char inlcomm;
	char init;
	long initoffset;
	} ;
#define eqvleng eqvtop

extern struct Equivblock *eqvclass;


struct Eqvchain
	{
	struct Eqvchain *eqvnextp;
	union
		{
		struct Primblock *eqvlhs;
		Namep eqvname;
		} eqvitem;
	long int eqvoffset;
	} ;


union Taggedblock
	{
	field tag;
	struct Headblock headblock;
	struct Nameblock nameblock;
	struct Paramblock paramblock;
	struct Exprblock exprblock;
	struct Constblock constblock;
	struct Listblock listblock;
	struct Addrblock addrblock;
	struct Tempblock tempblock;
	struct Errorblock errorblock;
	struct Primblock primblock;
	struct Impldoblock impldoblock;
	} ;




struct Literal
	{
	short littype;
	short litnum;
	union	{
		ftnint litival;
		double litdval;
		struct	{
			char litclen;	/* small integer */
			char litcstr[XL];
			} litcval;
		} litval;
	};

extern struct Literal litpool[ ];
extern int nliterals;



/* popular functions with non integer return values */


int *ckalloc();
char *varstr(), *nounder(), *varunder();
char *copyn(), *copys();
chainp hookup(), mkchain();
ftnint convci();
char *convic();
char *setdoto();
double convcd();
Namep mkname();
struct Labelblock *mklabel(), *execlab();
struct Extsym *mkext(), *newentry();
expptr addrof(), call1(), call2(), call3(), call4();
Tempp mktmpn();
Addrp builtin(), mktemp(), altmpn(), mkaltemp(), mkaltmpn(), autovar();
Addrp mkplace(), mkaddr(), putconst(), memversion();
expptr mkprim(), mklhs(), mkexpr(), mkconv(), intrconv(), mkfunct();
expptr fixexpr(), fixtype();
expptr errnode(), mkintcon();
tagptr cpexpr();
ftnint lmin(), lmax(), iarrlen();
