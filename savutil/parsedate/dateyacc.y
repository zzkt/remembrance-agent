/*$Log: dateyacc.y,v $
/*Revision 1.1.1.1  2001/04/24 15:40:54  finagler
/*New CVS import, starting with 2.09 (introduction of the GPL)
/*
 *Revision 1.2  1999/03/25 02:32:30  event
 *This appears to be a working version of the date indexer.  Work still needs to be done on the retrieval end...
 *
 * Revision 1.1  84/09/01  15:01:22  wales
 * Initial revision
 * 
 * Copyright (c) 1984 by Richard B. Wales
 *
 * Purpose:
 *
 *     YACC parser for "parsedate" routine.
 *
 * Usage:
 *
 *     Called as needed by the "parsedate" routine in "parsedate.c".
 *     Not intended to be called from any other routine.
 *
 * Notes:
 *
 * Global contents:
 *
 *     int yyparse ()
 *         Parses the date string pointed to by the global variable
 *         "yyinbuf".  Sets the appropriate fields in the global data
 *         structure "yyans".  The returned value is 1 if there was a
 *         syntax error, 0 if there was no error.
 *
 * Local contents:
 *
 *     None.
 */

/* ajs
 * ajs	Code added on 850314 to allow	goal	  := year.date '.' time
 * ajs				and	year.date := [CC]YYMMDD (YY > 23)
 * ajs	All added lines contain "ajs" for easy searching.
 * ajs	*/

%{
#ifdef RCSIDENT
static char rcsident[] = "$Header: /local/CVSROOT/RA/savutil/parsedate/dateyacc.y,v 1.1.1.1 2001/04/24 15:40:54 finagler Exp $";
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
%}

%union {
    int IntVal;
}

%token	DAY_NAME
%token	MONTH_NAME
%token	NUM9 NUM23 NUM59 NUM99 NUM2359 NUM9999 NUM235959
%token	NUM991231 NUM99991231				/* ajs */
%token	AMPM
%token	STD_ZONE DST_ZONE DST_SUFFIX

%type	<IntVal>	DAY_NAME
%type	<IntVal>	MONTH_NAME
%type	<IntVal>	NUM9 NUM23 NUM59 NUM99 NUM2359 NUM9999 NUM235959
%type	<IntVal>	NUM991231 NUM99991231		/* ajs */
%type	<IntVal>	AMPM
%type	<IntVal>	STD_ZONE DST_ZONE
%type	<IntVal>	num59 num zone.offset

%start	goal
%%

num59:
    NUM23
  | NUM59

num:
    NUM9
  | num59

goal:
    date
  | date dayname
  | date dayname time
  | date dayname time year
  | date dayname year
  | date dayname year time
  | date time
  | date time dayname
  | date time dayname year
  | date time year
  | date time year dayname
  | date.year
  | date.year dayname
  | date.year dayname time
  | date.year time
  | date.year time dayname
  | dayname date
  | dayname date time
  | dayname date time year
  | dayname date.year
  | dayname date.year time
  | dayname time date
  | dayname time date.year
  | dayname time year.date
  | dayname year.date
  | dayname year.date time
  | dayname year time date
  | time
  | time date
  | time date dayname
  | time date dayname year
  | time date.year
  | time date.year dayname
  | time dayname date
  | time dayname date.year
  | time dayname year.date
  | time year.date
  | time year.date dayname
  | time year dayname date
  | year.date
  | year.date dayname
  | year.date dayname time
  | year.date time
  | year.date time dayname
  | year dayname date
  | year dayname date time
  | year dayname time date
  | year time date
  | year time date dayname
  | year time dayname date
  | NUM2359
	{ yyans.hour   = $1 / 100;
	  yyans.minute = $1 % 100;
	  yyans.second = -1;		/* unspecified */
	}
  | dayname
  | yymmdd '.' time2359			/* ajs */
  | yymmdd '.' time			/* ajs */
  | yymmdd '.' time dayname		/* ajs */
  | error
	{ extern char *yyinbuf;
	  if (yyans.error == NULL) yyans.error = yyinbuf;
	}

