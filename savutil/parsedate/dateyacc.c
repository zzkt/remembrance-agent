
# line 38 "dateyacc.y"
#ifdef RCSIDENT
static char rcsident[] = "$Header: /local/CVSROOT/RA/savutil/parsedate/dateyacc.c,v 1.1.1.1 2001/04/24 15:40:54 finagler Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#define	yymaxdepth date_maxdepth
#define	yyparse	date_parse
#define	yylex	date_lex
#define	yyerror	date_error
#define	yylval	date_lval
#define	yychar	date_char
#define	yydebug	date_debug
#define	yypact	date_pact
#define	yyr1	date_r1
#define	yyr2	date_r2
#define	yydef	date_def
#define	yychk	date_chk
#define	yypgo	date_pgo
#define	yyact	date_act
#define	yyexca	date_exca
#define yyerrflag date_errflag
#define yynerrs	date_nerrs
#define	yyps	date_ps
#define	yypv	date_pv
#define	yys	date_s
#define	yy_yys	date_yys
#define	yystate	date_state
#define	yytmp	date_tmp
#define	yyv	date_v
#define	yy_yyv	date_yyv
#define	yyval	date_val
#define	yylloc	date_lloc
#define yyreds	date_reds
#define yytoks	date_toks
#define yylhs	date_yylhs
#define yylen	date_yylen
#define yydefred date_yydefred
#define yydgoto	date_yydgoto
#define yysindex date_yysindex
#define yyrindex date_yyrindex
#define yygindex date_yygindex
#define yytable	 date_yytable
#define yycheck	 date_yycheck

#define yyans date_ans
#define yyinbuf date_inbuf

#include "parsedate.h"
struct parsedate yyans;

/* No error routine is needed here. */
#define date_error(s)

# line 97 "dateyacc.y"
typedef union
#ifdef __cplusplus
	YYSTYPE
#endif
 {
    int IntVal;
} YYSTYPE;
# define DAY_NAME 257
# define MONTH_NAME 258
# define NUM9 259
# define NUM23 260
# define NUM59 261
# define NUM99 262
# define NUM2359 263
# define NUM9999 264
# define NUM235959 265
# define NUM991231 266
# define NUM99991231 267
# define AMPM 268
# define STD_ZONE 269
# define DST_ZONE 270
# define DST_SUFFIX 271

// malloc.h also doesn't seem to be needed, and Mac OS-X has trouble finding it.
// removed it. -- BJR 9/21/03
// #include <malloc.h>

#include <memory.h>
#include <unistd.h>

// I'm not sure why values.h was ever included. It seems that commenting it out 
// works just fine though.  -- BJR 1/18/01
// #include <values.h>

#ifdef __cplusplus

#ifndef yyerror
	void yyerror(const char *);
#endif
#ifndef yylex
	extern "C" int yylex(void);
#endif
	int yyparse(void);

#endif
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
YYSTYPE yylval;
YYSTYPE yyval;
typedef int yytabelem;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#if YYMAXDEPTH > 0
int yy_yys[YYMAXDEPTH], *yys = yy_yys;
YYSTYPE yy_yyv[YYMAXDEPTH], *yyv = yy_yyv;
#else	/* user does initial allocation */
int *yys;
YYSTYPE *yyv;
#endif
static int yymaxdepth = YYMAXDEPTH;
# define YYERRCODE 256

# line 410 "dateyacc.y"

yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 8,
	0, 55,
	268, 89,
	269, 89,
	270, 89,
	45, 89,
	43, 89,
	-2, 81,
-1, 16,
	268, 90,
	269, 90,
	270, 90,
	45, 90,
	43, 90,
	-2, 85,
-1, 31,
	268, 89,
	269, 89,
	270, 89,
	45, 89,
	43, 89,
	-2, 81,
-1, 118,
	0, 132,
	-2, 89,
	};
