/* indexers-date.c has all the (hopefully reusable) indexing functions
   for dates pointed to by entries in the template structure. */

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

#include "parsedoc.h"
#include "plugins-date.h"
#include "plugins.h"
#include "conftemplates.h"



/* Write wordvec file to disk, in directory dbdir with filename
   fnames.  If final_write_p == 1, close the files afterwards.  Free
   dvtree as you go.  write_text_index_file_and_free writes
   WORDVEC-OFFS and WORDVEC files, while index_store writes the DOCLEN
   file directly.
   
   File format:
      Format for WORDVEC_FILE: doc/weight pairs for the wordcode specified
      in the wordvec offset file.  No wordcode is listed here since it's 
      redundant.  Docnums are listed in reverse docnum order, 'cause that's
      easier to deal with in the indexing.

      Weight is represented as DB_UINT instead of DB_USHORT because it
      allows us to more easilly load it all in one read, even when
      we're reading things in reverse network byte order.  I think
      this will make the read faster, at the cost of wasting a couple
      bytes per weight.  Not sure if this is a valid assumption, and
      it should probably be tested on various platforms at some point.

      (DB_UINT)     (DB_UINT)     (DB_UINT)     (DB_UINT)   ...(DB_UINT)      (DB_UINT)
      DOCNUM-1,     WEIGHT-1,     DOCNUM-2,     WEIGHT-2,   ...DOCNUM-N1,     WEIGHT-N1,
      DOCNUM-2,     WEIGHT-1,     DOCNUM-2,     WEIGHT-2,   ...DOCNUM-N2,     WEIGHT-N2
      ...


      Format for WVOFF_FILE: wordcode + offset into wordvec file.  Offset is #
      of bytes from start that the first docnum for this wordvec appears.

      (width*DB_UINT)   (DB_UINT)
      WORDCODE-1,       OFFSET-1,
      WORDCODE-2,       OFFSET-2,
      ...

      Format for DOCLEN_FILE: docnum followed by fieldtype and
      document length (number words), in docnum order followed by
      fieldtype order.  Note that there might be missing document
      numbers or fields, since this is only for docs and fields with
      text-type.  

      (DB_UINT)    (DB_UINT)       (DB_UINT)       
      DOCNUM-1,    FIELDTYPE-1,    LENGTH-OF-DOC-1-FIELD-1,
      DOCNUM-1,    FIELDTYPE-2,    LENGTH-OF-DOC-1-FIELD-2,
      DOCNUM-2,    FIELDTYPE-1,    LENGTH-OF-DOC-2-FIELD-1,
      ...

  */

void write_date_index_file_and_free (Date_Tree *datetree, 
                                     char *dbdir,
                                     char *date_fname,
                                     int final_write_p) {
  static FILE *DATE_FILE = NULL;
  Date_Doc_List *doc, *doc_next;

  if (DATE_FILE == NULL) 
    DATE_FILE = open_or_die(dbdir, date_fname, "w");

  if (datetree != NULL) {    
    /* Write to the left */
    write_date_index_file_and_free (datetree->left, dbdir, date_fname, 0);
    
    /* Write me */

    doc = datetree->documents;
    while (doc != NULL) {
      fwrite_big(&(datetree->date), sizeof(DB_UINT), 1, DATE_FILE);
      fwrite_big(&(doc->docnum), sizeof(DB_UINT), 1, DATE_FILE);
      printf("Writing to %s:  %d  %d\n", date_fname, datetree->date, doc->docnum);
      doc_next = doc->next;
      free(doc);
      doc = doc_next;
    }
    
    /* Write to the right and fight fight fight */
    write_date_index_file_and_free (datetree->right, dbdir, date_fname, 0);  

    free(datetree);
  }

  if (final_write_p) {
    if (DATE_FILE != NULL) fclose(DATE_FILE);
    DATE_FILE = NULL;
  }
}



/* write checkpoint files.  These are written as the filename with a .## 
   after it, where ## is the number of this checkpoint.  */
void checkpoint_date_index_file_and_free (Date_Tree *datetree, 
					  int checkpoint_number, char *dbdir) {
  char date_fname[100];

  sprintf(date_fname, "%s.%.3d", DATES_FNAME, checkpoint_number);

  /* Write it out.  Remember to always close it afterwards, 'cause you can't
     have more than one checkpoint dir open at a time the way we're doing it
     now.  (the "1" as the last arg closes it) */
  write_date_index_file_and_free (datetree, dbdir, 
 				  date_fname, 1);
}