dayname:
    DAY_NAME
	{ yyans.c_weekday = $1; }
  | DAY_NAME '.'
	{ yyans.c_weekday = $1; }

date.year:
    date year
  | hyphen.date '-' year
  | slash.date '/' year

year.date:
    year date
  /* | year '-' hyphen.date	(leads to parser conflict) */
  | year '/' slash.date
  | yymmdd					/* ajs */
						/* ajs */
yymmdd:						/* ajs */
    NUM991231					/* ajs */
	{ yyans.year  = ($1 / 10000) + 1900;	/* ajs */
	  yyans.month = ($1 % 10000) / 100;	/* ajs */
	  yyans.day   = ($1 % 100);		/* ajs */
	}					/* ajs */
/*| NUM235959	(leads to parser conflict) */	/* ajs */
  | NUM99991231					/* ajs */
	{ yyans.year  = ($1 / 10000);		/* ajs */
	  yyans.month = ($1 % 10000) / 100;	/* ajs */
	  yyans.day   = ($1 % 100);		/* ajs */
	}					/* ajs */

date:
    num month.name
	{ yyans.day = $1; }
  | month.name num
	{ yyans.day = $2; }
  | num num
	{ yyans.month = $1; yyans.day = $2; }

hyphen.date:
    num '-' month.name
	{ yyans.day = $1; }
  | month.name '-' num
	{ yyans.day = $3; }
  | num '-' num
	{ yyans.month = $1; yyans.day = $3; }

slash.date:
    num '/' month.name
	{ yyans.day = $1; }
  | month.name '/' num
	{ yyans.day = $3; }
  | num '/' num
	{ yyans.month = $1; yyans.day = $3; }

year:
    NUM99		/* precludes two-digit date before 1960 */
	{ yyans.year = 1900 + $1; }
  | NUM2359
	{ yyans.year = $1; }
  | NUM9999
	{ yyans.year = $1; }

month.name:
    MONTH_NAME
	{ yyans.month = $1; }
  | MONTH_NAME '.'
	{ yyans.month = $1; }

time:
    hour.alone
  | hour am.pm
  | hour zone
  | hour am.pm zone

hour:
    NUM2359
	{ yyans.hour   = $1 / 100;
	  yyans.minute = $1 % 100;
	  yyans.second = -1;		/* unspecified */
	}
  | hour.alone

hour.alone:
    NUM9 ':' num59
	{ yyans.hour   = $1;
	  yyans.minute = $3;
	  yyans.second = -1;		/* unspecified */
	}
  | NUM9 '.' num59
	{ yyans.hour   = $1;
	  yyans.minute = $3;
	  yyans.second = -1;		/* unspecified */
	}
  | NUM9 ':' num59 ':' num59
	{ yyans.hour   = $1;
	  yyans.minute = $3;
	  yyans.second = $5;
	}
  | NUM9 '.' num59 '.' num59
	{ yyans.hour   = $1;
	  yyans.minute = $3;
	  yyans.second = $5;
	}
  | NUM23 ':' num59
	{ yyans.hour   = $1;
	  yyans.minute = $3;
	  yyans.second = -1;		/* unspecified */
	}
  | NUM23 '.' num59
	{ yyans.hour   = $1;
	  yyans.minute = $3;
	  yyans.second = -1;		/* unspecified */
	}
  | NUM23 ':' num59 ':' num59
	{ yyans.hour   = $1;
	  yyans.minute = $3;
	  yyans.second = $5;
	}
  | NUM23 '.' num59 '.' num59
	{ yyans.hour   = $1;
	  yyans.minute = $3;
	  yyans.second = $5;
	}
  | NUM2359 ':' num59
	{ yyans.hour   = $1 / 100;
	  yyans.minute = $1 % 100;
	  yyans.second = $3;
	}
  | NUM2359 '.' num59
	{ yyans.hour   = $1 / 100;
	  yyans.minute = $1 % 100;
	  yyans.second = $3;
	}
  | NUM235959
	{ yyans.hour   = $1 / 10000;
	  yyans.minute = ($1 % 10000) / 100;
	  yyans.second = $1 % 100;
	}