# define YYNPROD 133
# define YYLAST 436
yytabelem yyact[]={

    10,    15,    24,    22,    25,    27,    18,     8,    19,    26,
    20,    21,   131,   185,   133,    15,    24,    49,    50,    27,
    18,    48,    19,   184,    20,    21,    24,    22,    25,    27,
    18,    31,    19,    26,    20,    21,    24,    49,    50,    27,
    18,    48,    19,    56,    20,    21,    78,   179,    76,    78,
   172,    76,   173,   174,   170,   171,   164,   165,   157,   158,
   132,   155,   128,    50,    27,    15,    56,    32,    33,    15,
    18,    31,    19,    26,    24,    22,    25,    27,    56,    36,
    23,    26,    32,    33,    56,    18,    31,    19,    26,    15,
   168,    32,    33,    73,    64,    36,    65,    26,    32,    33,
   166,    67,    36,    68,    26,    32,    33,    15,   163,   118,
    14,    26,    18,    48,    19,    18,    48,    19,   134,   135,
   162,    68,   136,    15,    24,    49,    50,    27,    24,    49,
    50,    27,    49,    50,    27,   161,    72,    65,    70,   137,
   114,   115,     4,    60,    83,    28,    34,   160,    45,    51,
    53,   169,   167,    55,     2,    59,    82,    81,    37,    43,
   138,   139,    71,   140,   141,    80,   127,   111,    61,    58,
    12,    69,    86,    17,    12,    12,    16,    79,    89,     6,
    13,    62,    42,     9,    40,    46,    98,    99,   116,   103,
   104,     1,   156,    92,   159,   106,     0,   110,     0,   100,
     3,     0,     0,     0,     0,    38,    44,   107,   109,    12,
     0,     0,     0,     0,     0,    12,     0,     0,     0,    94,
     0,     0,     0,     0,     0,   102,   113,    62,   134,   135,
   145,     0,   136,     0,   119,   121,     0,     0,   129,   130,
    93,   175,   176,   177,   178,     0,   101,   180,   181,   182,
   183,   147,   152,    15,    24,    22,    25,    27,   149,    36,
   154,    26,   151,     0,   153,     0,     0,     0,     0,     0,
     0,    74,    75,    77,     0,    75,    77,    24,    22,    25,
    27,     0,    36,     0,    26,     0,     0,     0,    15,    24,
    49,    50,    27,     0,     0,    24,    49,    50,    27,     0,
     0,     0,     0,     0,     0,     0,     0,    24,    49,    50,
    27,     0,     0,     0,     0,    49,    50,    27,     5,     0,
     0,    29,    35,    39,     0,    52,    54,    57,    11,     0,
     0,     0,    11,    11,     0,     0,    30,     7,     0,    63,
    66,    41,    47,     0,     0,     0,     0,    84,     0,     0,
     0,     0,     0,    88,     0,     0,    90,    91,     0,    96,
    97,     0,     0,     0,     0,    85,    87,    11,     0,     0,
   105,     0,   108,    11,     0,     0,    95,     0,     0,     0,
   117,     0,    95,     0,   112,    63,    66,     0,     0,     0,
     0,     0,   120,   122,     0,   123,   124,     0,     0,     0,
     0,     0,     0,     0,   143,     0,   125,   126,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   142,     0,   144,     0,     0,   150,   146,     0,     0,
     0,     0,     0,     0,     0,   148 };
yytabelem yypact[]={

  -256,-10000000,  -192,  -168,  -232,  -242,  -168,    -4,    97,   122,
-10000000,    49,    56,   126,    91,   116,-10000000,     3,-10000000,-10000000,
-10000000,-10000000,   119,-10000000,   111,    98,-10000000,-10000000,  -177,  -150,
-10000000,    97,   119,    98,  -161,  -188,    97,  -177,  -161,  -222,
  -161,    19,-10000000,  -150,  -188,  -222,  -188,    31,-10000000,-10000000,
-10000000,  -161,  -188,  -184,  -134,-10000000,  -130,  -130,  -127,  -197,
  -197,  -154,-10000000,-10000000,  -130,  -130,-10000000,  -127,  -127,  -147,
  -147,-10000000,     6,-10000000,-10000000,  -209,   -31,-10000000,  -141,  -197,
  -197,-10000000,  -197,  -197,  -147,  -161,  -147,  -188,-10000000,-10000000,
  -147,-10000000,  -147,-10000000,-10000000,    37,-10000000,  -130,  -147,-10000000,
  -147,-10000000,-10000000,-10000000,  -130,-10000000,-10000000,  -161,  -130,  -188,
  -130,-10000000,    90,    74,-10000000,-10000000,-10000000,  -188,    97,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -210,
-10000000,  -141,  -211,  -141,    89,    62,-10000000,  -213,    42,   106,
    32,   105,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -215,  -221,-10000000,  -217,
  -197,  -197,  -197,  -197,  -224,-10000000,  -197,  -197,  -197,  -197,
  -248,-10000000,-10000000,  -258,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000 };
