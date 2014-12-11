/*LINTLIBRARY*/

/*$Log: parsedate.c,v $
/*Revision 1.1.1.1  2001/04/24 15:40:54  finagler
/*New CVS import, starting with 2.09 (introduction of the GPL)
/*
 *Revision 1.2  1999/03/25 02:32:30  event
 *This appears to be a working version of the date indexer.  Work still needs to be done on the retrieval end...
 *
 * Revision 1.1  84/09/01  15:01:30  wales
 * Initial revision
 * 
 * Copyright (c) 1984 by Richard B. Wales
 *
 * Purpose:
 *
 *     Manipulate character strings representing dates.
 *
 * Usage:
 *
 *     #include <parsedate.h>
 *
 *     char date;
 *     struct parsedate *pd;
 *
 *     pd = parsedate (date);
 *
 *     compute_unixtime (pd);
 *
 *     break_down_unixtime (pd);
 *
 *     date = mail_date_string (pd);
 *
 *     date = uucp_date_string (pd);
 *
 * Notes:
 *
 *     The returned value from "parsedate", "mail_date_string", or
 *     "uucp_date_string" points to static data whose contents are
 *     overwritten by the next call to the same routine.
 *
 *     "compute_unixtime" is implicitly called by "parsedate".
 *
 * Global contents:
 *
 *     struct parsedate *parsedate (date) char *date;
 *         Parse a character string representing a date and time into
 *         individual values in a "struct parsedate" data structure.
 *    
 *     compute_unixtime (pd) struct parsedate *pd;
 *         Given a mostly filled-in "struct parsedate", compute the day
 *         of the week and the internal UNIX representation of the date.
 *    
 *     break_down_unixtime (pd) struct parsedate *pd;
 *         Compute the date and time corresponding to the "unixtime" and
 *         "zone" values in a "struct parsedate".
 *    
 *     char *mail_date_string (pd) struct parsedate *pd;
 *         Generate a character string representing a date and time in
 *         the RFC822 (ARPANET mail standard) format.
 *    
 *     char *uucp_date_string (pd) struct parsedate *pd;
 *         Generate a character string representing a date and time in
 *         the UUCP mail format.
 *
 * Local contents:
 *
 *     None.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include "parsedate.h"

#ifdef RCSIDENT
static char rcsident[] = "$Header: /local/CVSROOT/RA/savutil/parsedate/parsedate.c,v 1.1.1.1 2001/04/24 15:40:54 finagler Exp $";
static char rcs_parsedate_hdr[] = RCS_PARSEDATE_HDR;
#endif

void compute_unixtime();
void break_down_unixtime();
int date_parse();

/* Number of seconds in various time intervals. */
#define SEC_PER_MIN  60
#define SEC_PER_HOUR (60*SEC_PER_MIN)
#define SEC_PER_DAY  (24*SEC_PER_HOUR)
#define SEC_PER_YEAR (365*SEC_PER_DAY)

/* Number of days in each month. */
static int monthsize[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };

