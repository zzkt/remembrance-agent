#ifndef _PARSEDOC_H_
#define _PARSEDOC_H_

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
#include "savant.h"

#if !defined(MAX_STRING)
#define MAX_STRING 1024
#endif

#if !defined(PATH_MAX) || (PATH_MAX < 1024)
#define PATHLEN 1023
#else
#define PATHLEN PATH_MAX
#endif

#define DOCLOC_FNAME "doclocs"
#define DLOFFS_FNAME "doclocs_offs"
#define TITLES_FNAME "titles"
#define TOFFS_FNAME "titles_offs"

#define MAX_DOC_LENGTH 10000000  /* Max length of a doc before we assume we goofed */

#include "conftemplates.h"
#include "gbuf.h"

extern int SavantVerbose, SavantDebug, SavantFollowSymlinks;

typedef struct _List_of_Filenames {
  char *filename;
  short is_url_p;   /* = 1 if this is a URL, 0 if it's a file */
  struct _List_of_Filenames *next;
} List_of_Filenames;


/* Doc_Info contains all the info you might want about a document or query.  
   When used on a query, docnum, doc_start, doc_end, and filename are meaningless
   and are ignored */
typedef struct _Doc_Info {
  DB_UINT docnum;               /* The docnum for this document.
                                   Numbers are assigned serially 
                                   starting at 0. */
  unsigned int doc_start;       /* The offset from the start of the 
				   file for the start of this doc 
				   (0 = from start) */
  unsigned int doc_end;         /* The offset from the start of the file 
				   for the end of this doc */
  char *filename;               /* The expanded filename for this particular 
				   file */
  GBuffer *documentText;        /* The document itself */
  GBuffer *fields[MAX_NUMBER_FIELDS];  /* The text in each field, in order 
					  specified in the template */
  void *parsedfields[MAX_NUMBER_FIELDS];  /* Parsed info for each field 
					     (from calling the parser for 
					     this field) */
} Doc_Info;

int is_bin_file_p(char *);

int string_present_p(char *string, char **strings);

void get_filenames(char *sources[], char *excludees[]);

List_of_Filenames *get_files_from_directory(char *sourcename, 
                                            char **excludees);


void get_and_append_filenames(char *sources[], char *excludees[], List_of_Filenames **lof);

General_Template *recognize_file (FILE *file,
                                  List_of_General_Templates *current_template);

General_Template *recognize_query(GBuffer query, List_of_General_Templates *current_template);

/* Find the next doc in a file.  documentText is the text from this
   current doc.  target_template is the template for this particular
   file's type.  file_ptr is the pointer to the file in question.  start
   and end will be filled with the start and end offsets in documentText */

void find_next_doc(GBuffer *documentText,     /* The text read in from the document.  Modified to eat docs each run. */
                   Doc_Info *document,           /* we fill in this document into document->documentText */
                   General_Template *target_template, /* Which template is this file? */
                   FILE *file_ptr);                   /* pointer to the doc file */

void find_fields(Doc_Info *docInfo, General_Template *target_template);

void parse_fields(Doc_Info *docInfo, General_Template *target_template);

DB_INT docloc_templateno (Retrieval_Database_Info *rdi, DB_UINT docnum);

void write_doc_info(Doc_Info *docInfo, General_Template *target_template, char *db_dir, int final_write_p);

void index_store_fields(Doc_Info *docInfo, General_Template *target_template, char *dbdir, int final_write_p);

void print_doc_info (Doc_Info *docInfo, General_Template *template);

void add_potential_top_contributor (Top_Contributors *topList, float additional, char *printname);

void merge_top_contributors (Top_Contributors *tc1, Top_Contributors *tc2);

void add_additional_to_doc_sim (Doc_Sim *thisSim, float additional, char *printname);

void add_maximum_to_doc_sim (Doc_Sim *thisSim, float newsim, char *printname);

int top_contributors_cmp_qsort(const void *tl1, const void *tl2);

#endif
