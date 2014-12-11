/* indexers.c has all the (hopefully reusable) indexing
   functions pointed to by entries in the template structure. */

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
#include "plugins-text.h"
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

void write_text_index_file_and_free (DV_Tree *dvtree, 
                                     char *dbdir, 
                                     char *wordvec_fname,
                                     char *wvoff_fname,
                                     int final_write_p) {
  static FILE *WORDVEC_FILE = NULL;
  static FILE *WVOFF_FILE = NULL;
  DB_UINT wv_off;
  Doc_List *doc, *doc_next;

  if (WORDVEC_FILE == NULL) 
    WORDVEC_FILE = open_or_die(dbdir, wordvec_fname, "w");
  if (WVOFF_FILE == NULL) 
    WVOFF_FILE = open_or_die(dbdir, wvoff_fname, "w");

  if (dvtree != NULL) {
    
    /* Write to the left */
    write_text_index_file_and_free (dvtree->left, dbdir, wordvec_fname, 
				    wvoff_fname, 0);
    
    /* Write me */
    wv_off = (DB_UINT)ftell(WORDVEC_FILE);
    fwrite_big(dvtree->wordcode, sizeof(DB_UINT), 3, WVOFF_FILE);
    fwrite_big(&wv_off, sizeof(DB_UINT), 1, WVOFF_FILE);
    
    doc = dvtree->documents;
    while (doc != NULL) {
      fwrite_big(&(doc->docnum), sizeof(DB_UINT), 1, WORDVEC_FILE);
      fwrite_big(&(doc->weight), sizeof(DB_UINT), 1, WORDVEC_FILE);
      doc_next = doc->next;
      free(doc);
      doc = doc_next;
    }
    
    /* Write to the right and fight fight fight */
    write_text_index_file_and_free (dvtree->right, dbdir, wordvec_fname, wvoff_fname, 0);  

    free(dvtree);
  }

  if (final_write_p) {
    if (WORDVEC_FILE != NULL) fclose(WORDVEC_FILE);
    if (WVOFF_FILE != NULL) fclose(WVOFF_FILE);
    WORDVEC_FILE = NULL;
    WVOFF_FILE = NULL;
  }
}



/* write checkpoint files.  These are written as the filename with a .## 
   after it, where ## is the number of this checkpoint.  */
void checkpoint_text_index_file_and_free (DV_Tree *dvtree, 
					  int checkpoint_number, char *dbdir) {
  char wordvec_fname[100];
  char wvoff_fname[100];

  sprintf(wordvec_fname, "%s.%.3d", WORDVEC_FNAME, checkpoint_number);
  sprintf(wvoff_fname, "%s.%.3d", WVOFF_FNAME, checkpoint_number);

  /* Write it out.  Remember to always close it afterwards, 'cause you can't
     have more than one checkpoint dir open at a time the way we're doing it
     now.  (the "1" as the last arg closes it) */
  write_text_index_file_and_free (dvtree, dbdir, 
 				  wordvec_fname, wvoff_fname, 1);
}