yytabelem yypgo[]={

     0,    80,   327,    60,   191,   153,   142,   318,   336,   200,
   179,   182,   188,   180,   110,   169,   176,   173,   136,    93 };
yytabelem yyr1[]={

     0,     1,     1,     2,     2,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,     6,     6,     9,     9,     9,    10,    10,    10,    11,
    11,     5,     5,     5,    13,    13,    13,    14,    14,    14,
     8,     8,     8,    15,    15,     7,     7,     7,     7,    17,
    17,    16,    16,    16,    16,    16,    16,    16,    16,    16,
    16,    16,    18,    19,    19,    19,    19,    19,    19,    19,
    19,    19,    19,    19,    19,    19,    19,    19,    19,    19,
    19,    19,    19,    19,    19,     3,     3,     3,     3,     3,
     3,     3,    12 };
yytabelem yyr2[]={

     0,     2,     2,     2,     2,     2,     4,     6,     8,     6,
     8,     4,     6,     8,     6,     8,     2,     4,     6,     4,
     6,     4,     6,     8,     4,     6,     6,     6,     6,     4,
     6,     8,     2,     4,     6,     8,     4,     6,     6,     6,
     6,     4,     6,     8,     2,     4,     6,     4,     6,     6,
     8,     8,     6,     8,     8,     3,     2,     6,     6,     8,
     3,     3,     5,     4,     6,     6,     4,     6,     2,     3,
     3,     5,     5,     5,     7,     7,     7,     7,     7,     7,
     3,     3,     3,     3,     5,     2,     4,     4,     6,     3,
     2,     7,     7,    11,    11,     7,     7,    11,    11,     7,
     7,     3,     3,     3,     5,     5,     7,     3,     5,     5,
     7,     5,     7,     7,     9,     7,     9,    11,     9,     7,
     9,     7,     9,    11,     9,     3,     7,     7,     3,     7,
     7,     3,     3 };
yytabelem yychk[]={

-10000000,    -4,    -5,    -9,    -6,    -7,   -10,    -8,   263,   -11,
   256,    -2,   -15,   -13,   -14,   257,   -16,   -17,   262,   264,
   266,   267,   259,    -1,   258,   260,   265,   261,    -6,    -7,
    -8,   263,   259,   260,    -6,    -7,   263,    -5,    -9,    -7,
   -10,    -8,   -11,    -5,    -9,    -6,   -10,    -8,   263,   259,
   260,    -6,    -7,    -6,    -7,    -5,    47,    -2,   -15,    58,
    46,    46,   -15,    -2,    45,    47,    -2,    45,    47,    45,
    47,    46,   -18,   -19,   268,   269,    45,   270,    43,    58,
    46,    46,    58,    46,    -7,    -8,    -6,    -8,    -7,    -6,
    -7,    -7,    -5,    -9,   -10,    -8,    -7,    -7,    -6,    -6,
    -5,    -9,   -10,    -6,    -6,    -7,    -6,    -5,    -7,    -5,
    -6,   -14,    -2,   -15,    -1,    -1,   -12,    -7,   263,   -15,
    -2,   -15,    -2,    -2,    -2,    -8,    -8,   -19,   271,   269,
   270,    43,    -3,    45,   259,   260,   263,    -3,    -1,    -1,
    -1,    -1,    -8,    -7,    -8,    -6,    -8,    -5,    -8,    -5,
    -7,    -5,    -6,    -5,    -6,   271,    -3,   269,   270,    -3,
    58,    46,    58,    46,   269,   270,    58,    46,    58,    46,
   269,   270,   271,   269,   270,    -1,    -1,    -1,    -1,   271,
    -1,    -1,    -1,    -1,   271,   271 };
