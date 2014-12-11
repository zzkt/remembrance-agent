#ifndef _BUFFER_H_
#define _BUFFER_H_


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

#include <stdio.h>
#define GBUFF_MAX_SIZE 16777216
#define GBUFF_INIT_SIZE 256          /* num bytes in a new growbuffer */


typedef struct
{ 
  char *value;
  int tail;    /* location of null-termination, so this == strlen(value) */
  int size;
} GBuffer;

/* after you definte a GrowBuffer, you should
   init_GBuffer it, to set up the vars initially. */
void init_GBuffer(GBuffer *gbuf);

/* after you init_GBuffer a GrowBuffer, don't
   forget to free it. */
void free_GBuffer(GBuffer *gbuf);

/* return char * part of GrowBuffer 
   (don't modify this string -- it might screw up our 
   internal idea of how big this GB is) */
char *strcast_GBuffer(GBuffer *src);

/*  writes n chars of src into dest */
void strncat_GBuffer(GBuffer *dest, char *src, size_t n);

/*  copy first n chars of src into the dest GrowBuffer */
void strncpy_GBuffer(GBuffer *dest, char *src, size_t n);

/* Chop out the first n characters of a growbuffer.  So
   strnchop_GBuffer(gbuf, 4) changes gbuf from "Boogie Woogie to 
   "ie Woogie". */
void strnchop_GBuffer(GBuffer *dest, size_t n);

#endif
