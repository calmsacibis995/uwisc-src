# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 200
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin ={stdin}, *yyout ={stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
/*
 *******************************************************************************
 *
 *	Copyright (c) 1985 Regents of the University of California.
 *	All rights reserved.  The Berkeley software License Agreement
 *	specifies the terms and conditions for redistribution.
 *
 *	@(#)commands.l	5.4 (Berkeley) 8/21/86
 *
 *  commands.l
 *
 *  	Andrew Cherenson 	CS298-26  Fall 1985
 *
 *	Lex input file for the nslookup program command interpreter.
 *	When a sequence is recognized, the associated action
 *	routine is called. The action routine may need to
 *	parse the string for additional information.
 *
 *  Recognized commands: (identifiers are shown in uppercase)
 *
 *	server NAME	- set default server to NAME, using default server
 *	lserver NAME	- set default server to NAME, using initial server
 *	finger [NAME]	- finger the optional NAME
 *	root		- set default server to the root
 *	ls NAME		- list the domain NAME
 *	view FILE	- sorts and view the file with more
 *	set OPTION	- set an option
 *	help 		- print help information
 *	? 		- print help information
 *	opt[ions]	- print options, current server, host
 *	NAME		- print info about the host/domain NAME 
 *			  using default server.
 *	NAME1 NAME2	- as above, but use NAME2 as server
 *
 *
 *   yylex Results:
 *	0		upon end-of-file.
 *	1		after each command.
 *  
 *******************************************************************************
 */

#include "res.h"
extern char rootServerName[];

# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
	{ 
					    /* 
					     * 0 == use current server to find
					     *	    the new one.
					     * 1 == use original server to find
					     *	    the new one.
					     */
					    SetDefaultServer(yytext, 0); 
					    return(1);
					}
break;
case 2:
	{ 
					    SetDefaultServer(yytext, 1); 
					    return(1);
					}
break;
case 3:
			{ 
					    SetDefaultServer(rootServerName, 1);
					    return(1);
					}
break;
case 4:
	{
					    /* 
					     * 2nd arg. 
					     *  0 == output to stdout
					     *  1 == output to file
					     */
					    Finger(yytext, 1); 
					    return(1);
					}
break;
case 5:
	{ 
					    Finger(yytext, 0); 
					    return(1);
					}
break;
case 6:
	{ 
					    ViewList(yytext); 
					    return(1);
					}
break;
case 7:
	{ 
					    /* 
					     * 2nd arg. 
					     *  0 == output to stdout
					     *  1 == output to file
					     */
					    ListHosts(yytext, 1);
					    return(1);
					}
break;
case 8:
	{ 
					    ListHosts(yytext, 0);
					    return(1);
					}
break;
case 9:
 	{ 
					    SetOption(yytext); 
					    return(1);
					}
break;
case 10:
			{ 
					    extern void PrintHelp();

					    PrintHelp();
					    return(1);
					}
break;
case 11:
			{ 
					    PrintHelp();
					    return(1);
					}
break;
case 12:
			{ 
					    ShowOptions(TRUE); 
					    return(1);
					}
break;
case 13:
	{
					    /* 
					     * 0 == output to stdout
					     * 1 == output to file
					     */
					    LookupHost(yytext, 1); 
					    return(1);
					}
break;
case 14:
	{
					    LookupHost(yytext, 0); 
					    return(1);
					}
break;
case 15:
	{
					    /* 
					     * 0 == output to stdout
					     * 1 == output to file
					     */
					    LookupHostWithServer(yytext, 1); 
					    return(1);
					}
break;
case 16:
{
					    LookupHostWithServer(yytext, 0); 
					    return(1);
					}
break;
case 17:
			{ 
					    return(1);
					}
break;
case 18:
				{ 
					    printf("Unrecognized command: %s", 
					    		yytext); 
					    return(1);
					}
break;
case 19:
				{ ; }
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
int yyvstop[] ={
0,

19,
0,

17,
18,
19,
0,

-14,
0,

-11,
0,

-14,
0,

-14,
0,

-14,
0,

-14,
0,

-14,
0,

-14,
0,

-14,
0,

18,
0,

17,
18,
0,

-14,
0,

14,
18,
0,

11,
18,
0,

-14,
0,

-14,
0,

-14,
0,

-14,
0,

-14,
0,

-14,
0,

-14,
0,

-16,
0,

-14,
0,

-14,
0,

-14,
0,

-14,
0,

-12,
-14,
0,

-14,
0,

-14,
0,

-14,
0,

-14,
0,

-16,
0,

16,
18,
0,

-13,
0,

-14,
0,

-10,
-14,
0,

-8,
-16,
0,

-14,
0,

-12,
-14,
0,

12,
14,
18,
0,

-14,
0,

-3,
-14,
0,

-14,
0,

-14,
0,

-14,
0,

-13,
0,

13,
18,
0,

-14,
0,

-10,
-14,
0,

10,
14,
18,
0,

-8,
-16,
0,

8,
16,
18,
0,

-14,
0,

-14,
0,

-3,
-14,
0,

3,
14,
18,
0,

-14,
0,

-9,
-16,
0,

-9,
0,

-14,
0,

-15,
0,

-5,
-14,
0,

-14,
0,

-14,
0,

-14,
0,

-9,
-16,
0,

9,
16,
18,
0,

-9,
0,

9,
18,
0,

-6,
-16,
0,

-6,
0,

-15,
0,

15,
18,
0,

-5,
-14,
0,

5,
14,
18,
0,

-7,
-15,
0,

-8,
0,

-8,
0,

-14,
0,

-12,
-14,
0,

-14,
0,

-6,
-16,
0,

6,
16,
18,
0,

-6,
0,

6,
18,
0,

-5,
-16,
0,

-7,
-15,
0,

7,
15,
18,
0,

-8,
0,

8,
18,
0,

-8,
0,

-14,
0,

-1,
-16,
0,

-5,
-16,
0,

5,
16,
18,
0,

-4,
-13,
0,

-2,
-16,
0,

-1,
-16,
0,

1,
16,
18,
0,

-4,
-13,
0,

4,
13,
18,
0,

-7,
0,

-2,
-16,
0,

2,
16,
18,
0,

-4,
-15,
0,

-7,
0,

7,
18,
0,

-4,
-15,
0,

4,
15,
18,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] ={
0,0,	0,0,	0,0,	0,0,	
2,4,	0,0,	4,4,	0,0,	
0,0,	0,0,	0,0,	1,3,	
2,5,	2,6,	4,4,	4,16,	
7,18,	7,19,	5,17,	8,8,	
8,20,	9,4,	0,0,	11,4,	
0,0,	18,18,	18,19,	0,0,	
10,4,	9,18,	9,19,	11,18,	
11,19,	12,4,	0,0,	13,4,	
10,18,	10,19,	23,32,	56,56,	
56,57,	12,18,	12,19,	13,18,	
13,19,	2,7,	21,4,	4,4,	
2,4,	7,7,	4,4,	22,4,	
7,7,	29,41,	21,18,	21,19,	
34,48,	34,49,	18,28,	22,18,	
22,19,	63,77,	9,7,	64,78,	
11,7,	9,7,	2,8,	11,7,	
41,41,	10,7,	0,0,	5,8,	
10,7,	0,0,	12,7,	0,0,	
13,7,	12,7,	18,29,	13,7,	
84,84,	84,85,	0,0,	39,39,	
39,40,	0,0,	29,42,	21,7,	
24,4,	29,42,	21,7,	0,0,	
22,7,	88,88,	88,89,	22,7,	
24,18,	24,19,	101,101,	101,102,	
0,0,	41,42,	0,0,	14,26,	
41,42,	2,9,	29,41,	2,10,	
15,27,	27,38,	5,9,	2,11,	
5,10,	43,58,	2,12,	25,35,	
5,11,	2,13,	2,14,	5,12,	
0,0,	2,15,	5,13,	5,14,	
31,44,	9,21,	5,15,	35,51,	
10,22,	24,7,	23,33,	26,36,	
24,7,	26,37,	28,39,	28,40,	
39,55,	11,23,	30,4,	47,65,	
32,4,	37,53,	37,19,	50,66,	
12,24,	13,25,	30,18,	30,19,	
32,32,	32,19,	52,69,	33,4,	
34,50,	36,4,	58,75,	21,30,	
65,79,	46,63,	22,31,	33,18,	
33,19,	36,18,	36,19,	38,4,	
46,63,	48,48,	48,49,	28,28,	
66,80,	46,64,	28,28,	38,18,	
38,19,	69,81,	37,7,	42,56,	
42,57,	37,7,	79,96,	30,7,	
55,73,	32,45,	30,7,	80,97,	
32,46,	0,0,	32,4,	44,59,	
44,60,	45,61,	45,62,	77,77,	
33,7,	0,0,	36,7,	33,7,	
0,0,	36,7,	48,28,	78,78,	
32,4,	32,29,	0,0,	24,34,	
38,7,	51,67,	51,68,	38,7,	
42,42,	54,72,	54,19,	42,42,	
0,0,	55,74,	59,59,	59,60,	
55,74,	0,0,	48,29,	53,4,	
44,7,	72,72,	45,45,	44,7,	
77,94,	45,45,	73,73,	53,53,	
53,19,	61,61,	61,62,	0,0,	
78,95,	55,73,	32,4,	67,67,	
67,68,	0,0,	51,7,	0,0,	
30,43,	51,7,	54,7,	70,82,	
70,83,	54,7,	0,0,	59,28,	
105,105,	105,106,	71,84,	71,85,	
82,82,	82,83,	72,86,	0,0,	
76,92,	72,87,	0,0,	73,74,	
53,70,	0,0,	73,74,	53,71,	
33,47,	74,88,	74,89,	59,29,	
67,28,	0,0,	36,52,	75,90,	
75,91,	0,0,	72,29,	92,92,	
70,70,	81,98,	81,19,	70,70,	
53,29,	38,54,	61,76,	71,71,	
86,99,	86,100,	71,71,	0,0,	
67,29,	76,93,	87,101,	87,102,	
76,93,	0,0,	90,90,	90,91,	
93,105,	93,106,	74,74,	99,99,	
99,100,	74,74,	94,107,	94,108,	
75,7,	82,55,	0,0,	75,7,	
92,93,	76,92,	81,7,	92,93,	
0,0,	81,7,	0,0,	95,109,	
95,108,	86,86,	107,107,	107,108,	
86,86,	96,110,	96,19,	87,87,	
0,0,	104,114,	87,87,	90,103,	
0,0,	93,93,	97,48,	97,49,	
93,93,	109,109,	109,108,	94,94,	
98,98,	98,19,	94,94,	103,112,	
103,113,	110,110,	110,19,	111,118,	
111,119,	112,112,	112,113,	90,104,	
95,95,	114,114,	116,123,	95,95,	
99,55,	0,0,	96,7,	115,121,	
115,122,	96,7,	104,115,	117,125,	
117,126,	104,115,	0,0,	97,7,	
118,118,	118,119,	97,7,	0,0,	
120,127,	98,111,	0,0,	107,116,	
103,103,	123,123,	110,117,	103,103,	
111,111,	127,127,	104,114,	111,111,	
121,121,	121,122,	114,115,	116,124,	
0,0,	114,115,	116,124,	0,0,	
115,115,	98,29,	0,0,	115,115,	
117,117,	0,0,	110,29,	117,117,	
124,129,	124,130,	112,120,	125,125,	
125,126,	120,128,	0,0,	116,123,	
120,128,	0,0,	123,124,	128,131,	
128,132,	123,124,	127,128,	129,129,	
129,130,	127,128,	131,131,	131,132,	
0,0,	118,55,	0,0,	0,0,	
0,0,	120,127,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	124,124,	0,0,	0,0,	
124,124,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
128,128,	0,0,	0,0,	128,128,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
125,55,	0,0,	0,0,	0,0,	
0,0};
struct yysvf yysvec[] ={
0,	0,	0,
yycrank+1,	0,		0,	
yycrank+-3,	0,		0,	
yycrank+0,	0,		yyvstop+1,
yycrank+-5,	0,		0,	
yycrank+-8,	yysvec+2,	0,	
yycrank+0,	0,		yyvstop+3,
yycrank+-7,	yysvec+4,	yyvstop+7,
yycrank+-10,	yysvec+4,	yyvstop+9,
yycrank+-20,	0,		yyvstop+11,
yycrank+-27,	0,		yyvstop+13,
yycrank+-22,	0,		yyvstop+15,
yycrank+-32,	0,		yyvstop+17,
yycrank+-34,	0,		yyvstop+19,
yycrank+-2,	yysvec+10,	yyvstop+21,
yycrank+-3,	yysvec+9,	yyvstop+23,
yycrank+0,	0,		yyvstop+25,
yycrank+0,	0,		yyvstop+27,
yycrank+-16,	yysvec+4,	yyvstop+30,
yycrank+0,	0,		yyvstop+32,
yycrank+0,	0,		yyvstop+35,
yycrank+-45,	0,		yyvstop+38,
yycrank+-50,	0,		yyvstop+40,
yycrank+-29,	yysvec+10,	yyvstop+42,
yycrank+-87,	0,		yyvstop+44,
yycrank+-4,	yysvec+13,	yyvstop+46,
yycrank+-17,	yysvec+24,	yyvstop+48,
yycrank+-8,	yysvec+10,	yyvstop+50,
yycrank+-125,	yysvec+4,	yyvstop+52,
yycrank+-44,	yysvec+4,	0,	
yycrank+-137,	0,		yyvstop+54,
yycrank+-12,	yysvec+12,	yyvstop+56,
yycrank+-139,	0,		yyvstop+58,
yycrank+-150,	0,		yyvstop+60,
yycrank+-47,	yysvec+9,	yyvstop+62,
yycrank+-11,	yysvec+24,	yyvstop+65,
yycrank+-152,	0,		yyvstop+67,
yycrank+-132,	yysvec+4,	yyvstop+69,
yycrank+-162,	0,		yyvstop+71,
yycrank+-74,	yysvec+4,	yyvstop+73,
yycrank+0,	0,		yyvstop+75,
yycrank+-59,	yysvec+4,	0,	
yycrank+-166,	yysvec+4,	yyvstop+78,
yycrank+-12,	yysvec+10,	yyvstop+80,
yycrank+-178,	yysvec+4,	yyvstop+82,
yycrank+-180,	yysvec+4,	yyvstop+85,
yycrank+-60,	yysvec+4,	0,	
yycrank+-21,	yysvec+36,	yyvstop+88,
yycrank+-156,	yysvec+4,	yyvstop+90,
yycrank+0,	0,		yyvstop+93,
yycrank+-32,	yysvec+13,	yyvstop+97,
yycrank+-196,	yysvec+4,	yyvstop+99,
yycrank+-49,	yysvec+10,	yyvstop+102,
yycrank+-218,	0,		yyvstop+104,
yycrank+-200,	yysvec+4,	yyvstop+106,
yycrank+-171,	yysvec+4,	0,	
yycrank+-30,	yysvec+4,	yyvstop+108,
yycrank+0,	0,		yyvstop+110,
yycrank+-40,	yysvec+33,	yyvstop+113,
yycrank+-205,	yysvec+4,	yyvstop+115,
yycrank+0,	0,		yyvstop+118,
yycrank+-220,	yysvec+4,	yyvstop+122,
yycrank+0,	0,		yyvstop+125,
yycrank+-52,	yysvec+4,	0,	
yycrank+-54,	yysvec+4,	0,	
yycrank+-55,	yysvec+10,	yyvstop+129,
yycrank+-58,	yysvec+21,	yyvstop+131,
yycrank+-226,	yysvec+4,	yyvstop+133,
yycrank+0,	0,		yyvstop+136,
yycrank+-59,	yysvec+33,	yyvstop+140,
yycrank+-234,	yysvec+4,	yyvstop+142,
yycrank+-241,	yysvec+4,	yyvstop+145,
yycrank+-212,	yysvec+53,	yyvstop+147,
yycrank+-217,	yysvec+4,	0,	
yycrank+-256,	yysvec+4,	yyvstop+149,
yycrank+-262,	yysvec+4,	yyvstop+151,
yycrank+-247,	yysvec+4,	0,	
yycrank+-182,	yysvec+4,	0,	
yycrank+-190,	yysvec+4,	0,	
yycrank+-64,	yysvec+33,	yyvstop+154,
yycrank+-68,	yysvec+11,	yyvstop+156,
yycrank+-268,	yysvec+4,	yyvstop+158,
yycrank+-243,	yysvec+4,	yyvstop+160,
yycrank+0,	0,		yyvstop+163,
yycrank+-71,	yysvec+4,	yyvstop+167,
yycrank+0,	0,		yyvstop+169,
yycrank+-275,	yysvec+4,	yyvstop+172,
yycrank+-281,	yysvec+4,	yyvstop+175,
yycrank+-84,	yysvec+4,	yyvstop+177,
yycrank+0,	0,		yyvstop+179,
yycrank+-285,	yysvec+4,	yyvstop+182,
yycrank+0,	0,		yyvstop+185,
yycrank+-266,	yysvec+4,	0,	
yycrank+-287,	yysvec+4,	yyvstop+189,
yycrank+-293,	yysvec+4,	yyvstop+192,
yycrank+-306,	yysvec+4,	yyvstop+194,
yycrank+-312,	yysvec+4,	yyvstop+196,
yycrank+-321,	yysvec+4,	yyvstop+198,
yycrank+-327,	yysvec+4,	yyvstop+201,
yycrank+-290,	yysvec+4,	yyvstop+203,
yycrank+0,	0,		yyvstop+206,
yycrank+-89,	yysvec+4,	yyvstop+210,
yycrank+0,	0,		yyvstop+212,
yycrank+-330,	yysvec+4,	yyvstop+215,
yycrank+-316,	yysvec+4,	0,	
yycrank+-239,	yysvec+4,	yyvstop+218,
yycrank+0,	0,		yyvstop+221,
yycrank+-309,	yysvec+4,	yyvstop+225,
yycrank+0,	0,		yyvstop+227,
yycrank+-324,	yysvec+4,	yyvstop+230,
yycrank+-332,	yysvec+4,	yyvstop+232,
yycrank+-334,	yysvec+4,	yyvstop+234,
yycrank+-336,	yysvec+4,	yyvstop+237,
yycrank+0,	0,		yyvstop+240,
yycrank+-340,	yysvec+4,	0,	
yycrank+-346,	yysvec+4,	yyvstop+244,
yycrank+-341,	yysvec+4,	0,	
yycrank+-350,	yysvec+4,	yyvstop+247,
yycrank+-355,	yysvec+4,	yyvstop+250,
yycrank+0,	0,		yyvstop+253,
yycrank+-359,	yysvec+4,	0,	
yycrank+-371,	yysvec+4,	yyvstop+257,
yycrank+0,	0,		yyvstop+260,
yycrank+-364,	yysvec+4,	0,	
yycrank+-387,	yysvec+4,	yyvstop+264,
yycrank+-390,	yysvec+4,	yyvstop+266,
yycrank+0,	0,		yyvstop+269,
yycrank+-368,	yysvec+4,	0,	
yycrank+-398,	yysvec+4,	yyvstop+273,
yycrank+-402,	yysvec+4,	yyvstop+276,
yycrank+0,	0,		yyvstop+278,
yycrank+-405,	yysvec+4,	yyvstop+281,
yycrank+0,	0,		yyvstop+284,
0,	0,	0};
struct yywork *yytop = yycrank+452;
struct yysvf *yybgin = yysvec+1;
char yymatch[] ={
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,'*' ,01  ,01  ,'-' ,'*' ,'-' ,
'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,
'*' ,'*' ,01  ,01  ,01  ,'-' ,01  ,01  ,
01  ,'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,
'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,
'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,
'*' ,'*' ,'*' ,01  ,01  ,01  ,01  ,'-' ,
01  ,'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,
'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,
'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,'*' ,
'*' ,'*' ,'*' ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] ={
0,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,
1,0,0,0,0,0,0,0,
0};
/*	ncform	4.1	83/08/11	*/

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank){		/* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