/* Three-letter abbreviations of month and day names. */
static char monthname[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
static char dayname[]   = "SunMonTueWedThuFriSat";

/* struct parsedate *parsedate (date) char *date;
 *     Analyze a character string representing a date and time.  The
 *     returned value points to a data structure with the desired
 *     information.  (NOTE:  The returned value points to static data
 *     whose contents are overwritten by each call.)
 */
struct parsedate *
parsedate (date)
    register char *date;
{
    register int year_save;
    extern struct parsedate date_ans;
    extern char *date_inbuf;
/*    extern char *sprintf(); */

    /* Initialize the returned-value structure. */
    date_ans.unixtime  = -1;
    date_ans.year      = -1;
    date_ans.month     = -1;
    date_ans.day       = -1;
    date_ans.hour      = -1;
    date_ans.minute    = -1;
    date_ans.second    = -1;
    date_ans.zone      = -1;
    date_ans.dst       = -1;
    date_ans.weekday   = -1;
    date_ans.c_weekday = -1;
    date_ans.error     =  NULL;

    /* Parse the argument string. */
    date_inbuf = date;
    if (date_parse () != 0 && date_ans.error == NULL)
      date_ans.error = date_inbuf;

    /* Validate the day of the month, compute/validate the day of the
     * week, and compute the internal UNIX form of the time.  See if
     * "compute_unixtime" found fault with the year or the day of the
     * month.  (Note that we have to remember the original "year" value
     * because it might legitimately have been -1 to begin with.)
     */
    year_save = date_ans.year; compute_unixtime (&date_ans);
    if (date_ans.error == NULL
	&& (date_ans.year != year_save
	    || (date_ans.month > 0 && date_ans.day < 0)
	    || (date_ans.month < 0 && date_ans.day > 0)))
	date_ans.error = date_inbuf;

    return &date_ans;
}

/* compute_unixtime (pd) struct parsedate *pd;
 *     Given a mostly filled-in "struct parsedate", compute the day of
 *     the week and the internal UNIX representation of the date.
 *
 *     A year before 1600 will be rejected and replaced with -1.  A
 *     date from 1600 on which falls outside the range representable in
 *     internal UNIX form will still have the correct day of the week
 *     computed.
 *
 *     The day of the week is always computed on the assumption that the
 *     Gregorian calendar is in use.  Days of the week for dates in the
 *     far future may turn out to be incorrect if any changes are made
 *     to the calendar between now and then.
 */
void
compute_unixtime (pd)
    register struct parsedate *pd;
{   register int weekday, n, l, a;

    /* Validate the year. */
    if (pd->year >= 0 && pd->year < 1600) pd->year = -1;

    /* Validate the day of the month.  Also calculate the number of days
     * in February (even if this is not February, we will need the num-
     * ber of days in February later on when computing the UNIX time).
     */
    if (pd->month > 0)
    {	if      (pd->year < 0)        monthsize[2] = 29;
	else if (pd->year %   4 != 0) monthsize[2] = 28;
	else if (pd->year % 100 != 0) monthsize[2] = 29;
	else if (pd->year % 400 != 0) monthsize[2] = 28;
	else                          monthsize[2] = 29;
	if (pd->day <= 0 || pd->day > monthsize[pd->month])
	    pd->day = -1;
    }

    /* Compute the day of the week.  The next several lines constitute a
     * perpetual-calendar formula.  Note, of course, that the "claimed"
     * day of the week (pd->c_weekday) is ignored here.
     */
    if (pd->year > 0 && pd->month > 0 && pd->day > 0)
    {	if (pd->month >= 3) n = pd->year / 100,
			    l = pd->year % 100;
	else                n = (pd->year-1) / 100,
			    l = (pd->year-1) % 100;
	a = (26 * ((pd->month+9)%12 + 1) - 2) / 10;
	weekday = (a+(l>>2)+(n>>2)+l-(n+n)+pd->day);
	while (weekday < 0) weekday += 7;
	pd->weekday = weekday % 7;
    }

    /* Figure out the internal UNIX form of the date. */
    if (pd->year >= 1969 && pd->year <= 2038
	&& pd->month > 0 && pd->day > 0
	&& pd->hour >= 0 && pd->minute >= 0
	&& pd->zone != -1 && pd->zone > -1440 && pd->zone < 1440)
    {	pd->unixtime =
	      SEC_PER_YEAR * (pd->year - 1970)
	    + SEC_PER_DAY  * ((pd->year - 1969) / 4)
	    /* month is taken care of later */
	    + SEC_PER_DAY  * (pd->day - 1)
	    + SEC_PER_HOUR * pd->hour
	    + SEC_PER_MIN  * pd->minute
	    /* seconds are taken care of later */
	    - SEC_PER_MIN  * pd->zone;
	if (pd->second >= 0)
	    pd->unixtime += pd->second;
	for (n = pd->month - 1; n > 0; n--)
	    pd->unixtime += SEC_PER_DAY * monthsize[n];
	if (pd->unixtime < 0) pd->unixtime = -1;
    }
    else pd->unixtime = -1;
}

/* break_down_unixtime (pd) struct parsedate *pd;
 *     Given the "unixtime" and "zone" fields of a "struct parsedate",
 *     compute the values of the "year", "month", "day", "hour", "min-
 *     ute", "second", and "weekday" fields.  The "dst" and "error"
 *     fields of the structure are not used or modified.
 */
void
break_down_unixtime (pd)
    register struct parsedate *pd;
{   register unsigned long timevalue;
    register int m, n;

    /* Validate the "unixtime" and "zone" fields. */
    if (pd->unixtime < 0
	|| pd->zone == -1 || pd->zone <= -1440 || pd->zone >= 1440)
    {	/* Sorry, can't do it. */
	pd->year = -1; pd->month = -1; pd->day = -1;
	pd->hour = -1; pd->minute = -1; pd->second = -1;
	pd->weekday = -1;
	return;
    }

    /* Even though "pd->unixtime" must be non-negative, and thus cannot
     * indicate a time earlier than 1970, a negative "pd->zone" could
     * cause the local date to be Wednesday, 31 December 1969.  Such a
     * date requires special handling.
     *
     * A local date earlier than 31 December 1969 is impossible because
     * "pd->zone" must represent a time-zone shift of less than a day.
     */
    if (pd->zone < 0 && pd->unixtime + SEC_PER_MIN * pd->zone < 0)
    {	pd->year = 1969; pd->month = 12; pd->day = 31;
	pd->weekday = 3;    /* Wednesday */
	timevalue = pd->unixtime + SEC_PER_MIN * pd->zone + SEC_PER_DAY;
	/* Note:  0 <= timevalue < SEC_PER_DAY */
	pd->hour = timevalue / SEC_PER_HOUR;
	pd->minute = (timevalue % SEC_PER_HOUR) / SEC_PER_MIN;
	pd->second = timevalue % SEC_PER_MIN;
	return;
    }

    /* Handle the general case (local time is 1970 or later). */
    timevalue = pd->unixtime + SEC_PER_MIN * pd->zone;

    /* day of the week (1 January 1970 was a Thursday) . . . */
    pd->weekday = (timevalue/SEC_PER_DAY + 4 /* Thursday */) % 7;

    /* year (note that the only possible century year here is 2000,
     * a leap year -- hence no special tests for century years are
     * needed) . . .
     */
    for (m = 1970; ; m++)
    {	n = (m%4==0) ? SEC_PER_YEAR+SEC_PER_DAY : SEC_PER_YEAR;
	if (n > timevalue) break;
	timevalue -= n;
    }
    pd->year = m;
    monthsize[2] = (m%4==0) ? 29 : 28;

    /* month . . . */
    for (m = 1; ; m++)
    {	n = SEC_PER_DAY * monthsize[m];
	if (n > timevalue) break;
	timevalue -= n;
    }
    pd->month = m;

    /* day, hour, minute, and second . . . */
    pd->day    = (timevalue / SEC_PER_DAY) + 1;
    pd->hour   = (timevalue % SEC_PER_DAY) / SEC_PER_HOUR;
    pd->minute = (timevalue % SEC_PER_HOUR) / SEC_PER_MIN;
    pd->second = timevalue % SEC_PER_MIN;
}

/* char *mail_date_string (pd) struct parsedate *pd;
 *     Generate a character string representing a date and time in the
 *     RFC822 (ARPANET mail standard) format.  A value of NULL is re-
 *     turned if "pd" does not contain all necessary data fields.
 *     (NOTE:  The returned value points to static data whose contents
 *     are overwritten by each call.)
 */
char *
mail_date_string (pd)
    register struct parsedate *pd;
{   register char *c;
    static char answer[50];

    /* Check the day of the month and compute the day of the week. */
    compute_unixtime (pd);

    /* Make sure all required fields are present. */
    if (pd->year < 0 || pd->month < 0 || pd->day < 0
	|| pd->hour < 0 || pd->minute < 0
	|| pd->zone == -1 || pd->zone <= -1440 || pd->zone >= 1440)
	return NULL;		/* impossible to generate string */

    /* Generate the answer string. */
    sprintf (answer,
	     "%.3s, %d %.3s %d %02d:%02d",
	     dayname + 3*pd->weekday,
	     pd->day, monthname + 3*(pd->month-1),
	     (pd->year >= 1960 && pd->year <= 1999)
		 ? pd->year - 1900 : pd->year,
	     pd->hour, pd->minute);
    c = answer + strlen (answer);
    if (pd->second >= 0) sprintf (c, ":%02d", pd->second), c += 3;
    *c++ = ' ';
    switch (pd->zone)
    {	/* NOTE:  Only zone abbreviations in RFC822 are used here. */
	case    0: strcpy (c, pd->dst ? "+0000" : "GMT");   break;
	case -240: strcpy (c, pd->dst ? "EDT"   : "-0400"); break;
	case -300: strcpy (c, pd->dst ? "CDT"   : "EST");   break;
	case -360: strcpy (c, pd->dst ? "MDT"   : "CST");   break;
	case -420: strcpy (c, pd->dst ? "PDT"   : "MST");   break;
	case -480: strcpy (c, pd->dst ? "-0800" : "PST");   break;
	default:
	    if (pd->zone >= 0)
		 sprintf (c, "+%02d%02d",  pd->zone/60,  pd->zone%60);
	    else sprintf (c, "-%02d%02d", -pd->zone/60, -pd->zone%60);
    }

    return answer;
}

/* char *uucp_date_string (pd) struct parsedate *pd;
 *     Generate a character string representing a date and time in the
 *     UUCP mail format.  A value of NULL is returned if "pd" does not
 *     contain all necessary data fields.  (NOTE:  The returned value
 *     points to static data whose contents are overwritten by each
 *     call.)
 */
char *
uucp_date_string (pd)
    register struct parsedate *pd;
{   register char *c;
    static char answer[50];

    /* Check the day of the month and compute the day of the week. */
    compute_unixtime (pd);

    /* Make sure all required fields are present. */
    if (pd->year < 0 || pd->month < 0 || pd->day < 0
	|| pd->hour < 0 || pd->minute < 0
	|| pd->zone == -1 || pd->zone <= -1440 || pd->zone >= 1440)
	return NULL;		/* impossible to generate string */

    /* Generate the answer string. */
    sprintf (answer,
	     "%.3s %.3s %d %02d:%02d",
	     dayname + 3*pd->weekday,
	     monthname + 3*(pd->month-1), pd->day,
	     pd->hour, pd->minute);
    c = answer + strlen (answer);
    if (pd->second >= 0) sprintf (c, ":%02d", pd->second), c += 3;
    switch (pd->zone)
    {	/* NOTE:  Only zone abbreviations in RFC822 are used here. */
	case    0: strcpy (c, pd->dst ? "+0000" : "-GMT");   break;
	case -240: strcpy (c, pd->dst ? "-EDT"  : "-0400"); break;
	case -300: strcpy (c, pd->dst ? "-CDT"  : "-EST");   break;
	case -360: strcpy (c, pd->dst ? "-MDT"  : "-CST");   break;
	case -420: strcpy (c, pd->dst ? "-PDT"  : "-MST");   break;
	case -480: strcpy (c, pd->dst ? "-0800" : "-PST");   break;
	default:
	    if (pd->zone >= 0)
		 sprintf (c, "+%02d%02d",  pd->zone/60,  pd->zone%60);
	    else sprintf (c, "-%02d%02d", -pd->zone/60, -pd->zone%60);
    }
    c = answer + strlen (answer);
    sprintf (c, " %d", pd->year);

    return answer;
}