yytabelem yydef[]={

     0,    -2,     5,    16,    56,    32,    44,     0,    -2,    68,
    60,     0,     0,     0,     0,    61,    -2,     0,    80,    82,
    69,    70,     3,     4,    83,     1,   101,     2,     6,    11,
    63,    -2,     0,     0,    17,    19,    89,    21,    24,     0,
    29,     0,    68,    33,    36,     0,    41,     0,    81,     3,
     1,    45,    47,     0,     0,    66,     0,     0,     0,     0,
     0,     0,    71,    73,     0,     0,    72,     0,     0,     0,
     0,    62,    86,    87,   102,   103,     0,   107,     0,     0,
     0,    84,     0,     0,     7,     9,    12,    14,    18,    20,
    22,    25,    26,    27,    28,     0,    30,     0,    34,    37,
    38,    39,    40,    42,     0,    46,    48,    49,     0,    52,
     0,    67,     0,     0,    99,   100,    57,    58,    -2,    74,
    76,    77,    79,    75,    78,    64,    65,    88,   104,   105,
   108,     0,   111,     0,   125,   128,   131,   109,    91,    92,
    95,    96,     8,    10,    13,    15,    23,    31,    35,    43,
    50,    51,    53,    54,    59,   106,   110,   119,   121,   112,
     0,     0,     0,     0,   113,   115,     0,     0,     0,     0,
   116,   118,   120,   122,   124,   126,   127,   129,   130,   114,
    93,    94,    97,    98,   117,   123 };
typedef struct
#ifdef __cplusplus
	yytoktype
#endif
{ char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"DAY_NAME",	257,
	"MONTH_NAME",	258,
	"NUM9",	259,
	"NUM23",	260,
	"NUM59",	261,
	"NUM99",	262,
	"NUM2359",	263,
	"NUM9999",	264,
	"NUM235959",	265,
	"NUM991231",	266,
	"NUM99991231",	267,
	"AMPM",	268,
	"STD_ZONE",	269,
	"DST_ZONE",	270,
	"DST_SUFFIX",	271,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"num59 : NUM23",
	"num59 : NUM59",
	"num : NUM9",
	"num : num59",
	"goal : date",
	"goal : date dayname",
	"goal : date dayname time",
	"goal : date dayname time year",
	"goal : date dayname year",
	"goal : date dayname year time",
	"goal : date time",
	"goal : date time dayname",
	"goal : date time dayname year",
	"goal : date time year",
	"goal : date time year dayname",
	"goal : date.year",
	"goal : date.year dayname",
	"goal : date.year dayname time",
	"goal : date.year time",
	"goal : date.year time dayname",
	"goal : dayname date",
	"goal : dayname date time",
	"goal : dayname date time year",
	"goal : dayname date.year",
	"goal : dayname date.year time",
	"goal : dayname time date",
	"goal : dayname time date.year",
	"goal : dayname time year.date",
	"goal : dayname year.date",
	"goal : dayname year.date time",
	"goal : dayname year time date",
	"goal : time",
	"goal : time date",
	"goal : time date dayname",
	"goal : time date dayname year",
	"goal : time date.year",
	"goal : time date.year dayname",
	"goal : time dayname date",
	"goal : time dayname date.year",
	"goal : time dayname year.date",
	"goal : time year.date",
	"goal : time year.date dayname",
	"goal : time year dayname date",
	"goal : year.date",
	"goal : year.date dayname",
	"goal : year.date dayname time",
	"goal : year.date time",
	"goal : year.date time dayname",
	"goal : year dayname date",
	"goal : year dayname date time",
	"goal : year dayname time date",
	"goal : year time date",
	"goal : year time date dayname",
	"goal : year time dayname date",
	"goal : NUM2359",
	"goal : dayname",
	"goal : yymmdd '.' time2359",
	"goal : yymmdd '.' time",
	"goal : yymmdd '.' time dayname",
	"goal : error",
	"dayname : DAY_NAME",
	"dayname : DAY_NAME '.'",
	"date.year : date year",
	"date.year : hyphen.date '-' year",
	"date.year : slash.date '/' year",
	"year.date : year date",
	"year.date : year '/' slash.date",
	"year.date : yymmdd",
	"yymmdd : NUM991231",
	"yymmdd : NUM99991231",
	"date : num month.name",
	"date : month.name num",
	"date : num num",
	"hyphen.date : num '-' month.name",
	"hyphen.date : month.name '-' num",
	"hyphen.date : num '-' num",
	"slash.date : num '/' month.name",
	"slash.date : month.name '/' num",
	"slash.date : num '/' num",
	"year : NUM99",
	"year : NUM2359",
	"year : NUM9999",
	"month.name : MONTH_NAME",
	"month.name : MONTH_NAME '.'",
	"time : hour.alone",
	"time : hour am.pm",
	"time : hour zone",
	"time : hour am.pm zone",
	"hour : NUM2359",
	"hour : hour.alone",
	"hour.alone : NUM9 ':' num59",
	"hour.alone : NUM9 '.' num59",
	"hour.alone : NUM9 ':' num59 ':' num59",
	"hour.alone : NUM9 '.' num59 '.' num59",
	"hour.alone : NUM23 ':' num59",
	"hour.alone : NUM23 '.' num59",
	"hour.alone : NUM23 ':' num59 ':' num59",
	"hour.alone : NUM23 '.' num59 '.' num59",
	"hour.alone : NUM2359 ':' num59",
	"hour.alone : NUM2359 '.' num59",
	"hour.alone : NUM235959",
	"am.pm : AMPM",
	"zone : STD_ZONE",
	"zone : STD_ZONE DST_SUFFIX",
	"zone : '-' STD_ZONE",
	"zone : '-' STD_ZONE DST_SUFFIX",
	"zone : DST_ZONE",
	"zone : '-' DST_ZONE",
	"zone : '+' zone.offset",
	"zone : '-' '+' zone.offset",
	"zone : '-' zone.offset",
	"zone : '-' '-' zone.offset",
	"zone : '+' zone.offset STD_ZONE",
	"zone : '+' zone.offset STD_ZONE DST_SUFFIX",
	"zone : '+' zone.offset DST_ZONE",
	"zone : '-' '+' zone.offset STD_ZONE",
	"zone : '-' '+' zone.offset STD_ZONE DST_SUFFIX",
	"zone : '-' '+' zone.offset DST_ZONE",
	"zone : '-' zone.offset STD_ZONE",
	"zone : '-' zone.offset STD_ZONE DST_SUFFIX",
	"zone : '-' zone.offset DST_ZONE",
	"zone : '-' '-' zone.offset STD_ZONE",
	"zone : '-' '-' zone.offset STD_ZONE DST_SUFFIX",
	"zone : '-' '-' zone.offset DST_ZONE",
	"zone.offset : NUM9",
	"zone.offset : NUM9 ':' num59",
	"zone.offset : NUM9 '.' num59",
	"zone.offset : NUM23",
	"zone.offset : NUM23 ':' num59",
	"zone.offset : NUM23 '.' num59",
	"zone.offset : NUM2359",
	"time2359 : NUM2359",
};
#endif /* YYDEBUG */
/* 
 *	Copyright 1987 Silicon Graphics, Inc. - All Rights Reserved
 */

