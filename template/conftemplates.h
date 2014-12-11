#ifndef _CONFTEMPLATES_H_
#define _CONFTEMPLATES_H_

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
#include <savutil.h>
#include <errno.h>
#include "gbuf.h"
#include "hash.h"

extern int SavantVerbose, SavantDebug;

#define RECOGNIZE_LIMIT 500   /* num characters to look for recognition in */

#ifndef EOVERFLOW
#define EOVERFLOW E2BIG
#endif

/* config stuff */
#define MAX_NUMBER_FIELDS 24         /* Max number of distinct field, and max number fields per template */
#define MAX_FIELD_NAME_LENGTH 30     /* Number of characters max in a field printname */
#define MAX_FILTER_REGEXPS 50
#define NUMBER_CONTRIBUTORS_TRACKED 5  /* Number of contributors kept track of for user feedback */
#define PRINTWORD_LENGTH 15  /* number of characters to use for a printable version of a "word" in feedback */

/* Title default information: what should we use for a field entry in a title
   if the field is blank? */
enum Title_Defaults_Type {BLANK_TITLE, FILENAME_TITLE, OWNER_TITLE,
			  MODTIME_TITLE};

/* Contains the elements ("words") that contributed the most to a single contributor */
typedef struct _Top_Contributors {
  float sim;                           /* Similarity this contributed */
  char printword[PRINTWORD_LENGTH];    /* What is this word anyway? */
} Top_Contributors;

/* Mentioned in Field typedef -- used in retrieval */
typedef struct _Doc_Sim {
  DB_UINT docnum;
  float sim;           /* number between 0.0 and 1.0 */
  float sim_breakdown[MAX_NUMBER_FIELDS];
  Top_Contributors top_contributors[NUMBER_CONTRIBUTORS_TRACKED];
} Doc_Sim;

/* The docsim, plus a breakdown of how each field contributed */
typedef struct _Doc_Sim_Totals {
  Doc_Sim docsim;
  float sim_breakdown[MAX_NUMBER_FIELDS];
} Doc_Sim_Totals;


/* This is all the info about the index database that you might want to compute
   once and pass to updaters.  Generic info only (no fieldtype specific
   stuff) */
typedef struct _Retrieval_Database_Info {
  int number_documents_total;   /* Total number of documents */
  char *db_dir; /* Expanded path of index files */
} Retrieval_Database_Info;

/* All the user-preferance variables (1 = true, 0 = false) */
typedef struct {
  int source_field_width;
  int ellipses;
  int index_dotfiles;
} UserVars;

extern UserVars Config;

/* Field: info on a generic field (independent of what file type it's in) */
typedef struct Field {
  DB_UINT typenum;        /* Number used in 6-bit type field, and
			     index into Array_of_Field_Types */
  char *printname;        /* Printable name of field, e.g. "BODY" */
  enum Title_Defaults_Type titleDefault;  /* what to do in title if blank */
  /* Parser is a function that takes field data (e.g. a string of
     words) and returns a parsed, machine storable version (e.g. a
     wordvec).  The parser may also do other pre-processing, like stemming
     words and removing stop-words.  Self is this field (necessary so the
     encoder can figure out the typenum).  (it's void instead of Field
     'cause of stupid C not doing forward references right.) */
  void *(*parser)(char *fielddata, void *self, DB_UINT docnum);
  GBuffer *(*deparser)(void *parseddata, void *self); /* Function that takes the output of the parser program and
                                                      returns a human-readable version of the data */
  void (*index_store)(void *parseddata,  
                      char *filename,
                      int last_write_p); /* Function that takes whatever type is returned by the parser
                                            and stores it in the appropriate structure, for later writing 
                                            to disk.  doc_start is the file offset for the start of this particular doc,
                                            doc_end is the end of this doc. */
  void *(*nextword)(void *parseddata, 
                    int reset_p);  /* ittorator that takes type returned by parser and returns the next
                                      "word" in the series.  If reset_p != 0, restart at the beginning of the list. */
  void (*update_sims_word)(void *word, 
                           Remem_Hash_Table *all_sims, 
                           void *self,
                           Retrieval_Database_Info *rdi);
                       /* Function that takes a word (returned by
                          nextword) and its weight and updates
                          all_sims with the new similarities.  self is
                          of type Field *        */
  void (*cleanup_parsed)(void *parseddata);     /* free memory any other cleanup that has to happen.  Takes whatever was
                                                   returned by parser. */
} Field;