/* merge_text_index_file: Take a series of checkpointed wordvec and 
   wordvec-offset files, and merge them into one file (doing a merge sort).
*/
void merge_text_index_file (int checkpoints_written, char *dbdir) {
  int i, minwc_index=0, starting=0, prev_minwc_written=0;
  size_t numwritten=0;
  short alldone=0, startedyet=0;
  FILE **IN_WORDVEC_FILES;
  FILE **IN_WVOFF_FILES;
  FILE *OUT_WORDVEC_FILE;
  FILE *OUT_WVOFF_FILE;
  DB_UINT in_offset, in_offset_next, out_offset;
  char wordvec_fname[PATH_MAX];
  char wvoff_fname[PATH_MAX];
  char errorstring[256];
  DB_UINT **wordcodes;
  DB_UINT *in_offsets;
  DB_UINT minwc[WORD_ENCODE_WIDTH];
  DB_UINT prev_minwc[WORD_ENCODE_WIDTH];

  in_offsets = (DB_UINT *)malloc(sizeof(DB_UINT) * checkpoints_written);
  wordcodes = (DB_UINT **)malloc(sizeof(DB_UINT *) * checkpoints_written);
  IN_WORDVEC_FILES = (FILE **)malloc(sizeof(FILE *) * checkpoints_written);
  IN_WVOFF_FILES = (FILE **)malloc(sizeof(FILE *) * checkpoints_written);

  OUT_WORDVEC_FILE = open_or_die(dbdir, WORDVEC_FNAME, "w");
  OUT_WVOFF_FILE = open_or_die(dbdir, WVOFF_FNAME, "w");

  /* Open each input file and read in the first wordcode from 
     the WVOFF files */
  for (i=0; i < checkpoints_written; i++) {
    sprintf(wordvec_fname, "%s.%.3d", WORDVEC_FNAME, i);
    sprintf(wvoff_fname, "%s.%.3d", WVOFF_FNAME, i);

    IN_WORDVEC_FILES[i] = open_or_die(dbdir, wordvec_fname, "r");
    IN_WVOFF_FILES[i] = open_or_die(dbdir, wvoff_fname, "r");
    wordcodes[i] = (DB_UINT *)malloc(sizeof(DB_UINT) * WORD_ENCODE_WIDTH);
    fread_big (wordcodes[i], sizeof(DB_UINT), WORD_ENCODE_WIDTH, 
	       IN_WVOFF_FILES[i]);
    fread_big (&(in_offsets[i]), sizeof(DB_UINT), 1, IN_WVOFF_FILES[i]);
  }

  while(!alldone) {
    alldone = 1;     /* we can always hope... */
    startedyet = 0;

    /* Find out which wordcode is the minimum */
    for (i=0; i < checkpoints_written; i++) {
      if (!feof(IN_WVOFF_FILES[i])) {
	alldone = 0;   /* not everyone's at eof yet */
	if (!startedyet || (wordcode_cmp(wordcodes[i], minwc) <= 0)) {     
	  /* new minimum word.  We're doing the <= 0 instead of -1
	     so it'll still add in reverse doc order (which is what each 
	     individual file is in) */
	  startedyet = 1;
          minwc_index = i;
	  wordcode_cpy(minwc, wordcodes[i]); /* copy it in */
	}
      }
    }

    /* Write out another word */
    if (startedyet) {
      in_offset = in_offsets[minwc_index];

      /* compute offset to next entry and update
	 wordcodes[minwc_index] with the next in line.  (use minwc for
	 this round's minimum from here on.  On EOF we won't worry
	 about the new wordvec (there isn't one), but want to make
	 sure we get the end of the wordvec file as the next offset so
	 we'll read in all the data. */
      if (fread_big (wordcodes[minwc_index], sizeof(DB_UINT), WORD_ENCODE_WIDTH,
                     IN_WVOFF_FILES[minwc_index]) < WORD_ENCODE_WIDTH) {
        if (ferror(IN_WVOFF_FILES[minwc_index])) {
          sprintf(errorstring, 
                  "merge_text_index_file: got a short read on wordcodes[%d], previous in_offset = 0x%x",
                  minwc_index, in_offset);
          SavantError(EIO, errorstring);
        }
      }
      
      if (fread_big (&in_offsets[minwc_index], sizeof(DB_UINT), 1, 
                     IN_WVOFF_FILES[minwc_index]) < 1) {
        if (feof(IN_WVOFF_FILES[minwc_index])) {
          in_offset_next = (DB_UINT)ftell_end(IN_WORDVEC_FILES[minwc_index]);
        }
        else {
          sprintf(errorstring, 
                  "merge_text_index_file: got a short read on in_offsets[%d], previous in_offset = 0x%x",
                  minwc_index, in_offset);
          SavantError(EIO, errorstring);
        }
      } 
      else {
        in_offset_next = in_offsets[minwc_index];
      }

      /* write data to output files */
      /* No document will be in more than one checkpoint file, but the
         same word might span checkpoints.  If so, just write the docs
         here. */
      if (!prev_minwc_written || wordcode_cmp(prev_minwc, minwc)) {
	out_offset = (DB_UINT)ftell(OUT_WORDVEC_FILE);
	fwrite_big (minwc, sizeof(DB_UINT),
		    WORD_ENCODE_WIDTH, OUT_WVOFF_FILE);
	fwrite_big (&out_offset, sizeof(DB_UINT),
		    1, OUT_WVOFF_FILE);
      }
      numwritten = fcpy_big (OUT_WORDVEC_FILE, 1, in_offset_next - in_offset,
			       IN_WORDVEC_FILES[minwc_index]);

      if (numwritten != in_offset_next - in_offset) {
	sprintf(errorstring, "merge_text_index_file: only %d bytes written to wordvec file, should be %d", 
                numwritten, in_offset_next - in_offset);
	SavantError(EIO, errorstring);
      }

      wordcode_cpy(prev_minwc, minwc);
      prev_minwc_written = 1;
    }
  }

  for (i=0; i < checkpoints_written; i++) {
    /* close & delete the checkpoint files */
    fclose(IN_WORDVEC_FILES[i]);
    fclose(IN_WVOFF_FILES[i]);

    sprintf(wordvec_fname, "%s%s.%.3d", dbdir, WORDVEC_FNAME, i);
    sprintf(wvoff_fname, "%s%s.%.3d", dbdir, WVOFF_FNAME, i);

    if (unlink(wordvec_fname)) {
      sprintf(errorstring, "Error unlinking (deleting) file %s", wordvec_fname);
      SavantError(errno, errorstring);
    }
    if (unlink(wvoff_fname)) {
      sprintf(errorstring, "Error unlinking (deleting) file %s", wvoff_fname);
      SavantError(errno, errorstring);
    }
  }
  fclose(OUT_WORDVEC_FILE);
  fclose(OUT_WVOFF_FILE);
  for (i=0; i < checkpoints_written; i++) {
    free(wordcodes[i]);
  }
  free(wordcodes);
  free(in_offsets);
  free(IN_WORDVEC_FILES);
  free(IN_WVOFF_FILES);
}