/* #ident	"@(#)yacc:yaccpar	1.10" */
#ident	"$Revision: 1.1.1.1 $"

/*
** Skeleton parser driver for yacc output
*/
#include "stddef.h"

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#ifdef __cplusplus
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( gettxt("uxlibc:78", "syntax error - cannot backup") );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#else
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( gettxt("uxlibc:78", "Syntax error - cannot backup") );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#endif
#define YYRECOVERING()	(!!yyerrflag)
#define YYNEW(type)	malloc(sizeof(type) * yynewmax)
#define YYCOPY(to, from, type) \
	(type *) memcpy(to, (char *) from, yynewmax * sizeof(type))
#define YYENLARGE( from, type) \
	(type *) realloc((char *) from, yynewmax * sizeof(type))
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-10000000)

/*
** global variables used by the parser
*/
YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
#if defined(__STDC__) || defined(__cplusplus)
int yyparse(void)
#else
int yyparse()
#endif
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

#if YYMAXDEPTH <= 0
	if (yymaxdepth <= 0)
	{
		if ((yymaxdepth = YYEXPAND(0)) <= 0)
		{
#ifdef __cplusplus
			yyerror(gettxt("uxlibc:79", "yacc initialization error"));
#else
			yyerror(gettxt("uxlibc:79", "Yacc initialization error"));
#endif
			YYABORT;
		}
	}
#endif

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			int yynewmax;
			ptrdiff_t yys_off;

			/* The following pointer-differences are safe, since
			 * yypvt, yy_pv, and yypv all are a multiple of
			 * sizeof(YYSTYPE) bytes from yyv.
			 */
			ptrdiff_t yypvt_off = yypvt - yyv;
			ptrdiff_t yy_pv_off = yy_pv - yyv;
			ptrdiff_t yypv_off = yypv - yyv;

			int *yys_base = yys;