/* AUGH  this one is bad */
/* this must be set up so it writes <date> <docnum> <date> <docnum> all the way down... */
/* merge_text_index_file: Take a series of checkpointed wordvec and 
   wordvec-offset files, and merge them into one file (doing a merge sort).
*/
void merge_date_index_file (int checkpoints_written, char *dbdir) {
  DB_UINT *dates;
  DB_UINT docnum;
  DB_UINT min;
  FILE **IN_FILES;
  FILE *OUT_FILE;
  char date_fname[PATH_MAX];
  int i, minindex;
  int all_files_empty = 0;


  dates = (DB_UINT *)malloc(sizeof(DB_UINT) * checkpoints_written);

  IN_FILES = (FILE **) malloc(sizeof(FILE *) * checkpoints_written);
  OUT_FILE = open_or_die(dbdir, DATES_FNAME, "w");

  /* open the files and read in the first date from each */
  for (i = 0; i < checkpoints_written; i++) {
    sprintf(date_fname, "%s.%.3d", DATES_FNAME, i);

    IN_FILES[i] = open_or_die(dbdir, date_fname, "r");
    /* dates[i] = (DB_UINT *)malloc(sizeof(DB_UINT)); */
    fread_big(&(dates[i]), sizeof(DB_UINT), 1, IN_FILES[i]);
  }


  while (1) {
    /* find the minimum date */
    min = -1;
    minindex = 0;
    while ((min == -1) && (all_files_empty == 0)) {
      min = dates[minindex];
      minindex++;
      if (minindex == checkpoints_written)
	all_files_empty = 1;
    }
    minindex--;

    if (all_files_empty = 1)
      break;

    for (i = minindex; i < checkpoints_written; i++) {
      if ((dates[i] != -1) && (dates[i] < min)) {
	min = dates[i];
	minindex = i;
      }
    }
    
    fread_big(&(docnum), sizeof(DB_UINT), 1, IN_FILES[minindex]);
    fwrite_big(&(dates[minindex]), sizeof(DB_UINT), 1, OUT_FILE);    
    fwrite_big(&(docnum), sizeof(DB_UINT), 1, OUT_FILE);
    printf("Writing to temp file %d:  %d  %d\n", minindex, dates[minindex], docnum);
    fread_big(&(dates[minindex]), sizeof(DB_UINT), 1, IN_FILES[minindex]);
    if (feof(IN_FILES[minindex])) {
      dates[minindex] = -1;
      fclose(IN_FILES[minindex]);
      /* delete the temp file here! */
    }
  }
  fclose(OUT_FILE);
}

/* index_store: take whatever type is returned by the parser and
   stores it in the appropriate structure, for later writing to disk.
   It can also write some info to disk immediately if it needs to
   (e.g. for checkpointing so you don't run out of RAM).  Store stuff
   statically so it can be used in subsequent calls.  If last_write_p == 1, 
   make sure everything gets written to disk. 

   Assumes we get documents one at a time in docnum order. */

void index_store_date (void *parsedata,   /* Type Date_Document_Field * */
                        char *dbdir,       /* directory of the database we're 
					      writing to */
                        int last_write_p)  /* last_write_p == 1 if we should 
					      finalize it to disk now */
{
  static Date_Tree *datetree = NULL;       /* The Date Tree being added to */
  static int checkpoints_written = 0;  /* Number of checkpoint directories 
					  written so far */
  static int mem_used_by_datetree = 0;   /* Memory used by dvtree.  If
                                          this gets too big we write
                                          to disk and merge later */
  static int we_are_done = 0;          /* Because we might be called to 
					  finalize our last write several
					  times (once per template that uses
					  this method in fact), we use this 
					  to insure we only do a merge and 
					  write once at the end. */
  Date_Tree *tree = NULL;
  Date_Document_Field *pd;
  DB_UINT docnum;

  pd = (Date_Document_Field *)parsedata;

  if (pd != NULL) 
    tree = pd->tree;
  else 
    tree = NULL;

  if (!we_are_done) {  
    mem_used_by_datetree += 
      add_document_to_datetree(&datetree, tree);

    if (last_write_p) {        /* Let's blow this thing and go home */
      if (checkpoints_written > 0) {
	if (datetree != NULL) {  /* write what's remaining */
	  checkpoint_date_index_file_and_free (datetree, checkpoints_written++, 
					       dbdir);
	}
	merge_date_index_file (checkpoints_written, dbdir); 
	datetree = NULL;      /* it just got freed */
      }
      else {
	write_date_index_file_and_free (datetree, dbdir, DATES_FNAME, 1);
	datetree = NULL;
      }
      we_are_done = 1;
    }

    if (!last_write_p && 
	datetree != NULL &&
	(mem_used_by_datetree > MAX_DOC_MEMORY)) {
      checkpoint_date_index_file_and_free (datetree, checkpoints_written++, 
					   dbdir);
      mem_used_by_datetree = 0;  /* it got freed */
      datetree = NULL;
    }
  }
}
