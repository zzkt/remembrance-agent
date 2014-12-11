/* retrievers-text.c has all the (hopefully reusable) retrieval
   functions for text pointed to by entries in the template
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

#include "parsedoc.h"
#include "plugins-text.h"
#include "plugins.h"
#include "conftemplates.h"
#include <math.h>

/* update_sims_word spec: All the update_sims_word routines take:
       void *word: a single element of the query vector (e.g. a single word)

       int weight: a weight for this word in the query (e.g. the term frequency)

       Doc_Sim *all_sims: Array of document similarities.  This is
       what gets updated by the routine, one word at a time.  It's a
       running total of similarities, so this might contain
       similarities already agregated from previous words.  Any
       algorithm that can run in one pass over each field & word in
       the query can be used.  The docnum is the index into all_sims.

       Field *field: a pointer to the field being updated.  This might
       be used to get field type, or can be ignored.

       Retrieval_Database_Info *rdi: info about the index database.
       Currently this struct contains the total number of documents
       and the path to the index database directory. */

/* TEXT */
/* File format (comment copied from indexers-text.c):
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


/* Do a binary search of WVOFF_FILE.  Assume word is between wvoff_low
   and wvoff_high (offsets into the file) if it exists at all.  Return
   either:

       0: The word is not in the file.
       1: The word has been found, and wvoff_low points to it.  wvoff_high points to one line higher.
          (this may be > end of file, so make sure the caller checks this) 

   This should really be rewritten to be generalized (i.e. include the
   length of a line and a pointer to the comparison program ala qsort.  */

int do_binary_search_wvoff (long *off_low_ptr, 
                            long *off_high_ptr,
                            Wordcode word,
                            FILE *WVOFF_FILE,
                            int linelen) {
  DB_UINT testword[WORD_ENCODE_WIDTH];
  long wvoff_point;
  int numlines;
  int cmp;
  int gotItp = -1;    /* Are we there yet Pappa Smurf? */
  long wvoff_low, wvoff_high;

  wvoff_low = *off_low_ptr;
  wvoff_high = *off_high_ptr;

  while (gotItp == -1) {
    /* Special case: We're at the last attempt, check boundaries */
    if ((wvoff_high - wvoff_low) == linelen) {
      fseek(WVOFF_FILE, wvoff_low, SEEK_SET);
      fread_big(testword, sizeof(DB_UINT), WORD_ENCODE_WIDTH, WVOFF_FILE);
      cmp = wordcode_cmp(testword, word);
      if (cmp == 0) {
        wvoff_high = wvoff_low + linelen;
        gotItp = 1;
      }
      else {
        fseek(WVOFF_FILE, wvoff_high, SEEK_SET);
        fread_big(testword, sizeof(DB_UINT), WORD_ENCODE_WIDTH, WVOFF_FILE);
        cmp = wordcode_cmp(testword, word);
        if (cmp == 0) {
          wvoff_low = wvoff_high;
          wvoff_high = wvoff_low + linelen;
          gotItp = 1;
        }
        gotItp = 0;
      }
    }
    else {
      numlines = (int)((wvoff_high - wvoff_low) / linelen);
      wvoff_point = (long)(((long)(numlines / 2.0)) * linelen + wvoff_low);

      fseek(WVOFF_FILE, wvoff_point, SEEK_SET);
      fread_big(testword, sizeof(DB_UINT), WORD_ENCODE_WIDTH, WVOFF_FILE);
      cmp = wordcode_cmp(testword, word);

      if (cmp == 0) {
        wvoff_high = wvoff_point + linelen;
        wvoff_low = wvoff_point;
        gotItp = 1;
      }
      else if (cmp < 0)                   /* We shot low */
        wvoff_low = wvoff_point; 
      else                          /* We shot high */
        wvoff_high = wvoff_point;
    }
  }

  *off_low_ptr = wvoff_low;
  *off_high_ptr = wvoff_high;
  return(gotItp);
}


/* Given a word, fill in an array of docnum, weight pairs associated
   with it, and return the number of docs in the array.  The array
   will be freed by whoever calls us. */
