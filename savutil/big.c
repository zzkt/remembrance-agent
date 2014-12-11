/* big.c: read and write to file in bigendian, regardless of the
native format. */

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
USA.  */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <savant.h>
#include <savutil.h>

size_t
fwrite_big(void *ptr, size_t size, size_t num, FILE *stream)
{
#if (WORDS_BIGENDIAN == 0)
  unsigned char *big = malloc(size*num);
  int i, j;
  size_t out;

  if (big == NULL)
    SavantError(ENOMEM, "Unable to malloc big in big.c");

  for(i=0; i<num; i++) {
    for(j=0; j<size; j++) {
      big[size*i + (size-1-j)] = ((unsigned char *) ptr)[size*i + j];
    }
  }
  
  out = fwrite((void *)big, size, num, stream);
  free(big);
  return(out);

#else
  return(fwrite(ptr, size, num, stream));

#endif
}

size_t
fread_big(void *ptr, size_t size, size_t num, FILE *stream)
{
#if (WORDS_BIGENDIAN == 0)
  unsigned char *big = malloc(size*num);
  int i, j;
  size_t out;

  if (big == NULL)
    SavantError(ENOMEM, "Unable to malloc big in big.c");

/*
  if (SavantDebug) {
    printf("fread_big: reading %d counts of size %d from stream #%d... ",
           num, size, stream);
  }
*/

  out = fread((void *)big, size, num, stream);

/*
  if (SavantDebug) {
    printf("done... ");
  }
*/

  for(i=0; i<num; i++) {
    for(j=0; j<size; j++) {
      ((unsigned char *)ptr)[size*i + (size-1-j)] = big[size*i + j];
    }
  }
  
  free(big);

/*
  if (SavantDebug) {
    printf("and processed.\n ");
  }
*/

  return(out);

#else
  return(fread(ptr, size, num, stream));

#endif
}

/* read num elements of size size from stream and write them to dest_stream.
   Here endianness doesn't matter 'cause it should be the same for both files.
*/
size_t fcpy_big(FILE *dest_stream, size_t size, size_t num, FILE *stream) {
  void *buf;
  size_t numread;
  size_t numwritten;
  buf = (void *)malloc(size * num);
  numread = fread(buf, size, num, stream);
  numwritten = fwrite(buf, size, numread, dest_stream);
  free(buf);
  return(numwritten);
}

/* give an ftell of the end of the document (why isn't there such a 
   command?) */
long ftell_end (FILE *stream) {
  long curr_pos, end_pos;
  curr_pos = ftell(stream);
  fseek(stream, 0, SEEK_END);
  end_pos = ftell(stream);
  fseek(stream, curr_pos, SEEK_SET);
  return(end_pos);
}

/* Do an fread without changing the file pointer */
size_t fread_peek_big(void *ptr, size_t size, size_t num, FILE *stream)
{
  long curr_pos;
  curr_pos = ftell(stream);
  fread_big(ptr, size, num, stream);
  fseek(stream, curr_pos, SEEK_SET);
}
