/* Growbuffer ADT.  Implements a char string that automatically
   reallocates memory as necessary. */

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

#include "gbuf.h"
#include "savant.h"

/* Return the minimum of n and strlen(n).  This won't read beyond n characters of string s,
   so if s isn't null terminated it won't matter so long as it's malloc'ed up to n. */
size_t strnlen (char *s, size_t n)
{
  int i;
  if (s == NULL) return(0);
  for (i=0; ((i<n) && (s[i] != '\0')); i++);
  return(i);
}

/* Create a new Growbuffer, point gbuf to it. */
void init_GBuffer(GBuffer *gbuf)
{
  gbuf->size = GBUFF_INIT_SIZE;
  gbuf->value = (char *)malloc(GBUFF_INIT_SIZE * sizeof(char));
  gbuf->value[0] = '\0';
  gbuf->tail = 0;
} 

/* Destructor for a growbuffer */
void free_GBuffer(GBuffer *gbuf)
{
  if (gbuf != NULL) {
    free(gbuf->value);
  }
  return;
}

/* return copy of char * part of GrowBuffer */
char *strcast_GBuffer(GBuffer *src) {
  char *retvalue;
  
  if (src == NULL) {
    return(NULL);
  }
  else {
    src->value[src->tail] = '\0';     /* This should always be true anyway, I'm just sanity checking */
    retvalue = src->value;            /* A pointer to it, not the original.  Potentially dangerous.  */
/*  retvalue = strdup(src->value); */
    return(retvalue);
  }
}

/* Like strncat, but for growbuffers */
void strncat_GBuffer(GBuffer *dest, char *src, size_t n)
{
  int i;
  if (n > strnlen(src,n)) n=strnlen(src,n);
  if ((dest->tail + n + 1) > dest->size) {
    dest->size = 2 * (RA_MAX(dest->size, (n + dest->tail + 1)));
    dest->value = realloc(dest->value, dest->size);
  }
  strncat(dest->value, src, n);
  dest->tail += n;
  dest->value[dest->tail] = '\0';
  return;
}

/* Like strncpy, but for GBuffers, except that it will always null terminate. */
void strncpy_GBuffer(GBuffer *dest, char *src, size_t n)
{
  int i;
  if (n > strnlen(src,n)) n=strnlen(src,n);
  if (n > dest->size) {
    dest->size = 2 * (RA_MAX(dest->size, n));
    dest->value = realloc(dest->value, dest->size);
  }
  strncpy(dest->value, src, n);
  dest->tail = n;
  dest->value[dest->tail] = '\0';
  return;
}

/* Chop out the first n characters of a growbuffer.  So
   strnchop_GBuffer(gbuf, 4) changes gbuf from "Boogie Woogie" to 
   "ie Woogie". */
void strnchop_GBuffer(GBuffer *dest, size_t n)
{
  char *temp;
  if (n > dest->tail) {            /* Cutting the whole thing */
    strncpy_GBuffer(dest, "", 0);
  }
  else {
    temp = (char *)malloc(sizeof(char) * (dest->tail - n + 1));
    strncpy(temp, (char *)(strcast_GBuffer(dest) + n), (dest->tail - n));
    strncpy_GBuffer(dest, temp, (dest->tail - n));
    free(temp);
  }
}