int text_documents_containing_this_word (DB_UINT **docsAndWeights,
                                         Wordcode word,
                                         FILE *WVOFF_FILE,
                                         FILE *WORDVEC_FILE) {
  static long end_wvoff_file = -1;
  static long end_wordvec_file = -1;
  long wvoff_low = 0;
  long wvoff_high, wvoff_point;
  int length_line = (WORD_ENCODE_WIDTH * sizeof(DB_UINT)) + sizeof(DB_UINT);
  int gotItp=0;
  DB_UINT offset_low, offset_high;

  if (end_wvoff_file == -1) end_wvoff_file = ftell_end(WVOFF_FILE);
  if (end_wordvec_file == -1) end_wordvec_file = ftell_end(WORDVEC_FILE);

  wvoff_high = end_wvoff_file;

  /* Do a binary search to find this word's offset (An optimization
       might be to remember our last position, and use that as our
       next low, since the words are used in order anyway.  Try this
       later maybe, not a big issue right now.) */
  gotItp = do_binary_search_wvoff(&wvoff_low, &wvoff_high, word, WVOFF_FILE, length_line);
  if (!gotItp) {
    return(0);      /* There wasn't a match */
  }
  else {
    fseek(WVOFF_FILE, wvoff_low + WORD_ENCODE_WIDTH * sizeof(DB_UINT), SEEK_SET);
    fread_big(&offset_low, sizeof(DB_UINT), 1, WVOFF_FILE);
    if (wvoff_high > end_wvoff_file) {
      offset_high = end_wordvec_file;
    }
    else {
      fseek(WVOFF_FILE, wvoff_high + WORD_ENCODE_WIDTH * sizeof(DB_UINT), SEEK_SET);
      fread_big(&offset_high, sizeof(DB_UINT), 1, WVOFF_FILE);
    }
    *docsAndWeights = (DB_UINT *)malloc((offset_high - offset_low));
    fseek(WORDVEC_FILE, offset_low, SEEK_SET);
    fread_big(*docsAndWeights, sizeof(DB_UINT), 
              (size_t)((offset_high - offset_low) / sizeof(DB_UINT)), WORDVEC_FILE);
    return ((int)((offset_high - offset_low) / (2.0 * sizeof(DB_UINT))));
  }
}


/* Load all the text doc info into memory for quick access later.
   This might give us a single-time speed hit on the first query due
   to disk access. */

Text_Doc_Info *load_text_doc_info (char *db_dir, int number_documents_total) {
  FILE *DOCLEN_FILE = NULL;
  DB_UINT *buffer, D, F;
  long filelen;
  int i, numentries;
  Text_Doc_Info *tdi;
  int numEntriesPerField[MAX_NUMBER_FIELDS];

  DOCLEN_FILE = open_or_die(db_dir, DOCLEN_FNAME, "r");
  filelen = ftell_end(DOCLEN_FILE);
  numentries = (int)(filelen / (sizeof(DB_INT) * 3));
  buffer = (DB_UINT *)malloc(filelen);
  tdi = (Text_Doc_Info *)malloc(sizeof(Text_Doc_Info));
  tdi->document_lengths = 
    (DB_UINT *)calloc(number_documents_total * MAX_NUMBER_FIELDS, sizeof(DB_UINT));
  tdi->avg_document_length =
    (DB_UINT *)calloc(MAX_NUMBER_FIELDS, sizeof(DB_UINT));
  fread_big(buffer, sizeof(DB_UINT), (size_t)(numentries * 3), DOCLEN_FILE);
  tdi->number_documents = number_documents_total;

  for (i=0; i < MAX_NUMBER_FIELDS; i++) numEntriesPerField[i] = 0;
  for (i=0; i < numentries; i++) {
    D = buffer[3 * i];
    F = buffer[3 * i + 1];
    numEntriesPerField[F]++;
    tdi->document_lengths[(D * MAX_NUMBER_FIELDS) + F] = buffer[3 * i + 2];
    tdi->avg_document_length[F] += buffer[3 * i + 2];
  }
  for (F=0; F < MAX_NUMBER_FIELDS; F++) {
    if (numEntriesPerField[F] == 0) {
      tdi->avg_document_length[F] = 0;
    }
    else {
      tdi->avg_document_length[F] = (int)(tdi->avg_document_length[F] / numEntriesPerField[F]);
    }
  }
  free(buffer);
  return(tdi);
}

/* Algorithm:
   Find docs & wieghts associated with this word through WVOFF_FILE 
   and WORDVEC_FILE.

   The weighting algorithm is essentially the one used in the City
   University Okapi BSS (Basic Search System).  See Robertson, S.E. et
   al. "Okapi at TREC-3" in Overview of the Third Text REtrieval
   Conference (TREC-3).  Edited by D.K. Harman.  Gaithersburg, MD:
   NIST, 1995

   Additional similarity  =   W * tf (k1 + 1) (k3 + 1) qtf 
                              ------------------------------
                                   (K + tf) (k3 + qtf)

   Where:
     W = log(N - n + 0.5) - log(n + 0.5)
     N = total number of documents 
     n = number of documents containing this word
     K = k1 ((1 - b) + b * dl / avdl)
     dl = document length (number of words)
     avdl = average document length (number of words, rounded off)
     tf = term frequency (weight) of this word in the document
     qtf = term frequency (weight) of this word in the query
     k1 = a knob: high k1 means tf is more important ( == 1.2 in City's TREC-6)
     k3 = a knob: high k3 means qtf is more important (== anywhere from 0 to 1000 in TREC-6)
     b = a knob: high b means penalize big documents more (== 0.75 in TREC-6)
   
*/