#ifdef YYEXPAND
			yynewmax = YYEXPAND(yymaxdepth);
#else
			yynewmax = 2 * yymaxdepth;	/* double table size */
			if (yymaxdepth == YYMAXDEPTH)	/* first time growth */
			{
				void *newyys = YYNEW(int);
				void *newyyv = YYNEW(YYSTYPE);
				if (newyys != 0 && newyyv != 0)
				{
					yys = YYCOPY(newyys, yys, int);
					yyv = YYCOPY(newyyv, yyv, YYSTYPE);
				}
				else
					yynewmax = 0;	/* failed */
			}
			else				/* not first time */
			{
				yys = YYENLARGE(yys, int);
				yyv = YYENLARGE(yyv, YYSTYPE);
				if (yys == 0 || yyv == 0)
					yynewmax = 0;	/* failed */
			}
#endif
			if (yynewmax <= yymaxdepth)	/* tables not expanded */
			{
#ifdef __cplusplus
				yyerror( gettxt("uxlibc:80", "yacc stack overflow") );
#else
				yyerror( gettxt("uxlibc:80", "Yacc stack overflow") );
#endif
				YYABORT;
			}
			yymaxdepth = yynewmax;

			/* reset pointers into yys */
			yys_off = yys - yys_base;
			yy_ps = yy_ps + yys_off;
			yyps = yyps + yys_off;

			/* reset pointers into yyv */
			yypvt = yyv + yypvt_off;
			yy_pv = yyv + yy_pv_off;
			yypv = yyv + yypv_off;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
#ifdef __cplusplus
				yyerror( gettxt("uxlibc:81", "syntax error") );
#else
				yyerror( gettxt("uxlibc:81", "Syntax error") );
#endif
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
				/* FALLTHRU */
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 55:
# line 179 "dateyacc.y"
{ yyans.hour   = yypvt[-0].IntVal / 100;
	  yyans.minute = yypvt[-0].IntVal % 100;
	  yyans.second = -1;		/* unspecified */
	} break;
case 60:
# line 188 "dateyacc.y"
{ extern char *yyinbuf;
	  if (yyans.error == NULL) yyans.error = yyinbuf;
	} break;
case 61:
# line 194 "dateyacc.y"
{ yyans.c_weekday = yypvt[-0].IntVal; } break;
case 62:
# line 196 "dateyacc.y"
{ yyans.c_weekday = yypvt[-1].IntVal; } break;
case 69:
# line 211 "dateyacc.y"
{ yyans.year  = (yypvt[-0].IntVal / 10000) + 1900;	/* ajs */
	  yyans.month = (yypvt[-0].IntVal % 10000) / 100;	/* ajs */
	  yyans.day   = (yypvt[-0].IntVal % 100);		/* ajs */
	} break;
case 70:
# line 217 "dateyacc.y"
{ yyans.year  = (yypvt[-0].IntVal / 10000);		/* ajs */
	  yyans.month = (yypvt[-0].IntVal % 10000) / 100;	/* ajs */
	  yyans.day   = (yypvt[-0].IntVal % 100);		/* ajs */
	} break;
