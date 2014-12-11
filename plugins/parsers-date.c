/* parsers-date.c has all the (hopefully reusable) parser and deparser
   functions for dates pointed to by entries in the template
   structure. */

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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "parsedate.h"
#include "plugins-date.h"
#include "conftemplates.h"


/* parse_date:  fielddata contains a string representation of a date.
     this string is passed to a date parser which returns a unix timestamp
     value for that date.  a Date_Document_Field is then returned which
     contains this timestamp and and document number associated with it.
*/
void *parse_date (char *fielddata, void *self, DB_INT docnum) {
  /*  We are using DB_UINT for timestamps because maxint is 2^32 = 4294967296.
      Currently, our timestamp is 918681392 (from doing "date +%s"), so we're
      fine till ~2106.  so:

      WARNING!!!!  This software is not Year 2106 compliant.  The crusty old
      C programmers in 2106 might want to change that :)

      EXTRA OTHER WARNING!!!  this code assumes that if fielddata contains
      multiple dates, they are separated by a carriage return.  the format
      must be as follows:

      <date><cr>
      <date><cr>
      ...
      <date><cr>
  */
  Date_Tree *tree = NULL;
  Date_Document_Field *return_value = NULL;
  int p=0, q=0, cr=0, s=0;
  int r;
  GBuffer g;
  struct parsedate *pd;

  init_GBuffer(&g); 

  /* figure out how many dates are in the fielddata */
  while(fielddata[s] != '\0') {
    if (fielddata[s] == '\n')
      cr++;
    s++;
  }
  cr++;

  return_value = (Date_Document_Field *)malloc(sizeof(Date_Document_Field));
  return_value->docnum = (DB_UINT)docnum;
  
  /* printf("Creating document[%d]\n", docnum); */

  while(1) {
    if (fielddata[q] == '\n' || fielddata[q] == '\0') {
      pd = parsedate(strcast_GBuffer(&g));
      strnchop_GBuffer(&g, strlen(strcast_GBuffer(&g)));
      datetree_add_word(&tree, 1, (DB_UINT)pd->unixtime, docnum);
      /* printf("  Date[%d]: %d\n", r, (DB_UINT)pd->unixtime); */
      p++;

      if (fielddata[q] == '\0')
	break;
    }
    strncat_GBuffer(&g, &fielddata[q], 1);
    q++;    
  }      
  
  free_GBuffer(&g);
  return_value->tree = tree;
  return return_value;
}

/* deparse_text: looks at a parsed date field (a Date_Document_Field
   contained in fielddata) and returns a GBuffer with some printable
   text describing the dates.

   i think this code has some memory management issues regarding the
   freeing of the GBuffer s.  not sure how to solve it though.

*/
GBuffer *deparse_date (void *fielddata, void *self) {
  GBuffer *s;
  char b[100];
  int n=0;
  
  init_GBuffer(s);
  
  deparse_helper(((Date_Document_Field *)fielddata)->tree, s);

  if (s == NULL) {
    return(NULL);
  }
  return(s);
}

void deparse_helper(Date_Tree *tree, GBuffer *buf) {
  char temp[100];

  if (tree->left != NULL)
    deparse_helper(tree->left, buf);

  sprintf(temp, "%d ", tree->date);
  strncat_GBuffer(buf, temp, strlen(temp));

  if (tree->right != NULL)
    deparse_helper(tree->right, buf);
}

/* Return next date in a Date_Document_Field.  If reset_p != 0, start
   over.  Right now, we're just re-searching the tree every time to
   find the next date.  Not a big deal since we're only using this for
   queries, and we expect those to be relatively small.

   We're bundling the date, into the Date_Word_Info.  This will get
   freed by update_sims_word. */
void *nextword_date (void *fielddata, int reset_p) {
  static Date_Tree *stateptr=NULL;   /* ptr to the previous word, so we can find the next. */
  Date_Document_Field *fd;
  Date_Word_Info *dwi;
  
  if (fielddata == NULL) return(NULL);
  if (stateptr == NULL) reset_p = 1;
 
  fd = (Date_Document_Field *)fielddata;
  if (reset_p)
    stateptr = fd->tree;
  stateptr = date_greater_than(&(stateptr->date), fd->tree, reset_p);
  if (stateptr == NULL)
    return(NULL);

  dwi = (Date_Word_Info *)malloc(sizeof(Date_Word_Info));   /* Gets freed by update_sims_word */
  dwi->date = stateptr->date;
  return(dwi);
}

/* Return node with the minimum date that's still > date.  If 
   return_smallest == 1, just return the minimum wordcode */
