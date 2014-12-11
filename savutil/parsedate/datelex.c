/*$Log: datelex.c,v $
/*Revision 1.1.1.1  2001/04/24 15:40:54  finagler
/*New CVS import, starting with 2.09 (introduction of the GPL)
/*
 *Revision 1.2  1999/03/25 02:32:28  event
 *This appears to be a working version of the date indexer.  Work still needs to be done on the retrieval end...
 *
 * Revision 1.1  84/09/01  15:01:14  wales
 * Initial revision
 * 
 * Copyright (c) 1984 by Richard B. Wales
 *
 * Purpose:
 *
 *     Lexical analyzer for "parsedate" routine.  This lexer was orig-
 *     inally written in LEX, but rewriting it as an ad-hoc routine
 *     resulted in an enormous savings in space and a significant
 *     increase in speed.
 *
 * Usage:
 *
 *     Called as needed by the YACC parser ("dateyacc.c").  Not intended
 *     to be called from any other routine.
 *
 * Notes:
 *
 * Global contents:
 *
 *     int date_lex ()
 *         Returns the token number (from the YACC grammar) of the next
 *         token in the input string pointed to by the global variable
 *         "date_inbuf".  The global variable "date_lval" is set to the lexi-
 *         cal value (if any) of the token.  "date_inbuf" is set to point
 *         to the first character in the input string which is not a
 *         part of the token just recognized.
 *
 * Local contents:
 *
 *     struct wordtable *find_word (word) char *word;
 *         Returns a pointer to the entry in the "wordtable" array cor-
 *         responding to the string "word".  If "word" is not found, the
 *         returned value is NULL.
 */

/* ajs
 * ajs	Code added 850314 to allow NUM991231 and NUM99991231.
 * ajs	All added/changed lines contain "ajs" for easy searching.
 * ajs	*/

#ifdef RCSIDENT
static char rcsident[] = "$Header: /local/CVSROOT/RA/savutil/parsedate/datelex.c,v 1.1.1.1 2001/04/24 15:40:54 finagler Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#define yylval date_lval

#include "dateyacc.h"
#include "parsedate.h"

/* pointer to the input string */
char *date_inbuf;

/* "answer" structure */
struct parsedate date_ans;

/* Binary-search word table.
 * Entries must be sorted in ascending order on "text" value, and the
 * total number of entries must be one less than a power of 2.  "Filler"
 * entries (with "token" values of -1) are inserted at the beginning and
 * end of the table to pad it as necessary.
 */
#define WORDTABLE_SIZE 127	/* MUST be one less than power of 2 */
#define MAX_WORD_LENGTH 20	/* used to weed out overly long words
				 * in "date_lex".  Must be at least as long
				 * as the longest word in "wordtable",
				 * but may be longer.
				 */