/* We've eliminated query-term-frequency as a seperate argument, and
   instead have nextword return ALL info that might be needed by the
   particular algorithm (as a void *).  E.g. query term weight, number
   of terms in the query, or whatever.  This also lets, e.g., a
   different algorithm use floats instead of ints for their qtf.  */

void update_sims_word_text_okapi(void *wordvoid,     /* The word (type Text_Word_Info) */
                                 Remem_Hash_Table *all_sims,
                                 void *fieldvoid,    /* This field (type Field) */
                                 Retrieval_Database_Info *rdi) {
  static Text_Doc_Info *tdi = NULL;
  static FILE *WVOFF_FILE = NULL;
  static FILE *WORDVEC_FILE = NULL;
  static float partial_normalization_term = -1.0;   /* Cacheable part of the normalization term */

  Doc_Sim *sim_element;
  int numDocsForThisWord, i,j;
  float additionalSim = 0.0;
  DB_UINT docnum, weight;
  DB_UINT *docsAndWeights = NULL;
  float fudgefactor = 3.0;    /* Constant multiplier for the whole thing -- to offset overnormalization.
                                 This sucks, but I haven't got a better way yet. */
  /* Vars for the Okapi algorithm */
  /* These have to be declared volatile because of a linux gcc (&
     others?) compiler bug that gets tickled if the optimizer tries to
     inline any of these things (or some of these things -- I didn't
     figure out which.  */
  volatile float W, K, k1, k3, b;
  volatile int N, n, dl, avdl, tf, qtf, query_length;

  volatile float Wmax, Kmax, normalization_factor;    /* For computing normalizaton factor */
  volatile int tfmax, qtfmax;                         /* For computing normalizaton factor */

  N = rdi->number_documents_total;
  k1 = 1.2;
  k3 = 100.0;  /* Set high, since we expect long queries and we want to exploit that fact */
  b = 0.75;

  /* partial_normalization_term is only computed once, and doesn't
     change for a particular index database.  We incorporate the query
     length in later.  This is added to the Okapi algorithm to make
     sure the similarity doesn't go over 1.0 for a given field. */
  if (partial_normalization_term < 0.0) {
    Wmax = (float)(log(N - 0.5) - log(1.5));
    Kmax = (float)(k1 * (1.0 - b));
    tfmax = 1;    /* Assume this is a good max -- we pin it if we go over 1.0 similarity anyway */
    qtfmax = 1;   /* Assume this is a good max -- we pin it if we go over 1.0 similarity anyway */
    partial_normalization_term = (float)((Wmax * tfmax * (k1 + 1) * (k3 + 1) * qtfmax) 
                                         / (float)((Kmax + tfmax) * (k3 + qtfmax) * fudgefactor));
  }

  if (tdi == NULL) 
    tdi = load_text_doc_info(rdi->db_dir, rdi->number_documents_total);
  if (WVOFF_FILE == NULL) 
    WVOFF_FILE = open_or_die(rdi->db_dir, WVOFF_FNAME, "r");
  if (WORDVEC_FILE == NULL) 
    WORDVEC_FILE = open_or_die(rdi->db_dir, WORDVEC_FNAME, "r");

  /* For this word, find all the docs that contain it */
  numDocsForThisWord = 
    text_documents_containing_this_word(&docsAndWeights,
					((Text_Word_Info *)wordvoid)->word,
					WVOFF_FILE,
					WORDVEC_FILE);
  if (numDocsForThisWord != 0) {  /* Only do something if there were any hits */
    n = numDocsForThisWord;
    avdl = tdi->avg_document_length[((Field *)fieldvoid)->typenum];
    if (avdl < 1) avdl = 1;       /* Sanity check */

    for (i = 0; i < numDocsForThisWord; i++) {
      docnum = docsAndWeights[i * 2];

      qtf = ((Text_Word_Info *)wordvoid)->weight;

      /* query_length = number of unique non-stopwords total */
      query_length = ((Text_Word_Info *)wordvoid)->length; 

      tf = docsAndWeights[i * 2 + 1];
      dl = tdi->document_lengths[(docnum * MAX_NUMBER_FIELDS) + ((Field *)fieldvoid)->typenum];
      K = (float)(k1 * ((1.0 - b) + b * (double)dl / (double)avdl));
      W = (float)(log(N - n + 0.5) - log(n + 0.5));

      /* this * query_length is serious black magic.  There's no
         real reason to think that this will give a normalized value across
         several queries, which is what we want with thresholds. 

         What we actually want is a normalized query that corelates
         well with how useful people find a suggestion. It makes sense
         (and preliminary experiments show) that longer queries will
         give better suggestions because they have more material /
         words.  However, without normalization longer queries have a
         *much* higher return. We need to find a happy medium.

 */
      normalization_factor = partial_normalization_term * (float)query_length;
      additionalSim = (float)((W * tf * (k1 + 1) * (k3 + 1) * qtf) / (float)((K + tf) * (k3 + qtf)));
      additionalSim = additionalSim / normalization_factor;

      /* Get doc's sim if there, add to hash if not */
      sim_element = (Doc_Sim *)rememHashGet(docnum, all_sims);
      if (sim_element == NULL) {
	sim_element = (Doc_Sim *)calloc(1,sizeof(Doc_Sim));
	sim_element->docnum = docnum;
	rememHashPut(docnum, sim_element, all_sims);
      }

      add_additional_to_doc_sim (sim_element, additionalSim, 
				 ((Text_Word_Info *)wordvoid)->printword);
    }
  }
  free(docsAndWeights);

}

