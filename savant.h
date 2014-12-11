#ifndef _SAVANT_
#define _SAVANT_

/*
All code included in versions up to and including 2.09:
   Copyright (C) 1996-2001 Massachusetts Institute of Technology.

All modifications subsequent to version 2.09 are copyright Bradley
Rhodes or their respective authors.

Developed by Bradley Rhodes at the Media Laboratory, MIT, Cambridge,
Massachusetts, with support from British Telecom and Merrill Lynch.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.  For commercial licensing under other
terms, please consult the MIT Technology Licensing Office.

This program may be subject to the following US and/or foreign
patents (pending): "Method and Apparatus for Automated,
Context-Dependent Retrieval of Information," MIT Case No. 7870TS. If
any of these patents are granted, royalty-free license to use this
and derivative programs under the GNU General Public License are
hereby granted.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/

extern void SavantError(int errcode, char *errstring);

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <ctype.h>
#include <math.h>
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <time.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
/*
#ifdef HAVE_VALUES_H
#include <values.h>
#endif
*/
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

typedef struct dirent savant_direct;

/* The following is to try to maintain a standard size 
   of the data types used in the on-disk databases. */

#if (SIZEOF_INT == 4)
typedef int DB_INT;
typedef unsigned int DB_UINT;
#elif (SIZEOF_SHORT == 4)
typedef short DB_INT;
typedef unsigned short DB_UINT;
#elif (SIZEOF_LONG == 4)
typedef long DB_INT;
typedef unsigned long DB_UINT;
#else
typedef int DB_INT;
typedef unsigned int DB_UINT;
#endif

#if (SIZEOF_SHORT == 2)
typedef short DB_SHORT;
typedef unsigned short DB_USHORT;
#elif (SIZEOF_INT == 2)
typedef short DB_INT;
typedef unsigned int DB_USHORT;
#else
typedef short DB_SHORT;
typedef unsigned short DB_USHORT;
#endif

#if (SIZEOF_FLOAT == 4)
typedef float DB_FLOAT;
#elif (SIZEOF_DOUBLE == 4)
typedef double DB_FLOAT;
#else
typedef float DB_FLOAT;
#endif

#ifdef RA_MIN
#undef RA_MIN
#endif
#ifdef RA_MAX
#undef RA_MAX
#endif

#if !defined(PATH_MAX)
#define PATH_MAX 1024
#endif

#define RA_MIN(a,b) (((a)<(b))?(a):(b))
#define RA_MAX(a,b) (((a)>(b))?(a):(b))

#define REMEM_VERSION "Savant relevance engine, ver. 2.12, 2/16/04\nCopyright 2004 MIT Media Lab and Bradley Rhodes"
#define REMEM_VERSION_NUMBER "2.12, 2/16/04"

#endif


