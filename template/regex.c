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
#include <regex.h>
#include <errno.h>

/* takes in a regular expression regexp, a string target, and an integer length_to_search.

   scans the first length_to_search characters of target, and attempts to match regexp to
   them.  if regexp is found, sets start and end equal to the first character of the match
   and the last character of the match, respectively.
*/

void regex_find(char *regexp, char *target, int length_to_search, int *start, int *end)
{
  const char *errptr;
  char errortext[256];
  int vector[256], error_offset;
  pcre *pattern;

  if (regexp == NULL) {
    *start = -1;
    *end = -1;
    return;
  }

/* For pcre-1.09
  if (NULL == (pattern = pcre_compile(regexp, PCRE_DOTALL, &errptr, &error_offset))) {
*/
  if (NULL == (pattern = pcre_compile(regexp, PCRE_DOTALL, &errptr, &error_offset, NULL))) {
    sprintf(errortext, "regex_find: pcre error %s at location %d", errptr, error_offset);
    SavantError(ENOEXEC, errortext);
  }

/* For pcre-1.09
  pcre_exec(pattern, NULL, target, length_to_search, 0, vector, 256);
*/
  pcre_exec(pattern, NULL, target, length_to_search, 0, 0, vector, 256);
  *start = vector[0];
  *end = vector[1];
  free(pattern);
  return;
}


/*  at some point, code should be introduced to utilize the pcre_study command */
/*  because it will give us a little speed bonus... */

/* takes in an array of regexps and a GBuffer target.

   scans through target and removes every occurrance of every regexp
   from target.
*/
void regex_filter(char **regexps, GBuffer *target) {
  const char *errptr;
  char *temp_string, *ptr, errortext[512];
  int vector[256], error_offset, i=0, groups_matched;
  pcre *pattern;

  if (regexps == NULL)
    return;

  if ((temp_string = (char *)malloc(sizeof(char) * (target->size + 1))) == NULL) {
    SavantError(ENOMEM, "Unable to malloc temp_string in regex.c\n");
  }

  for(i=0; ((i<MAX_NUMBER_FILTERS) && (regexps[i] != NULL)); i++) {

/* For pcre v. 1.09
    if (NULL == (pattern = pcre_compile(regexps[i], PCRE_DOTALL, &errptr, &error_offset))) {
*/
    if (NULL == (pattern = pcre_compile(regexps[i], PCRE_DOTALL, &errptr, &error_offset, NULL))) {
      sprintf(errortext, "regex_filter: pcre_compile error %s at location %d\n", errptr, error_offset);
      SavantError(ENOEXEC, errortext);
    }

    temp_string[0] = '\0';
    ptr = strcast_GBuffer(target);
    vector[0] = vector[1] = 0;

    while(vector[0] >= 0) {
/* For pcre version 1.09
      groups_matched = pcre_exec(pattern, NULL, ptr, strlen(ptr), 0, vector, 256);
*/
      groups_matched = pcre_exec(pattern, NULL, ptr, strlen(ptr), 0, 0, vector, 256);

      if (vector[0] >= 0) {
	strncat(temp_string, ptr, vector[0]);
	ptr = ptr + vector[1] * sizeof(char);
      }
    }
    strcat(temp_string, ptr);
    strncpy_GBuffer(target, temp_string, strlen(temp_string));
    pcre_free(pattern); 
  }
  
  free(temp_string);
 
  return;
}










