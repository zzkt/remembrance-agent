/* savutil.h - types and externs for libsavutil.a */
#ifndef _SAVUTIL_H_
#define _SAVUTIL_H_

/*
All code included in versions up to and including 2.09:
   Copyright (C) 2001 Massachusetts Institute of Technology.

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
#include <savant.h>

extern int SavantVerbose, SavantDebug;


/*** Misc ***/

/*** Extern declarations ***/

/*from big.c */
extern size_t fread_big(void *, size_t, size_t, FILE *);
extern size_t fwrite_big(void *, size_t, size_t, FILE *);
size_t fcpy_big(FILE *dest_stream, size_t size, size_t num, FILE *stream);
long ftell_end (FILE *stream);


/* from opendie.c */
extern FILE *open_or_die(char *, char *, char *);

#endif /* #ifndef _SAVUTIL_H_ */