/* Template_Field_Info: info on a field specific to a type of file (e.g. HTML) */
typedef struct _Template_Field_Info {
  Field *field;             /* Pointer to the generic field info */
  char *id_regexp;          /* Regexp to find this field in a document */
  int id_index;             /* Index to actually get the data out of the find regexp */
  char **filter_regexp;     /* Regexp for post-filtering on the field data, before parsing */
  int bias;                 /* The bias number for this field in this kind of document */
  int filter_count;         /* Add integer for current size of filter array*/
  int title_length;         /* Number of characters to print out in the titles file for this field */
} Template_Field_Info;

/* Actions: What to do with a template (currently only "Reject" and "Accept") */
enum Action_Types {REJECT_ACTION, ACCEPT_ACTION};

/* Template types: When is this template used?  (currently only "index" and "retrieve") */
enum Template_Types {INDEX_TYPE, QUERY_TYPE};

/* Template: all the info on how to parse a particular kind of file (e.g. "HTML") */
typedef struct {
  DB_INT typenum;    /* A template number so we can id a filetype in docloc_offs, and index into All_General_Templates */
  char *printname;   /* Printable name for this kind of file (e.g. "RMAIL") */
  char *recognize;   /* Regexp to recognize this document type */
  char *delimiter;   /* Regexp to recognize the delimiter for this document */
  Template_Field_Info **fields;    /* Array pointers to the field-info used by this template, NULLs for the rest */
  enum Action_Types action;
  enum Template_Types templatetype;  /* index or retrieve? */
} General_Template;

typedef struct _List_of_General_Templates {
  General_Template *template;
  struct _List_of_General_Templates *next;
} List_of_General_Templates;

typedef struct _Array_of_Field_Types {
  int num_fields;   /* Number of fields currently populating the array */
  Field **field;    /* Array of MAX_NUMBER_FIELDS pointers to type Field */
} Array_of_Field_Types;


/* Prototypes */

/* Create a new template, with an empty Template_Field_Info array.  
   Make sure to strcpy all strings into malloced buffers */
General_Template *create_template (char *printname, char *recognize, char *delimiter, 
                                   enum Action_Types action, enum Template_Types templatetype);

/* Return the template_field_info pointer that has the included pointer */
Template_Field_Info *tfi_from_name (General_Template *template, char *fieldname);

/* Return the field pointer (from the global "All_Fields") that has the included pointer */
Field *field_from_name (char *fieldname);

/* Add a new Template_Field_Info line to a template */
void add_template_field_info (General_Template *template, Field *field, char *id_regexp, int id_index, 
                              char **filter_regexp, int bias, int title_length);

/* Free memory pointed to by template (including all sub-fields and strings in the data struct) */
void free_template (General_Template *template);

/* Create a field and add it to fieldArray, choosing the next fieldnum from the array 
   and updating the Array's num_fields */
void create_and_add_field (Array_of_Field_Types *fieldArray,
                           char *printname, 
			   enum Title_Defaults_Type titleDefault,
                           void *(*parser)(char *fielddata, void *self, DB_UINT docnum), 
                           GBuffer *(*deparser)(void *parseddata, void *self), 
                           void (*index_store)(void *parseddata,
                                               char *filename,
                                               int last_write_p),
                           void *(*nextword)(void *parseddata, 
                                             int reset_p),
                           void (*update_sims_word)(void *word, 
                                                    Remem_Hash_Table *all_sims, 
                                                    void *self,
                                                    Retrieval_Database_Info *rdi),
                           void (*cleanup_parsed)(void *parseddata));

/* Print configuration stuff (mainly for debugging) */
void print_config ();
void print_field (Field *f);
void print_template (General_Template *t);
void print_template_field_info (Template_Field_Info *tfi);
Field *get_field_from_allfields(Array_of_Field_Types *aft, char *pname);

#endif