struct wordtable
    {	char *text;
	int   token;
	int   lexval;
    } wordtable[WORDTABLE_SIZE] =
    {/* text            token           lexval */
      { "",		-1,		0 },
      { "",		-1,		0 },
      { "",		-1,		0 },
      { "",		-1,		0 },
      { "",		-1,		0 },
      { "",		-1,		0 },
      { "",		-1,		0 },
      { "",		-1,		0 },
      { "",		-1,		0 },
      { "",		-1,		0 },
      { "",		-1,		0 },
      { "A",		STD_ZONE,	60 },	/* UTC+1h */
      { "ACSST",	DST_ZONE,	630 },	/* Cent. Australia */
      { "ACST",		STD_ZONE,	570 },	/* Cent. Australia */
      { "ADT",		DST_ZONE,	-180 },	/* Atlantic (Canada) */
      { "AESST",	DST_ZONE,	660 },	/* E. Australia */
      { "AEST",		STD_ZONE,	600 },	/* E. Australia */
      { "AM",		AMPM,		0 },
      { "APR",		MONTH_NAME,	4 },
      { "APRIL",	MONTH_NAME,	4 },
      { "AST",		STD_ZONE,	-240 },	/* Atlantic (Canada) */
      { "AT",		0,		0 },	/* "at" (throwaway) */
      { "AUG",		MONTH_NAME,	8 },
      { "AUGUST",	MONTH_NAME,	8 },
      { "AWSST",	DST_ZONE,	540 },	/* W. Australia */
      { "AWST",		STD_ZONE,	480 },	/* W. Australia */
      { "B",		STD_ZONE,	120 },	/* UTC+2h */
      { "BST",		DST_ZONE,	60 },	/* Great Britain */
      { "C",		STD_ZONE,	180 },	/* UTC+3h */
      { "CDT",		DST_ZONE,	-300 },
      { "CST",		STD_ZONE,	-360 },
      { "D",		STD_ZONE,	240 },	/* UTC+4h */
      { "DEC",		MONTH_NAME,	12 },
      { "DECEMBER",	MONTH_NAME,	12 },
      { "DST",		DST_SUFFIX,	0 },
      { "E",		STD_ZONE,	300 },	/* UTC+5h */
      { "EDT",		DST_ZONE,	-240 },
      { "EET",		STD_ZONE,	120 },	/* Eastern Europe */
      { "EETDST",	DST_ZONE,	180 },	/* Eastern Europe */
      { "EST",		STD_ZONE,	-300 },
      { "F",		STD_ZONE,	360 },	/* UTC+6h */
      { "FEB",		MONTH_NAME,	2 },
      { "FEBRUARY",	MONTH_NAME,	2 },
      { "FRI",		DAY_NAME,	5 },
      { "FRIDAY",	DAY_NAME,	5 },
      { "G",		STD_ZONE,	420 },	/* UTC+7h */
      { "GMT",		STD_ZONE,	0 },
      { "H",		STD_ZONE,	480 },	/* UTC+8h */
      { "HDT",		DST_ZONE,	-540 },	/* Hawaii/Alaska */
      { "HST",		STD_ZONE,	-600 },	/* Hawaii/Alaska */
      { "I",		STD_ZONE,	540 },	/* UTC+9h */
      { "IST",		STD_ZONE,	120 },	/* Israel */
      { "JAN",		MONTH_NAME,	1 },
      { "JANUARY",	MONTH_NAME,	1 },
      { "JUL",		MONTH_NAME,	7 },
      { "JULY",		MONTH_NAME,	7 },
      { "JUN",		MONTH_NAME,	6 },
      { "JUNE",		MONTH_NAME,	6 },
      { "K",		STD_ZONE,	600 },	/* UTC+10h */
      { "L",		STD_ZONE,	660 },	/* UTC+11h */
      { "M",		STD_ZONE,	720 },	/* UTC+12h */
      { "MAR",		MONTH_NAME,	3 },
      { "MARCH",	MONTH_NAME,	3 },
      { "MAY",		MONTH_NAME,	5 },
      { "MDT",		DST_ZONE,	-360 },
      { "MET",		STD_ZONE,	60 },	/* Central Europe */
      { "METDST",	DST_ZONE,	120 },	/* Central Europe */
      { "MON",		DAY_NAME,	1 },
      { "MONDAY",	DAY_NAME,	1 },
      { "MST",		STD_ZONE,	-420 },
      { "N",		STD_ZONE,	-60 },	/* UTC-1h */
      { "NDT",		DST_ZONE,	-150 },	/* Nfld. (Canada) */
      { "NOV",		MONTH_NAME,	11 },
      { "NOVEMBER",	MONTH_NAME,	11 },
      { "NST",		STD_ZONE,	-210 },	/* Nfld. (Canada) */
      { "O",		STD_ZONE,	-120 },	/* UTC-2h */
      { "OCT",		MONTH_NAME,	10 },
      { "OCTOBER",	MONTH_NAME,	10 },
      { "ON",		0,		0 },	/* "on" (throwaway) */
      { "P",		STD_ZONE,	-180 },	/* UTC-3h */
      { "PDT",		DST_ZONE,	-420 },
      { "PM",		AMPM,		12 },
      { "PST",		STD_ZONE,	-480 },
      { "Q",		STD_ZONE,	-240 },	/* UTC-4h */
      { "R",		STD_ZONE,	-300 },	/* UTC-5h */
      { "S",		STD_ZONE,	-360 },	/* UTC-6h */
      { "SAT",		DAY_NAME,	6 },
      { "SATURDAY",	DAY_NAME,	6 },
      { "SEP",		MONTH_NAME,	9 },
      { "SEPT",		MONTH_NAME,	9 },
      { "SEPTEMBER",	MONTH_NAME,	9 },
      { "SUN",		DAY_NAME,	0 },
      { "SUNDAY",	DAY_NAME,	0 },
      { "T",		STD_ZONE,	-420 },	/* UTC-7h */
      { "THU",		DAY_NAME,	4 },
      { "THUR",		DAY_NAME,	4 },
      { "THURS",	DAY_NAME,	4 },
      { "THURSDAY",	DAY_NAME,	4 },
      { "TUE",		DAY_NAME,	2 },
      { "TUES",		DAY_NAME,	2 },
      { "TUESDAY",	DAY_NAME,	2 },
      { "U",		STD_ZONE,	-480 },	/* UTC-8h */
      { "UT",		STD_ZONE,	0 },
      { "UTC",		STD_ZONE,	0 },
      { "V",		STD_ZONE,	-540 },	/* UTC-9h */
      { "W",		STD_ZONE,	-600 },	/* UTC-10h */
      { "WED",		DAY_NAME,	3 },
      { "WEDNESDAY",	DAY_NAME,	3 },
      { "WEDS",		DAY_NAME,	3 },
      { "WET",		STD_ZONE,	0 },	/* Western Europe */
      { "WETDST",	DST_ZONE,	60 },	/* Western Europe */
      { "X",		STD_ZONE,	-660 },	/* UTC-11h */
      { "Y",		STD_ZONE,	-720 },	/* UTC-12h */
      { "YDT",		DST_ZONE,	-480 },	/* Yukon */
      { "YST",		STD_ZONE,	-540 },	/* Yukon */
      { "Z",		STD_ZONE,	0 },	/* UTC */
      { "\177",		-1,		0 },
      { "\177",		-1,		0 },
      { "\177",		-1,		0 },
      { "\177",		-1,		0 },
      { "\177",		-1,		0 },
      { "\177",		-1,		0 },
      { "\177",		-1,		0 },
      { "\177",		-1,		0 },
      { "\177",		-1,		0 },
      { "\177",		-1,		0 },
      { "\177",		-1,		0 }
    };