case 71:
# line 224 "dateyacc.y"
{ yyans.day = yypvt[-1].IntVal; } break;
case 72:
# line 226 "dateyacc.y"
{ yyans.day = yypvt[-0].IntVal; } break;
case 73:
# line 228 "dateyacc.y"
{ yyans.month = yypvt[-1].IntVal; yyans.day = yypvt[-0].IntVal; } break;
case 74:
# line 232 "dateyacc.y"
{ yyans.day = yypvt[-2].IntVal; } break;
case 75:
# line 234 "dateyacc.y"
{ yyans.day = yypvt[-0].IntVal; } break;
case 76:
# line 236 "dateyacc.y"
{ yyans.month = yypvt[-2].IntVal; yyans.day = yypvt[-0].IntVal; } break;
case 77:
# line 240 "dateyacc.y"
{ yyans.day = yypvt[-2].IntVal; } break;
case 78:
# line 242 "dateyacc.y"
{ yyans.day = yypvt[-0].IntVal; } break;
case 79:
# line 244 "dateyacc.y"
{ yyans.month = yypvt[-2].IntVal; yyans.day = yypvt[-0].IntVal; } break;
case 80:
# line 248 "dateyacc.y"
{ yyans.year = 1900 + yypvt[-0].IntVal; } break;
case 81:
# line 250 "dateyacc.y"
{ yyans.year = yypvt[-0].IntVal; } break;
case 82:
# line 252 "dateyacc.y"
{ yyans.year = yypvt[-0].IntVal; } break;
case 83:
# line 256 "dateyacc.y"
{ yyans.month = yypvt[-0].IntVal; } break;
case 84:
# line 258 "dateyacc.y"
{ yyans.month = yypvt[-1].IntVal; } break;
case 89:
# line 268 "dateyacc.y"
{ yyans.hour   = yypvt[-0].IntVal / 100;
	  yyans.minute = yypvt[-0].IntVal % 100;
	  yyans.second = -1;		/* unspecified */
	} break;
case 91:
# line 276 "dateyacc.y"
{ yyans.hour   = yypvt[-2].IntVal;
	  yyans.minute = yypvt[-0].IntVal;
	  yyans.second = -1;		/* unspecified */
	} break;
case 92:
# line 281 "dateyacc.y"
{ yyans.hour   = yypvt[-2].IntVal;
	  yyans.minute = yypvt[-0].IntVal;
	  yyans.second = -1;		/* unspecified */
	} break;
case 93:
# line 286 "dateyacc.y"
{ yyans.hour   = yypvt[-4].IntVal;
	  yyans.minute = yypvt[-2].IntVal;
	  yyans.second = yypvt[-0].IntVal;
	} break;
case 94:
# line 291 "dateyacc.y"
{ yyans.hour   = yypvt[-4].IntVal;
	  yyans.minute = yypvt[-2].IntVal;
	  yyans.second = yypvt[-0].IntVal;
	} break;
case 95:
# line 296 "dateyacc.y"
{ yyans.hour   = yypvt[-2].IntVal;
	  yyans.minute = yypvt[-0].IntVal;
	  yyans.second = -1;		/* unspecified */
	} break;
case 96:
# line 301 "dateyacc.y"
{ yyans.hour   = yypvt[-2].IntVal;
	  yyans.minute = yypvt[-0].IntVal;
	  yyans.second = -1;		/* unspecified */
	} break;
case 97:
# line 306 "dateyacc.y"
{ yyans.hour   = yypvt[-4].IntVal;
	  yyans.minute = yypvt[-2].IntVal;
	  yyans.second = yypvt[-0].IntVal;
	} break;
case 98:
# line 311 "dateyacc.y"
{ yyans.hour   = yypvt[-4].IntVal;
	  yyans.minute = yypvt[-2].IntVal;
	  yyans.second = yypvt[-0].IntVal;
	} break;
case 99:
# line 316 "dateyacc.y"
{ yyans.hour   = yypvt[-2].IntVal / 100;
	  yyans.minute = yypvt[-2].IntVal % 100;
	  yyans.second = yypvt[-0].IntVal;
	} break;
case 100:
# line 321 "dateyacc.y"
{ yyans.hour   = yypvt[-2].IntVal / 100;
	  yyans.minute = yypvt[-2].IntVal % 100;
	  yyans.second = yypvt[-0].IntVal;
	} break;
case 101:
# line 326 "dateyacc.y"
{ yyans.hour   = yypvt[-0].IntVal / 10000;
	  yyans.minute = (yypvt[-0].IntVal % 10000) / 100;
	  yyans.second = yypvt[-0].IntVal % 100;
	} break;
case 102:
# line 333 "dateyacc.y"
{ if (yyans.hour < 1 || yyans.hour > 12)
	    yyans.hour = -1;		/* invalid */
	  else
	  { if (yyans.hour == 12) yyans.hour = 0;
	    yyans.hour += yypvt[-0].IntVal;		/* 0 for AM, 12 for PM */
	} } break;