/* index_store: take whatever type is returned by the parser and
   stores it in the appropriate structure, for later writing to disk.
   It can also write some info to disk immediately if it needs to
   (e.g. for checkpointing so you don't run out of RAM).  Store stuff
   statically so it can be used in subsequent calls.  If last_write_p == 1, 
   make sure everything gets written to disk. 

   Assumes we get documents one at a time in docnum order. */

void index_store_text (void *parsedata,   /* Type Text_Document_Field * */
                        char *dbdir,       /* directory of the database we're 
					      writing to */
                        int last_write_p)  /* last_write_p == 1 if we should 
					      finalize it to disk now */
{
  static DV_Tree *dvtree = NULL;       /* The DV Tree being added to */
  static int checkpoints_written = 0;  /* Number of checkpoint directories 
					  written so far */
  static int mem_used_by_dvtree = 0;   /* Memory used by dvtree.  If
                                          this gets too big we write
                                          to disk and merge later */
  static int we_are_done = 0;          /* Because we might be called to 
					  finalize our last write several
					  times (once per template that uses
					  this method in fact), we use this 
					  to insure we only do a merge and 
					  write once at the end. */
  static FILE *DOCLEN_FILE = NULL;
  DV_Tree *tree = NULL;
  Text_Document_Field *pd;
  DB_UINT docnum, fieldtypenum;

  pd = (Text_Document_Field *)parsedata;

  if (pd != NULL) 
    tree = pd->tree;
  else 
    tree = NULL;

  if (DOCLEN_FILE == NULL) 
    DOCLEN_FILE = open_or_die(dbdir, DOCLEN_FNAME, "w");

  /* Write DOCLEN_FILE info */
  if (pd != NULL) {
    docnum = pd->docnum;
    fieldtypenum = pd->fieldtypenum;
    fwrite_big(&docnum, sizeof(DB_UINT), 1, DOCLEN_FILE);
    fwrite_big(&fieldtypenum, sizeof(DB_UINT), 1, DOCLEN_FILE);
    fwrite_big(&(pd->length), sizeof(DB_UINT), 1, DOCLEN_FILE);
  }

  if (!we_are_done) {  
    mem_used_by_dvtree += 
      add_document_to_dvtree(&dvtree, tree);


    if (last_write_p) {        /* Let's blow this thing and go home */
      if (checkpoints_written > 0) {
	if (dvtree != NULL) {  /* write what's remaining */
	  checkpoint_text_index_file_and_free (dvtree, checkpoints_written++, 
					       dbdir);
	}
	merge_text_index_file (checkpoints_written, dbdir); 
	dvtree = NULL;      /* it just got freed */
      }
      else {
	write_text_index_file_and_free (dvtree, dbdir, WORDVEC_FNAME, 
					WVOFF_FNAME, 1);
	dvtree = NULL;
      }
      fclose(DOCLEN_FILE);
      we_are_done = 1;
    }

    if (!last_write_p && 
	dvtree != NULL &&
	(mem_used_by_dvtree > MAX_DOC_MEMORY)) {
      checkpoint_text_index_file_and_free (dvtree, checkpoints_written++, 
					   dbdir);
      mem_used_by_dvtree = 0;  /* it got freed */
      dvtree = NULL;
    }
  }
}