static struct wordtable *find_word();

/* int date_lex ()
 *     Return the next token for the YACC parser.
 */
int
date_lex ()
{   static char buffer[MAX_WORD_LENGTH+1];
    register char *c, *d;
    register struct wordtable *wt;
    register int num, ndgts;

  restart:
    /* We will return here if an invalid input token is detected. */
    c = buffer; d = date_inbuf;

    /* Skip over blanks, tabs, commas, and parentheses. */
    do { *c = *d++; }
	while (*c == ' ' || *c == '\t' || *c == ','
	       || *c == '(' || *c == ')');

    /* A zero (null) byte signals the end of the input. */
    if (*c == 0)
    {	date_inbuf = --d;		/* stay put on the null */
	return 0;
    }

    /* Process a word (looking it up in "wordtable"). */
    if ((*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z'))
    {	if (*c >= 'a' && *c <= 'z') *c += 'A' - 'a';
	while (c < buffer + MAX_WORD_LENGTH
	       && ((*d >= 'A' && *d <= 'Z')
		   || (*d >= 'a' && *d <= 'z')))
	{   *++c = *d++;
	    if (*c >= 'a' && *c <= 'z') *c += 'A' - 'a';
	}
	if ((*d >= 'A' && *d <= 'Z') || (*d >= 'a' && *d <= 'z'))
	{   /* Word is too long (over MAX_WORD_LENGTH characters). */
	    do { d++; } while ((*d >= 'A' && *d <= 'Z')
			       || (*d >= 'a' && *d <= 'z'));
	    date_inbuf = d;
	    goto error;
	}
	*++c = 0; date_inbuf = d;
	if ((wt = find_word (buffer)) == NULL) goto error;
	if (wt->token == 0) goto restart;	/* ignore this word */
	date_lval.IntVal = wt->lexval;
	return wt->token;
    }

    /* Process a number. */
    if (*c >= '0' && *c <= '9')
    {	num = *c - '0'; ndgts = 1;
	for (ndgts = 1; ndgts < 8 && *d >= '0' && *d <= '9'; ndgts++)  /* ajs */
	    num = 10*num + (*d++ - '0');
	if (*d >= '0' && *d <= '9')
	{   /* Number is too long (over 8 digits). */		/* ajs */
	    do { d++; } while (*d >= '0' && *d <= '9');
	    date_inbuf = d;
	    goto error;
	}
	date_inbuf = d;
	date_lval.IntVal = num;
	switch (ndgts)
	{   case 1:  return NUM9;
	    case 2:  if (num <= 23) return NUM23;
		     if (num <= 59) return NUM59;
		     /*otherwise*/  return NUM99;
	    case 3:
	    case 4:  if (num/100 <= 23 && num%100 <= 59) return NUM2359;
		     /*otherwise*/                       return NUM9999;
	    case 5:
	    case 6:  if (num/10000 <= 23
			 && (num%10000)/100 <= 59
			 && num%100 <= 59)
			 return NUM235959;
		     if ((((num % 10000) / 100) <= 12)	/* ajs */
		      &&  ((num % 100) <= 31))		/* ajs */
			 return NUM991231;		/* ajs */
		     goto error;
	    case 8:  if ((((num % 10000) / 100) <= 12)	/* ajs */
		      &&  ((num % 100) <= 31))		/* ajs */
			 return NUM99991231;		/* ajs */
		     goto error;			/* ajs */
	    default: goto error;
    }	}

    /* Pass back the following delimiter tokens verbatim.. */
    if (*c == '-' || *c == '+' || *c == '/' || *c == ':' || *c == '.')
    {	date_inbuf = d;
	return *c;
    }

  error:
    /* An unidentified character was found in the input. */
    date_inbuf = d;
    if (date_ans.error == NULL) date_ans.error = date_inbuf;
    goto restart;
}

/* struct wordtable *find_word (word) char *word;
 *     Look up a word in the "wordtable" array via a binary search.
 */
static
struct wordtable *
find_word (word)
    register char *word;
{   register int low, mid, high;
    register int comparison;

    low = -1;
    high = WORDTABLE_SIZE;
    while (low+1 < high)
    {	mid = (low + high) / 2;
	comparison = strcmp (wordtable[mid].text, word);
	if (comparison == 0) return wordtable+mid;
	if (comparison > 0)  high = mid;
	else                 low = mid;
    }
    return NULL;
}
