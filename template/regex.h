#ifndef _REGEX_H_
#define _REGEX_H_

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
#define MAX_NUMBER_FILTERS 512

#include <sys/types.h>
#include <string.h>
#include <gbuf.h>
#include <pcre.h>

/*
  regex_filter:

    char **regexps  -  array of regex strings, each of which will
                       be used as a filter.

    char *target    -  string from which all regex matches will be
                       be removed.

*/
void regex_filter(char **regexps, GBuffer *target);


/*
  regex_find:

  char *regexp      -  the first occurance of this regex will be
                       identified...

  char *target      -  ...in this string.

  int length_to_search - distance into target to search

  int start         -  return value of start of regex match

  int end           -  return value of end of regex match
*/
void regex_find(char *regexp, char *target, int length_to_search, int *start, int *end);

#endif

