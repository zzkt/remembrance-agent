#ifndef _TEXT_H_
#define _TEXT_H_
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

/*** Wordcodes are unsigned int arrays of width WORD_ENCODE_WIDTH ****/
typedef DB_UINT * Wordcode;

#define WORD_ENCODE_WIDTH 3  /* number bytes per word in a wordvec */

/* DO NOT CHANGE THIS FROM 6.  It's not known if the algorithm
   actually -works- if this isn't 6, and there may be places in the
   code that use some knowledge of the fact that this is 6 without
   actually referring to CHARACTER_ENCODE_WIDTH.  Eit.  */
#define CHARACTER_ENCODE_WIDTH 6  /* number of bits per character in a packed wordvec */

/* maximum number of unique non-stop words (after filtering) that are looked 
   at in indexing and retrieval in a given text field. This is counted from the 
   beginning of the document, and is meant to get around the fact that super long
   documents both seem to come up too often (the weighting schemes aren't normalizing
   against doc length well enough) and if your query is relevant to something in the middle
   of the document that doesn't help you much anyway. 

   If zero, there's no max.
*/
#define MAX_NUMBER_WORDS_PER_INDEXED_FIELD 500 

/* A list of documents, with weights for each.  Used by DV_Tree structure, below. */
typedef struct _Doc_List {
  DB_UINT docnum;
  DB_UINT weight;
  struct _Doc_List *next;
} Doc_List;

/* tree rep of a multi-document Vector, with wordcodes as sorting keys */
typedef struct _DV_Tree {
  unsigned int wordcode[WORD_ENCODE_WIDTH];
  char printword[PRINTWORD_LENGTH + 1];
  Doc_List *documents;
  struct _DV_Tree *left;
  struct _DV_Tree *right;
} DV_Tree;

/* rep for a single text document.  This consists of a docvec containing the
   words plus agregate info like doc length */
typedef struct _Text_Document_Field {
  DB_UINT docnum;  /* Docnum for this particular document */
  DB_UINT length;  /* Number of unique words in this field of this doc */
  DB_UINT fieldtypenum;  /* Field type number (same as used in the
                              text-word's 6-bit type field */
  DV_Tree *tree;   /* The words in this field of this document */
} Text_Document_Field;

/* Info on a specific word in a query (passed from nextword_text to update_sims_word_text) */
typedef struct _Text_Word_Info {
  Wordcode word;
  char printword[PRINTWORD_LENGTH];    /* printable string for this word (used for user feedback of a query) */
  int weight;          /* term frequency */
  int length;          /* Number unique terms in this document, used for normalization */
} Text_Word_Info;

/* This contains all the info the text algorithm might need */
typedef struct _Text_Doc_Info {
  int number_documents;
  DB_UINT *document_lengths;    /* Sparse array of document length
                                   document_length[(DOCNUM * MAX_NUMBER_FIELDS) + FIELDNUM] */
  DB_UINT *avg_document_length; /* Sparse array of average document length
                                   avg_document_length[FIELDNUM] */
} Text_Doc_Info;


#define MAX_DOC_MEMORY 10000000    /* number bytes to use for document vectors
                                      before checkpointing to disk */

#define WORDVEC_FNAME "wordvecs"
#define WVOFF_FNAME "wvoffs"
#define DOCLEN_FNAME "doclens"


/* Some other prototypes, from parsers-text.c */

int wordcode_cpy(Wordcode dest,	Wordcode source);
int wordcode_cmp(Wordcode code1, Wordcode code2);

int add_document_to_dvtree(DV_Tree **target, DV_Tree *documentTree);

void encode_text_word(unsigned char *s, 
                      unsigned int * code,
                      unsigned int field_type);

#endif