Date_Tree *date_greater_than(DB_UINT *date, Date_Tree *root, int return_smallest) {
  Date_Tree *potential_answer = NULL;
  int comparison;

  if (root == NULL) return(NULL);

  comparison = (return_smallest || (root->date < *date)) ? -1 :
	   ((root->date > *date) ? 1 : 0);

  if (comparison > 0) {     /* our node > date, so look for smaller */
    potential_answer = date_greater_than(date, root->left, return_smallest);
    return((potential_answer == NULL) ? root : potential_answer);
  }
  else {    /* our node <= wordcode, look for bigger */
    return(date_greater_than(date, root->right, return_smallest));
  }
}

void free_parsed_date (void *parseddata) {
  Date_Document_Field *df;

  if (parseddata == NULL) return;

  df = (Date_Document_Field *)parseddata;
  /*free(df->dates);*/
  free(df);
  return;
}

/* Add a date to a Date_Tree

   NOTE: This assumes that we'll be adding a document at a time, and adding all dates
   for a particular document at once.  If this isn't true, we'll need to actually scan
   the whole doclist for a docvec node.

   Return the number of bytes malloc'ed this call.  (Jees, doing our
   own memory management and everything.  Why not just get a real
   language and be done with it?)

*/
int datetree_add_word(Date_Tree **treeptr,
                    DB_UINT date,
                    DB_UINT docnum) {
  Date_Doc_List *doclist;
  Date_Tree *tree = *treeptr;
  int cmp;
  int i;

  if (tree==NULL) {
    /* create new node */
    if ((tree = (Date_Tree *)malloc(sizeof(Date_Tree))) == NULL)
      SavantError(ENOMEM, "Unable to malloc *tree in parsers-date.c");

    if ((tree->documents = (Date_Doc_List *)malloc(sizeof(Date_Doc_List))) == NULL)
      SavantError(ENOMEM, "Unable to malloc document list in parsers-date.c");

    tree->date = date;
    tree->left = NULL;
    tree->right = NULL;
    tree->documents->next = NULL;
    tree->documents->docnum = docnum;
    *treeptr = tree;
    return(sizeof(Date_Tree) + sizeof(Date_Doc_List));
  }
  
  /* TODO: fix this comment */
  /* If we've already got a match for this wordcode AND this doc, just
     increment the weight.  If we've got the wordcode but not the doc,
     prepend this document to the front.

     NOTE: This assumes that we'll be adding a document at a time, and adding
     all words for a particular document at once.  If this isn't true, we'll 
     need to actually scan the whole doclist for a docvec node.  */

  /* cmp = wordcode_cmp(code, tree->wordcode); */

  if(date == tree->date) {                   /* then we are adding a date that is already in the tree... */
    /* hmm... what to do here? */
    /* Got the word but not doc, add the doc to list 
       This adds in reverse-doc order, 'cause it's easier. */
    if (tree->documents->docnum == docnum) { /* already have this date & doc */
      return(0);     /* Didn't alloc anything this round */
    }
    else {
      if ((doclist = (Date_Doc_List *)malloc(sizeof(Date_Doc_List))) == NULL) 
	SavantError(ENOMEM, "Unable to malloc doclist in parsers-date.c");
      doclist->docnum = docnum;
      doclist->next = tree->documents;
      tree->documents = doclist;
      return(sizeof(Date_Doc_List));
    }
  }
  else if(date < tree->date) {
    return(datetree_add_word(&(tree->left), date, docnum));
  }
  else {
    return(datetree_add_word(&(tree->right), date, docnum));
  }
}


/* add_document_to_datetree: merge the documentTree into target.
   documentTree is assumed to only contain a single document (one
   entry in the Date_Doc_List per word) -- otherwise we'd have to do more
   checking and scanning.

   NOTE: In this implementation, we're first throwing words into a
   binary tree (Date_Tree) when parsing data.  This happens in
   parse_text, and is done so we can add up dates for duplicates.
   We then merge trees in add_document_dvtree when indexing
   (index_store_text).  An alternate method would be to just collect
   documents in a linked list in parse_date, and only do the
   tree-search once instead of twice.  The downside is we'd have to
   malloc more memory, 'cause we'd have a list entry for every
   occurence of a date whether it's duplicated or not. 

   Return the number of bytes added to target this round, so we can
   keep track of memory usage.  (There really should be a better way.)  */

int add_document_to_datetree (Date_Tree **target, Date_Tree *documentTree) {
  int mem = 0;    /* memory used */

  if (documentTree == NULL) return(0);      /* nothing to do */

  /* Add everything to the left, add everything to the right */
  if (documentTree->left != NULL) {
    mem += add_document_to_datetree(target, documentTree->left);
  }
  if (documentTree->right != NULL) {
    mem += add_document_to_datetree(target, documentTree->right);
  }

  /* I'm now a leaf node, so add & free myself */
  mem += datetree_add_word(target, 
			   documentTree->date,
			   documentTree->documents->docnum);

/* We used to free things here, but now we're freeing in a top-level call in main. */
/*free(documentTree->documents);
  free(documentTree);
*/
  return(mem);
}  


