#ifndef _DATE_PARSERS_H_
#define _DATE_PARSERS_H_
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
#include "conftemplates.h"

#define DATES_FNAME "dates"

#define MAX_DOC_MEMORY 10000000    /* number bytes to use for document vectors
                                      before checkpointing to disk */

/* A list of documents, with weights for each.  Used by Date_Tree structure, below. */
typedef struct _Date_Doc_List {
  DB_UINT docnum;
  struct _Date_Doc_List *next;
} Date_Doc_List;

typedef struct _Date_Tree {
  DB_UINT date;
  Date_Doc_List *documents;
  struct _Date_Tree *left;
  struct _Date_Tree *right;
} Date_Tree;

typedef struct _Date_Document_Field {
  DB_UINT docnum;   /* Docnum for this particular document */
  Date_Tree *tree;
} Date_Document_Field;

typedef struct _Date_Word_Info {
  DB_UINT date;
} Date_Word_Info;

void deparse_helper(Date_Tree *tree, GBuffer *buf);
Date_Tree *date_greater_than(DB_UINT *date, Date_Tree *root, int return_smallest);

#endif