case 103:
# line 342 "dateyacc.y"
{ yyans.zone = yypvt[-0].IntVal; yyans.dst = 0; } break;
case 104:
# line 344 "dateyacc.y"
{ yyans.zone = yypvt[-1].IntVal + 60; yyans.dst = 1; } break;
case 105:
# line 346 "dateyacc.y"
{ yyans.zone = yypvt[-0].IntVal; yyans.dst = 0; } break;
case 106:
# line 348 "dateyacc.y"
{ yyans.zone = yypvt[-1].IntVal + 60; yyans.dst = 1; } break;
case 107:
# line 350 "dateyacc.y"
{ yyans.zone = yypvt[-0].IntVal; yyans.dst = 1; } break;
case 108:
# line 352 "dateyacc.y"
{ yyans.zone = yypvt[-0].IntVal; yyans.dst = 1; } break;
case 109:
# line 354 "dateyacc.y"
{ yyans.zone = yypvt[-0].IntVal; yyans.dst = 0; } break;
case 110:
# line 356 "dateyacc.y"
{ yyans.zone = yypvt[-0].IntVal; yyans.dst = 0; } break;
case 111:
# line 358 "dateyacc.y"
{ yyans.zone = - yypvt[-0].IntVal; yyans.dst = 0; } break;
case 112:
# line 360 "dateyacc.y"
{ yyans.zone = - yypvt[-0].IntVal; yyans.dst = 0; } break;
case 113:
# line 363 "dateyacc.y"
{ yyans.zone = yypvt[-1].IntVal; yyans.dst = 0; } break;
case 114:
# line 365 "dateyacc.y"
{ yyans.zone = yypvt[-2].IntVal; yyans.dst = 0; } break;
case 115:
# line 367 "dateyacc.y"
{ yyans.zone = yypvt[-1].IntVal; yyans.dst = 0; } break;
case 116:
# line 369 "dateyacc.y"
{ yyans.zone = yypvt[-1].IntVal; yyans.dst = 0; } break;
case 117:
# line 371 "dateyacc.y"
{ yyans.zone = yypvt[-2].IntVal; yyans.dst = 0; } break;
case 118:
# line 373 "dateyacc.y"
{ yyans.zone = yypvt[-1].IntVal; yyans.dst = 0; } break;
case 119:
# line 375 "dateyacc.y"
{ yyans.zone = - yypvt[-1].IntVal; yyans.dst = 0; } break;
case 120:
# line 377 "dateyacc.y"
{ yyans.zone = - yypvt[-2].IntVal; yyans.dst = 0; } break;
case 121:
# line 379 "dateyacc.y"
{ yyans.zone = - yypvt[-1].IntVal; yyans.dst = 0; } break;
case 122:
# line 381 "dateyacc.y"
{ yyans.zone = - yypvt[-1].IntVal; yyans.dst = 0; } break;
case 123:
# line 383 "dateyacc.y"
{ yyans.zone = - yypvt[-2].IntVal; yyans.dst = 0; } break;
case 124:
# line 385 "dateyacc.y"
{ yyans.zone = - yypvt[-1].IntVal; yyans.dst = 0; } break;
case 125:
# line 389 "dateyacc.y"
{ yyval.IntVal = 60 * yypvt[-0].IntVal; } break;
case 126:
# line 391 "dateyacc.y"
{ yyval.IntVal = 60 * yypvt[-2].IntVal + yypvt[-0].IntVal; } break;
case 127:
# line 393 "dateyacc.y"
{ yyval.IntVal = 60 * yypvt[-2].IntVal + yypvt[-0].IntVal; } break;
case 128:
# line 395 "dateyacc.y"
{ yyval.IntVal = 60 * yypvt[-0].IntVal; } break;
case 129:
# line 397 "dateyacc.y"
{ yyval.IntVal = 60 * yypvt[-2].IntVal + yypvt[-0].IntVal; } break;
case 130:
# line 399 "dateyacc.y"
{ yyval.IntVal = 60 * yypvt[-2].IntVal + yypvt[-0].IntVal; } break;
case 131:
# line 401 "dateyacc.y"
{ yyval.IntVal = 60 * (yypvt[-0].IntVal / 100) | (yypvt[-0].IntVal % 100); } break;
case 132:
# line 405 "dateyacc.y"
{ yyans.hour   = yypvt[-0].IntVal / 100;	/* ajs */
	  yyans.minute = yypvt[-0].IntVal % 100;	/* ajs */
	  yyans.second = -1;		/* ajs */
	} break;
	}
	goto yystack;		/* reset registers in driver code */
}