/* Algorithm:
   Find docs & wieghts associated with this word through WVOFF_FILE 
   and WORDVEC_FILE.

   There are several TFiDF algorithms, so this has a few choices.  The three main 
   components are:
         ntf: A normalized term frequency for the word within a particular document
         IDF: An inverse document frequency weighting, which favors rare words
         the similarity metric that combines them.

   There are several choices for each, several listed in:
      Harman, Donna.  "Ranking Algorithms" in Frakes, WB and R
      Baewa-Yates (eds), "Information Retrieval: Data Structures and
      Algorithms." Prentice Hall, Upper Saddle River, NJ 92,
      pp. 373-376.

   Currently I'm using:
      IDF = log((N - n) / n);                  (Croft and Harper, 1979)
      ntf = log(tf + 1) / log(doclength);      (Harman 1986)
      similarity = sum-over-query (ntf * IDF)  (Harman 1986)
*/
void update_sims_word_text_tfidf(void *wordvoid,     /* The word (type Text_Word_Info) */
                                 Remem_Hash_Table *all_sims,
                                 void *fieldvoid,    /* This field (type Field) */
                                 Retrieval_Database_Info *rdi) {
  static Text_Doc_Info *tdi = NULL;
  static FILE *WVOFF_FILE = NULL;
  static FILE *WORDVEC_FILE = NULL;
  static float partial_normalization_term = -1.0;   /* Cacheable part of the normalization term */
  const int fudge_factor = 12;  /* fudge factor to divide by so value is between 0 and 1. This is disgusting. */

  Doc_Sim *sim_element;
  int numDocsForThisWord, i;
  float additionalSim = 0.0;
  DB_UINT docnum, weight;
  DB_UINT *docsAndWeights = NULL;

  /* Vars for TFiDF */
  /* These have to be declared volatile because of a linux gcc (&
     others?) compiler bug that gets tickled if the optimizer tries to
     inline any of these things (or some of these things -- I didn't
     figure out which.  */
  volatile float IDF, ntf;
  volatile int N, n, dl, tf;

  N = rdi->number_documents_total;

  if (tdi == NULL) 
    tdi = load_text_doc_info(rdi->db_dir, rdi->number_documents_total);
  if (WVOFF_FILE == NULL) 
    WVOFF_FILE = open_or_die(rdi->db_dir, WVOFF_FNAME, "r");
  if (WORDVEC_FILE == NULL) 
    WORDVEC_FILE = open_or_die(rdi->db_dir, WORDVEC_FNAME, "r");

  /* For this word, find all the docs that contain it */
  numDocsForThisWord = 
    text_documents_containing_this_word(&docsAndWeights,
					((Text_Word_Info *)wordvoid)->word,
					WVOFF_FILE,
					WORDVEC_FILE);
  if (numDocsForThisWord != 0) {  /* Only do something if there were any hits */
    n = numDocsForThisWord;
    if (n < N) {              /* Sanity check */
      IDF = log((N - n) / n);
      for (i = 0; i < numDocsForThisWord; i++) {
        docnum = docsAndWeights[i * 2];
        tf = docsAndWeights[i * 2 + 1];
        dl = tdi->document_lengths[(docnum * MAX_NUMBER_FIELDS) + ((Field *)fieldvoid)->typenum];

        if (dl > 2)                    /* More sanity check */
          ntf = log(tf + 1) / log(dl);
        else 
          ntf = tf;

        additionalSim = (float)(ntf * IDF / fudge_factor);

        /* Get doc's sim if there, add to hash if not */
        sim_element = (Doc_Sim *)rememHashGet(docnum, all_sims);
        if (sim_element == NULL) {
          sim_element = (Doc_Sim *)calloc(1,sizeof(Doc_Sim));
          sim_element->docnum = docnum;
          rememHashPut(docnum, sim_element, all_sims);
        }

        add_additional_to_doc_sim (sim_element, additionalSim, ((Text_Word_Info *)wordvoid)->printword);
      }
    }
  }
  free(docsAndWeights);
}


