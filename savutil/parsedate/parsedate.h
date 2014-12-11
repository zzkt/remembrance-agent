/* $Log: parsedate.h,v $
/* Revision 1.1.1.1  2001/04/24 15:40:54  finagler
/* New CVS import, starting with 2.09 (introduction of the GPL)
/*
 * Revision 1.2  1999/03/25 02:32:31  event
 * This appears to be a working version of the date indexer.  Work still needs to be done on the retrieval end...
 *
 * Revision 1.1  84/09/01  15:01:38  wales
 * Initial revision
 * 
 * Copyright (c) 1984 by Richard B. Wales
 *
 */
#ifdef RCSIDENT
#define RCS_PARSEDATE_HDR "$Header: /local/CVSROOT/RA/savutil/parsedate/parsedate.h,v 1.1.1.1 2001/04/24 15:40:54 finagler Exp $"
#endif

/* Data structure returned by "parsedate".
 *
 * A value of NULL for "error" means that no syntax errors were detected
 * in the argument value.  A non-NULL value points to the byte position
 * within the argument string at which it was discovered that an error
 * existed.
 *
 * A value of -1 means that the field was never given a value, or that
 * the value supplied was invalid.  (A side effect of this convention is
 * that a time zone offset of -1 -- i.e., one minute west of GMT -- is
 * indistinguishable from an invalid or unspecified time zone offset.
 * Since the likelihood of "-0001" being a legitimate time zone is nil,
 * banning it is a small price to pay for the uniformity of using -1 as
 * a "missing/invalid" indication for all fields.)
 */
struct parsedate
    {	long unixtime;	/* UNIX internal representation of time */
	char *error;	/* NULL = OK; non-NULL = error */
	int year;	/* year (1600 on) */
	int month;	/* month (1-12) */
	int day;	/* day of month (1-31) */
	int hour;	/* hour (0-23) */
	int minute;	/* minute (0-59) */
	int second;	/* second (0-59) */
	int zone;	/* time zone offset in minutes -- "+" or "-" */
	int dst;	/* daylight savings time (0 = no, 1 = yes) */
	int weekday;	/* real day of week (0-6; 0 = Sunday) */
	int c_weekday;	/* claimed day of week (0-6; 0 = Sunday) */
    };

struct parsedate *parsedate();