am.pm:
    AMPM
	{ if (yyans.hour < 1 || yyans.hour > 12)
	    yyans.hour = -1;		/* invalid */
	  else
	  { if (yyans.hour == 12) yyans.hour = 0;
	    yyans.hour += $1;		/* 0 for AM, 12 for PM */
	} }

zone:
    STD_ZONE
	{ yyans.zone = $1; yyans.dst = 0; }
  | STD_ZONE DST_SUFFIX
	{ yyans.zone = $1 + 60; yyans.dst = 1; }
  | '-' STD_ZONE
	{ yyans.zone = $2; yyans.dst = 0; }
  | '-' STD_ZONE DST_SUFFIX
	{ yyans.zone = $2 + 60; yyans.dst = 1; }
  | DST_ZONE
	{ yyans.zone = $1; yyans.dst = 1; }
  | '-' DST_ZONE
	{ yyans.zone = $2; yyans.dst = 1; }
  | '+' zone.offset
	{ yyans.zone = $2; yyans.dst = 0; }
  | '-' '+' zone.offset
	{ yyans.zone = $3; yyans.dst = 0; }
  | '-' zone.offset
	{ yyans.zone = - $2; yyans.dst = 0; }
  | '-' '-' zone.offset
	{ yyans.zone = - $3; yyans.dst = 0; }
/* new stuff follows  -cabbage */
  | '+' zone.offset STD_ZONE
	{ yyans.zone = $2; yyans.dst = 0; }
  | '+' zone.offset STD_ZONE DST_SUFFIX
	{ yyans.zone = $2; yyans.dst = 0; }
  | '+' zone.offset DST_ZONE
	{ yyans.zone = $2; yyans.dst = 0; }
  | '-' '+' zone.offset STD_ZONE
	{ yyans.zone = $3; yyans.dst = 0; }
  | '-' '+' zone.offset STD_ZONE DST_SUFFIX
	{ yyans.zone = $3; yyans.dst = 0; }
  | '-' '+' zone.offset DST_ZONE
	{ yyans.zone = $3; yyans.dst = 0; }
  | '-' zone.offset STD_ZONE
	{ yyans.zone = - $2; yyans.dst = 0; }
  | '-' zone.offset STD_ZONE DST_SUFFIX
	{ yyans.zone = - $2; yyans.dst = 0; }
  | '-' zone.offset DST_ZONE
	{ yyans.zone = - $2; yyans.dst = 0; }
  | '-' '-' zone.offset STD_ZONE
	{ yyans.zone = - $3; yyans.dst = 0; }
  | '-' '-' zone.offset STD_ZONE DST_SUFFIX
	{ yyans.zone = - $3; yyans.dst = 0; }
  | '-' '-' zone.offset DST_ZONE
	{ yyans.zone = - $3; yyans.dst = 0; }

zone.offset:
    NUM9
	{ $$ = 60 * $1; }
  | NUM9 ':' num59
	{ $$ = 60 * $1 + $3; }
  | NUM9 '.' num59
	{ $$ = 60 * $1 + $3; }
  | NUM23
	{ $$ = 60 * $1; }
  | NUM23 ':' num59
	{ $$ = 60 * $1 + $3; }
  | NUM23 '.' num59
	{ $$ = 60 * $1 + $3; }
  | NUM2359
	{ $$ = 60 * ($1 / 100) | ($1 % 100); }

time2359:				/* ajs */
    NUM2359				/* ajs */
	{ yyans.hour   = $1 / 100;	/* ajs */
	  yyans.minute = $1 % 100;	/* ajs */
	  yyans.second = -1;		/* ajs */
	}				/* ajs */

%%
