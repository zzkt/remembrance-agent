/* rmain.c: This is the main for Savant 2.0, now with regexp parsing. */

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

#include "rmain.h"
#include "parsedoc.h"
#include "conftemplates.h"
#include "plugins.h"
#include <savant.h>
#include <savutil.h>
#include <string.h>
#include <stdlib.h>

extern List_of_General_Templates *All_General_Templates;
extern Array_of_Field_Types *All_Fields;

void instructions(void);
void RetrieveError (int errcode, char *errstring);

int SavantVerbose = 0, SavantDebug = 0, SavantFollowSymlinks = 0;
UserVars Config;       /* Initialized in load_config */

/* RetrieveError is Retrieve's error handler.  SavantError will
   point to it.  All it does is print the error string
   and exit.  */
void RetrieveError (int errcode, char *errstring)
{
  fprintf(stderr, "%s\n", errstring);
  exit(errcode);
}

void instructions(void)  
{
  fprintf(stderr,"%s\n", REMEM_VERSION);
  fprintf(stderr,"\nusage:\n");
  fprintf(stderr,"   ra-retrieve [--version] [-v] [-d] <base-dir> [--docnum <docnum>]\n");
  fprintf(stderr,"   -v invokes savant retrieval verbosely;  -d provides debugging messages.\n");
  fprintf(stderr,"   --docnum prints the specified docnum and exits (also doesn't use as much memory).\n");
  fflush(stderr);
}

void print_menu(void)  
{
  printf("\n"
         "query [n]          : Find n most relevant documents to a query.  Default is 5.\n"
         "retrieve n         : Retrieve and print the document with document number n.\n"
         "loc-retrieve n     : Retrieve and print the document location for document\n"
         "                     number n.\n"
         "info               : Display version and database info.\n"
         "quit               : Quit.\n"

/*  These are advanced, and probably won't be used in interactive mode anyway.

         "print-biases       : List the handset query biases for each field defined in the template structure.\n"
         "use-handset-biases : Use handset biases instead of those defined by the query templates.\n"
         "use-template-biases: Use template biases instead of those defined by hand (default).\n"
         "set-bias <fieldname> <value> : set the value for an individual query bias.\n"
*/
         "?                  : Display this help.\n"
         "\nCommand: ");
  fflush(stdout);
}

enum Retrieve_Command get_command (int *argint, char *argstring) {
  char command[129];
  char *argptr=NULL;
  char *spacepos1, *spacepos2;
  *argint = 0;
  argstring[0]='\0';

  if (feof(stdin)) return(QUIT_COMMAND);
  fgets(command,128,stdin);
  argptr = strstr(command, "query");

  if (argptr != NULL) {    
    *argint = atoi(argptr + 5);
    return(QUERY_COMMAND);
  }

  if ((argptr = strstr(command, "quit")) != NULL) {
    return(QUIT_COMMAND);
  }

  if ((argptr = strstr(command, "loc-retrieve")) != NULL) {
    *argint = atoi(argptr + 12);
    return(LOCRETRIEVE_COMMAND);
  }

  if ((argptr = strstr(command, "retrieve")) != NULL) {
    *argint = atoi(argptr + 8);
    return(RETRIEVE_COMMAND);
  }

  if (((argptr = strstr(command, "?")) != NULL) ||
      ((argptr = strstr(command, "help")) != NULL)) {
    return(HELP_COMMAND);
  }

  if ((argptr = strstr(command, "print-biases")) != NULL) {
    return(PRINT_BIASES);
  }

  if ((argptr = strstr(command, "use-handset-biases")) != NULL) {
    return(USE_HANDSET_BIASES);
  }

  if ((argptr = strstr(command, "use-template-biases")) != NULL) {
    return(USE_TEMPLATE_BIASES);
  }

  if ((argptr = strstr(command, "set-bias")) != NULL) {
    spacepos1 = strchr(command, (int)' ');
    if (spacepos1 != NULL) {
      spacepos2 = strchr(spacepos1+1, (int)' ');
      if (spacepos2 != NULL) {
        strncpy(argstring, spacepos1+1, RA_MIN(MAX_FIELD_NAME_LENGTH, 
                                               (spacepos2 - spacepos1 - 1)));
        argstring[RA_MIN(MAX_FIELD_NAME_LENGTH, (spacepos2 - spacepos1 - 1))] = '\0';
        *argint = atoi(spacepos2 + 1);
      }
    }
    else {
      argstring[0] = '\0';
      *argint = -1;
    }
    return(SET_BIAS_COMMAND);
  }

  if ((argptr = strstr(command, "info")) != NULL) {
    return(DB_INFO_COMMAND);
  }

  return(UNKNOWN_COMMAND);
}

void get_query(GBuffer *query) {
  char line[129];

  if(SavantVerbose) {
    printf("Type your query now. End with a control-D or control-E on a line by itself.\n");
    fflush(stdout);
  }
  while(fgets(line,128,stdin) && !(feof(stdin))) {
    if (strcmp(line,"\n") == 0) {
      break;
    }

    strncat_GBuffer(query, line, strlen(line));
  }
  clearerr(stdin);
}

void resetTopContributors (Top_Contributors *tc) {
  int j;
  for (j=0; j < NUMBER_CONTRIBUTORS_TRACKED; j++) {
    tc[j].sim = 0.0;
    tc[j].printword[0] = '\0';
  }
}

void resetDocSims(Remem_Hash_Table *docSims) {
  rememHashClear(docSims);
} 

void resetDocSimsTotals(Remem_Hash_Table *docSims) {
  rememHashClear(docSims);
}

Remem_Hash_Table *initDocSims(int num_docs_total) {
  Remem_Hash_Table *return_value;
  int j;
  return_value = (Remem_Hash_Table *)rememHashCreateTable(65536 + 1);
  if (return_value == NULL) {
    SavantError(ENOMEM, "Can't allocate DocSims in rmain.c");
  }
  return(return_value);
}

Remem_Hash_Table *initDocSimsTotal(int num_docs_total) {
  Remem_Hash_Table *return_value;
  int j;
  return_value = (Remem_Hash_Table *)rememHashCreateTable(65536 + 1);
  if (return_value == NULL) {
    SavantError(ENOMEM, "Can't allocate DocSimsTotal in rmain.c");
  }
  return(return_value);
}

/* Return the number of documents in the database, judging by the size
   of the titles offset file */
int number_documents(char *db_dir) {
  FILE *TOFFS_FILE;
  int retval;
  TOFFS_FILE = open_or_die(db_dir, TOFFS_FNAME, "r");
  retval = (int)(ftell_end(TOFFS_FILE) / sizeof(DB_INT));
  fclose(TOFFS_FILE);
  return(retval);
}

/* For qsort.  Note that we return -1 for docsim1 > docsim2, so 
   that we get large-to-small ordering. */
int compareDocSims(const void *docsim1, const void *docsim2) {
  Doc_Sim *ds1, *ds2;
  ds1 = (Doc_Sim *)docsim1;
  ds2 = (Doc_Sim *)docsim2;

  if (ds1->sim > ds2->sim) 
    return -1;
  else if (ds1->sim < ds2->sim)
    return 1;
  else return 0;
}

/* For qsort.  Note that we return -1 for docsim1 > docsim2, so 
   that we get large-to-small ordering. */
int compareDocSimsTotals(const void *docsim1, const void *docsim2) {
  Doc_Sim_Totals *ds1, *ds2;
  ds1 = (Doc_Sim_Totals *)docsim1;
  ds2 = (Doc_Sim_Totals *)docsim2;

  if (ds1->docsim.sim > ds2->docsim.sim) 
    return -1;
  else if (ds1->docsim.sim < ds2->docsim.sim)
    return 1;
  else return 0;
}

/* Apply biases to these doc sims 

   The query and index vectors each have associated biases attached to
   each of their fields, like so:
     
     Query biases = q1, q2, ..., q{num_fields}
     Index biases = i1, i2, ..., i{num_fields}
     
     Non-normalized biases = q1*i1, q2*i2, ...
     
     Normalized biases = q1*i1/M, q2*i2/M, ...
     where M = combined_bias_sum = q1*i1 + q2*i2 + ... 

     Biased-sim = sim * normalized-bias
*/
 void bias_sims(Remem_Hash_Table *all_sims, 
               List_of_General_Templates *All_General_Templates, 
               Field *thisfield,
               int *querybiases,
               Retrieval_Database_Info *rdi,
               int newquery_p) {
  static int **index_biases = NULL;      /* Cached, computed once ever */
  static int number_templates = -1;      /* Cached, computed once ever */
  static float **combined_biases = NULL;   /* Cached, computed once per query (when newquery_p != 0) */

  Doc_Sim *sim_element;
  General_Template *current_template = NULL;
  List_of_General_Templates *lotptr = NULL;
  Template_Field_Info *tfi = NULL;
  DB_UINT i;
  int T, F, bias;
  int combined_bias_magnitude = 0;

  /* A lot of info gets cached.  Set it here, if not set yet. */
  if (number_templates == -1) {
    number_templates = 0;
    for (lotptr = All_General_Templates; (lotptr != NULL); lotptr = lotptr->next) {
      number_templates++;
    }
  }
  /* index_biases is a 2D array of biases for the different kinds of index files:
     index_bias[T][F] = index bias for template-typenum T and field-typenum F */
  if (index_biases == NULL) {
    index_biases = (int **)malloc(number_templates * sizeof(int *));
    for (i=0; i < number_templates; i++) {
      index_biases[i] = (int *)calloc(MAX_NUMBER_FIELDS, sizeof(int));
    }
    for (lotptr = All_General_Templates; (lotptr != NULL); lotptr = lotptr->next) {
      for (i=0; (lotptr->template->fields[i] != NULL) && (i < MAX_NUMBER_FIELDS); i++) {
        T = lotptr->template->typenum;
        F = lotptr->template->fields[i]->field->typenum;
        bias = lotptr->template->fields[i]->bias;
        index_biases[T][F] = bias;
      }
    }
  }

  if (combined_biases == NULL) {
    combined_biases = (float **)malloc(number_templates * sizeof(float *));
    for (i=0; i < number_templates; i++) {
      combined_biases[i] = (float *)malloc(MAX_NUMBER_FIELDS * sizeof(float));
    }
  }
  if (newquery_p) {
    for (T=0; T < number_templates; T++) {
      combined_bias_magnitude = 0;
      for (F=0; F < MAX_NUMBER_FIELDS; F++) {
        combined_biases[T][F] = (float)(querybiases[F] * index_biases[T][F]);
        combined_bias_magnitude += querybiases[F] * index_biases[T][F];
      }
      if (combined_bias_magnitude > 0) {          /* Sanity check -- might be a type "REJECT" template */
        for (F=0; F < MAX_NUMBER_FIELDS; F++) {
          combined_biases[T][F] = combined_biases[T][F] / (float)combined_bias_magnitude;
        }
      }
    }
  }

  /* OK, so now all the (possibly cached) data is computed -- go
     through the docs and apply the bias */
  for (sim_element = (Doc_Sim *)rememHashItterate(all_sims,1);
       sim_element != NULL;
       sim_element = (Doc_Sim *)rememHashItterate(all_sims,0)) {
    if (sim_element->sim < 0.0) {  /* Sanity Check */
      sim_element->sim = 0.0;
    }
    else {
      if (sim_element->sim > 1.0) {
	sim_element->sim = 1.0;
      }
      if (sim_element->sim > 0.0) {
	T = docloc_templateno(rdi, sim_element->docnum);
	F = thisfield->typenum;

	sim_element->sim = sim_element->sim * combined_biases[T][F];
      }
    }
  }
 }

/* Do a sort on the top M doc-sims from the hash table, returning an
   array of Doc_Sim_Totals of length M. 
   First just find the top M in any order, then do a q-sort on them.
*/

Doc_Sim_Totals *sortDocSims(Remem_Hash_Table *docSims, 
			    int num_docs_total, 
			    int num_to_sort) {
  int i,j, minindex;
  volatile float maxsim, minsim;
  Doc_Sim_Totals tempDocSim;
  Doc_Sim_Totals *sim_total_element;
  Doc_Sim_Totals *topSims;


  topSims = (Doc_Sim_Totals *)malloc(sizeof(Doc_Sim_Totals) * num_to_sort);

  if (num_to_sort > num_docs_total) num_to_sort = num_docs_total;

  for (i=0;i<num_to_sort; i++) {
    topSims[i].docsim.sim = -1.0;
  }
  minindex = 0;

  /* Do a single pass and pull out the top sims */
  for (sim_total_element = (Doc_Sim_Totals *)rememHashItterate(docSims,1);
       sim_total_element != NULL;
       sim_total_element = (Doc_Sim_Totals *)rememHashItterate(docSims,0)) {
    if (sim_total_element->docsim.sim > topSims[minindex].docsim.sim) {
      memcpy(&(topSims[minindex]), sim_total_element, sizeof(Doc_Sim_Totals));
      for (i=1, minsim = topSims[0].docsim.sim, minindex=0;
	   i<num_to_sort; i++) {
	if (topSims[i].docsim.sim < topSims[minindex].docsim.sim) {
	  minindex = i;
	  minsim = topSims[i].docsim.sim;
	}
      }
    }
  }
  qsort((void *)topSims, 
	num_to_sort, 
	sizeof(Doc_Sim_Totals), 
	&compareDocSimsTotals);

  return(topSims);
}

/* Figure out similarity for all fields, given this query and all documents.
   This includes figuring out biases.  Return info in total_sims, sorted by similarity. 

   Method:
   for each field:
     for each word in the query:
       Update similarities for this word
     bias the similarities for this field
   sort the results & return them as an array of Doc_Sim_Totals of
   length number_docs_being_printed */
Doc_Sim_Totals *rank_docs_for_fields(Remem_Hash_Table *total_sims, 
                          Remem_Hash_Table *all_sims,
                          Doc_Info *queryInfo, 
                          General_Template *query_template,
                          List_of_General_Templates *All_General_Templates,
			  Retrieval_Database_Info *rdi,
                          int number_docs_being_printed,
                          int *querybiases) {
  void *(*nextword)(void *fielddata, int reset_p) = NULL;
  void (*update_sims_word)(void *word, Remem_Hash_Table *all_sims, void *self,
			   Retrieval_Database_Info *rdi) = NULL;   
  int fieldnum, i;
  void *newword = NULL;
  int wordweight = 0;
  int newquery_p = 1;
  Field *thisfield;
  Doc_Sim_Totals *sim_total_element;
  Doc_Sim *sim_element;

  /* Find and add in a similarity for each field */
  for (fieldnum = 0; 
       ((fieldnum < MAX_NUMBER_FIELDS) && 
	(query_template->fields[fieldnum] != NULL)); 
       fieldnum++) {
    thisfield = query_template->fields[fieldnum]->field;

    nextword = thisfield->nextword;
    update_sims_word = thisfield->update_sims_word;

    /* Skip if don't have functions defined */
    if ((nextword != NULL) && (update_sims_word != NULL)) {      
      newword = nextword(queryInfo->parsedfields[fieldnum], 1);

      /* If no word at all, go on to next field */
      if (newword != NULL) {                      
        while (newword != NULL) {
          update_sims_word(newword, all_sims, thisfield, rdi);
          free(newword);
          newword = nextword(queryInfo->parsedfields[fieldnum], 0);
        }      
        
	/* modify the sims to handle index and query weights (biases) */
        bias_sims(all_sims, All_General_Templates, thisfield, 
                  querybiases, rdi, newquery_p);
        newquery_p = 0;
        
	/* For each similarity of this field, add it to the total_sims
	   hash table. */
	for (sim_element = (Doc_Sim *)rememHashItterate(all_sims,1);
	     sim_element != NULL;
	     sim_element = (Doc_Sim *)rememHashItterate(all_sims,0)) {
	  if (sim_element->sim > 0.0) {
	    sim_total_element = 
	      (Doc_Sim_Totals *)rememHashGet(sim_element->docnum, total_sims);

	    /* If not in the totals hash table yet, put it there */
	    if (sim_total_element == NULL) {
	      sim_total_element = 
		(Doc_Sim_Totals *)calloc(1,sizeof(Doc_Sim_Totals));
	      sim_total_element->docsim.docnum = sim_element->docnum;
	      rememHashPut(sim_element->docnum, sim_total_element, total_sims);
	    }
	    
	    sim_total_element->docsim.sim += sim_element->sim;
	    sim_total_element->sim_breakdown[thisfield->typenum] +=
	      sim_element->sim;
	    merge_top_contributors(sim_total_element->docsim.top_contributors,
				   sim_element->top_contributors);
	    sim_element->sim = 0.0;
	    resetTopContributors(sim_element->top_contributors);
	  }
	}
      }
    }
  }
  return(sortDocSims(total_sims, rdi->number_documents_total, 
		     number_docs_being_printed));
}

/* Set querybiases to the defaults set in tfi */
void set_querybiases (int *querybiases, Template_Field_Info **tfi) {
  int i;
  for (i=0; i < MAX_NUMBER_FIELDS; i++) {
    querybiases[i] = 0;
  }
  for (i=0; ((tfi[i] != NULL) && (i < MAX_NUMBER_FIELDS)); i++) {
    querybiases[tfi[i]->field->typenum] = tfi[i]->bias;
  }
}


/* print a particular document. */
void document_name_and_offsets (int docnum, 
                                Retrieval_Database_Info *rdi,
                                char *docfilename,
                                DB_INT *doc_start,
                                DB_INT *doc_end) {
  static FILE *DOCLOC_FILE = NULL;
  static FILE *DLOFFS_FILE = NULL;
  static long endDoclocFile = -1;

  DB_INT dloff_low, dloff_high, template_number;

  if (DOCLOC_FILE == NULL) 
    DOCLOC_FILE = open_or_die(rdi->db_dir, DOCLOC_FNAME, "r");
  if (DLOFFS_FILE == NULL) 
    DLOFFS_FILE = open_or_die(rdi->db_dir, DLOFFS_FNAME, "r");

  if (endDoclocFile == -1) endDoclocFile = ftell_end(DOCLOC_FILE);

  fseek(DLOFFS_FILE, docnum * sizeof(DB_INT) * 5, SEEK_SET);
  fread_big(&dloff_low, sizeof(DB_INT), 1, DLOFFS_FILE);
  fread_big(&dloff_high, sizeof(DB_INT), 1, DLOFFS_FILE);
  fread_big(doc_start, sizeof(DB_INT), 1, DLOFFS_FILE);
  fread_big(doc_end, sizeof(DB_INT), 1, DLOFFS_FILE);
  fread_big(&template_number, sizeof(DB_INT), 1, DLOFFS_FILE);

  fseek(DOCLOC_FILE, dloff_low, SEEK_SET);
  fread_big(docfilename, 1, dloff_high - dloff_low, DOCLOC_FILE);
  docfilename[dloff_high - dloff_low] = '\0';
}


/* print the top n document titles.  Doc_Sim is presumed sorted. */
void print_top_docs(Doc_Sim_Totals *all_sims, 
		    int numDocsToPrint,
		    Retrieval_Database_Info *rdi) {
  static FILE *TITLES_FILE = NULL;
  static FILE *TOFFS_FILE = NULL;
  static long endTitlesFile = -1;

  char printbuf[TITLE_LENGTH_MAX + 1];
  int i,j;
  DB_UINT docnum, toff_dbuint;
  float sim;
  long toff, toff_high;
  size_t length;

  if (TITLES_FILE == NULL) 
    TITLES_FILE = open_or_die(rdi->db_dir, TITLES_FNAME, "r");

  if (TOFFS_FILE == NULL) 
    TOFFS_FILE = open_or_die(rdi->db_dir, TOFFS_FNAME, "r");

  if (endTitlesFile == -1) endTitlesFile = ftell_end(TITLES_FILE);

  for (i=0; ((i < numDocsToPrint) && 
             (i < rdi->number_documents_total) &&
             (all_sims[i].docsim.sim > 0.0));
       i++) {
    docnum = all_sims[i].docsim.docnum;
    sim = all_sims[i].docsim.sim;
    fseek(TOFFS_FILE, docnum * sizeof(DB_INT), SEEK_SET);
    fread_big((void *)&toff_dbuint, sizeof(DB_INT), 1, TOFFS_FILE);
    toff = toff_dbuint;
    if (docnum == rdi->number_documents_total - 1)
      length = endTitlesFile - toff - 1;  /* the -1 is to remove the CR */
    else {
      fread_big((void *)&toff_dbuint, sizeof(DB_INT), 1, TOFFS_FILE);
      toff_high = toff_dbuint;
      length = toff_high - toff - 1;  /* the -1 is to remove the CR */
    }

    fseek(TITLES_FILE, toff, SEEK_SET);
    fread_big(printbuf, 1, length, TITLES_FILE);
    printbuf[length] = '\0';
    printf("%-4d%.2f | %d | %s", i+1, sim, docnum, printbuf);
    qsort(all_sims[i].docsim.top_contributors, NUMBER_CONTRIBUTORS_TRACKED,
          sizeof(Top_Contributors), &top_contributors_cmp_qsort);
    for (j=0; (j < NUMBER_CONTRIBUTORS_TRACKED); j++) {
      if (strlen(all_sims[i].docsim.top_contributors[j].printword) > 0) {
        if (j>0) printf(", ");
        printf("%s", all_sims[i].docsim.top_contributors[j].printword);
      }
    }
    printf(" | ");
    for (j=0; j < MAX_NUMBER_FIELDS; j++) {
      if (j>0) printf(", ");
      printf("%.2f", all_sims[i].sim_breakdown[j]);
    }
    printf("\n");
  }
  printf("%s\n", RETRIEVAL_SEPERATOR_STRING);
  fflush(stdout);
}

/* Print the contents of the given docnum.  Executed by a retrieve command or with the --docnum commandline arg */
void print_document_contents (int docnum, Retrieval_Database_Info *rdi) {
  FILE *RetrievedFile;
  char *retrievebuf;
  char docfilename[PATH_MAX];
  int bytesread, i;
  List_of_General_Templates *templatelist;
  DB_INT templateno;  /* Template number for this doc */
  DB_INT doc_start, doc_end;

  if ((docnum < 0) || (docnum >= rdi->number_documents_total))
    printf("Document numbers range from 0 to %d\n", rdi->number_documents_total - 1);
  else {
    document_name_and_offsets(docnum, rdi, docfilename, &doc_start, &doc_end);
    RetrievedFile = fopen(docfilename, "r");
    if (RetrievedFile == NULL) {
      printf("Can't open file %s\n(Perhaps the index files are out of date?).\n", docfilename);
    }
    else {
      if (fseek(RetrievedFile, doc_start, SEEK_SET)) {
        printf("Can't find character offset %d in file %s\n(Perhaps the index files are out of date?).\n",
               doc_start, docfilename);
      }
      else {
        retrievebuf = (char *)malloc((doc_end - doc_start + 1) * sizeof(char));
        bytesread = fread(retrievebuf, sizeof(char), doc_end - doc_start, RetrievedFile);
        retrievebuf[bytesread] = '\0';

        templateno = docloc_templateno (rdi, docnum);
        for (i=0, templatelist = All_General_Templates; i < templateno; i++, templatelist = templatelist->next);

        printf("0\n%s\n%s", templatelist->template->printname, retrievebuf);
        printf("%s\n", RETRIEVAL_SEPERATOR_STRING);
        free(retrievebuf);
      }
    }
  }
}


int main(int argc, char *argv[]) {
  GBuffer *temp;
  int i;
  char *config_name=NULL, *db_name=NULL, db_dir[1024], errorstring[128];
  enum Retrieve_Command retrieve_command;   /* command entered in the command loop */
  int command_arg;  /* arg for the command entered in the command loop */
  char command_argstring[MAX_FIELD_NAME_LENGTH];
  General_Template *template = NULL;   /* template for this query */
  Doc_Info queryInfo;
  GBuffer query;    /* the query entered */
  Doc_Sim_Totals *sorted_sims_array;
  List_of_General_Templates *templatelist;
  DB_INT templateno;
  Remem_Hash_Table *all_sims;
  Remem_Hash_Table *total_sims;

  int override_query_biases = 0;
  int querybiases[MAX_NUMBER_FIELDS], handsetbiases[MAX_NUMBER_FIELDS];
  char docfilename[PATH_MAX];
  DB_INT doc_start, doc_end;
  Retrieval_Database_Info *rdi;
  Field *thisfield;
  char *tmp;
  long int docnumToLoad = -1;

  /* Set the error handler to be RetrieveError */
  SetSavantError(&RetrieveError);

  for (i=1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'c':
	if (config_name == NULL) {
	  config_name = argv[++i];
	}
	else {
	  instructions();
          SavantError(EINVAL, "");
	  /* exit(-1); */
	}
	break;
      case 'v':
	SavantVerbose = 1;
	break;
      case 'd':
	SavantDebug = 1;
	break;
      case '-':
        if (strcmp(argv[i], "--version")==0) {
          printf("%s\n", REMEM_VERSION);
          SavantError(EINVAL, "");
        }
        else if (strcmp(argv[i], "--docnum")==0) {
          docnumToLoad = strtol(argv[++i], &tmp, 10);
          if (tmp==argv[i]) {
            sprintf(errorstring, "%s is not a valid docnum\n", argv[i]);
            SavantError(EINVAL, errorstring);
          }
        }
        break;
        
      default:
	instructions();
        SavantError(EINVAL, "");
	/* exit(-1); */
      }
    }
    else if (db_name == NULL) {
      db_name = argv[i];
    }
    else {
      instructions();
      SavantError(EINVAL, "");
      /* exit(-1); */
    }
  }

  if(SavantVerbose || SavantDebug) {
    printf("%s\n",REMEM_VERSION);
  }

  if (db_name == NULL) {
    instructions();
    SavantError(EINVAL, "");
    /* exit(-1); */
  }

  /* if not absolute pathname, tack it onto cwd */
  /*  if(db_name[0] != '/') {
      getcwd(db_dir, 510);
      strcat(db_dir, "/");
      strcat(db_dir, db_name);
      }
      else {
  */
  strcpy(db_dir, db_name);
  /*  }*/

  /* Initialize stuff (damn but I want a real OO language!) */
  rdi = (Retrieval_Database_Info *)malloc(sizeof(Retrieval_Database_Info));

  /* Now that that's set, we can find out how many docs we're looking at */
  rdi->number_documents_total = number_documents(db_dir);
  rdi->db_dir = db_dir;

  if (SavantDebug) printf("rmain.c: About to load config\n");
  load_config();

  /* If they asked for a docnum on the command-line, just print and exit.
     No need to do memory-allocation for the similarity stuff or other
     query-type things. */
  if (docnumToLoad >= 0) {
    print_document_contents(docnumToLoad, rdi);
    SavantError(EINVAL, "");
  }

  /* Initialize query-stuff, 'cause we're going interactive */
  queryInfo.filename = NULL; queryInfo.doc_start = 0; queryInfo.doc_end = 0; queryInfo.docnum = 0;
  queryInfo.documentText = NULL;
  for (i=0; i < MAX_NUMBER_FIELDS; i++) {
    queryInfo.fields[i] = (GBuffer *)malloc(sizeof(GBuffer));
    init_GBuffer(queryInfo.fields[i]);
  }
  init_GBuffer(&query);

  all_sims = initDocSims(rdi->number_documents_total);
  total_sims = initDocSimsTotal(rdi->number_documents_total);

/*
OK, here's the old call-herarchy for retrieve.  It's ugly.

main: handle all the command-line args and initialize variables

  init_read: open all the index files, and read in all the docvec
  magnitudes.  Also reads in the window map info.  (this is
  basically stuff that is needed for each search, in theory at
  least).

  savant_retrieve: load in all the bias info.  This is presumably
  also a one-shot (like the stuff loaded by init_read).  Enter the
  main query loop for retrieve.

    get_query: parse the query, and break into types
      
      vectorize_buffer: actually encode the words, throw out stop
      words, stem, etc.  Call dvtree_increment to store the code in
      the DV_Tree "tree" for this vector, and return that tree.

      merge_dvtrees: merge different fieldtypes into a single docvec

    find_matches: figures out the term weight for each word/term.
    Remember best matches so you can print the terms later

      update_matches: called once for each unique word in the query.
      It goes through the supplied wordvec and adds appropriate
      similarity contributions to the appropriate slots in all_sims.

        sim_contrib: takes a single doc-ID/word-frequency pair from a
        retrieved wordvec and, using some other extenuating
        information, deciding how much it will contribute to that
        document's similarity rating.  Does the logs, chopping
        algorithm, & stuff like that.  Also applies bias info here.

    print_suggestions:  Print them out.

-------------------------------------------------------------
New system:

main: handle all the command-line args and initialize vars.  Load config info,
which includes a seperate retrieval template.  (same fieldtypes as indexing, different
templates & regexps.)

  command_loop: get the command (e.g. query, print, etc)
    get_query: read in the query itself into a GBuffer.  No processing yet.
    recognize_query: recognize the query type.  Like recognize_file, but with whole query.
    find_fields: find each field contents and put in a growbuffer.

    for each field:
      parser: run the parser procedure on the query, take the output (e.g. a docvec).

      rank_documents: given the parsed info, individual word
      similarity plugin, and multi-word sim. plugin rank each document
      and store in document order.  Keep track of the top K words that
      are most relevant for this field.

        single-word-metric: compute the relevance of a single word
        weight in a query to that word's weight in a document.

        multi-word-metric: given a vector of similarities produced by
        the single-word-metric, what's the similarity between two
        vectors.

    Once for the query:
      merge_field_rankings: given several document rankings, merge
      them to a single one using query biases.

      print_suggestions: print the titles/info for the top N
      documents.  Also print out the top K words that were most
      relevant for each suggested document being chosen.

*/
  
  if(SavantVerbose || SavantDebug) {
    print_menu();
  }
  override_query_biases = 0;
  for (i=0; i < MAX_NUMBER_FIELDS; i++) {
    handsetbiases[i] = 1;
  }
  while ((retrieve_command = get_command(&command_arg, command_argstring)) != QUIT_COMMAND) {
    switch (retrieve_command) {

    case QUERY_COMMAND:
      if (command_arg <= 0) command_arg = 5;
/*      if (command_arg > 999) command_arg = 999;*/
      strncpy_GBuffer(&query, "", 0);         /* clear from the last time */
      get_query (&query);
      queryInfo.documentText = &query;
      template = recognize_query (query, All_General_Templates);
      if ((template == NULL) || (template->action == REJECT_ACTION)) {
        break;   /* Shouldn't we print a "." or something? */
      }
      if (!override_query_biases) {
        set_querybiases(querybiases, template->fields);
      }
      find_fields(&queryInfo, template);
      filter_fields(&queryInfo, template);
      parse_fields(&queryInfo, template);

      if (SavantDebug) print_doc_info(&queryInfo, template);

      sorted_sims_array = rank_docs_for_fields (total_sims, 
						all_sims, 
						&queryInfo, 
						template, 
						All_General_Templates,
						rdi, 
						command_arg,
						(override_query_biases ?
						 handsetbiases : querybiases));

      print_top_docs(sorted_sims_array, command_arg, rdi);
      free(sorted_sims_array);
      cleanup_fields(&queryInfo, template);

      /* Clean it now so we don't have to later */
      resetDocSimsTotals(total_sims);   
      resetDocSims(all_sims);   

      break;

    case LOCRETRIEVE_COMMAND:
      if ((command_arg < 0) || (command_arg >= rdi->number_documents_total))
        printf("Document numbers range from 0 to %d\n", rdi->number_documents_total - 1);
      else {
        document_name_and_offsets(command_arg, rdi, docfilename, &doc_start, &doc_end);

        templateno = docloc_templateno (rdi, command_arg);
        for (i=0, templatelist = All_General_Templates; i < templateno; i++, templatelist = templatelist->next);

        printf("%d\n%d\n%s\n%s\n", doc_start, doc_end, docfilename, templatelist->template->printname);
        printf("%s\n", RETRIEVAL_SEPERATOR_STRING);
      }
      break;

    case RETRIEVE_COMMAND:
      print_document_contents (command_arg, rdi);
      break;

    case HELP_COMMAND:
      print_menu();
      break;

    case SET_BIAS_COMMAND:
      thisfield = get_field_from_allfields(All_Fields, command_argstring);
      if (thisfield == NULL) 
        printf("Can't find field named \"%s\"\n", command_argstring);
      else {
        handsetbiases[thisfield->typenum] = command_arg;
        if (SavantVerbose) { printf("Setting %s = %d\n", command_argstring, command_arg); }
      }
      break;

    case PRINT_BIASES:
      for (i=0; i < MAX_NUMBER_FIELDS; i++) {
        if (All_Fields->field[i] != NULL) 
          printf("%s: %d\n", All_Fields->field[i]->printname, handsetbiases[i]);
      }
      if (override_query_biases) 
        printf("Handset biases USED\n");
      else
        printf("Handset biases INACTIVE (using template biases instead)\n");
      break;

    case USE_HANDSET_BIASES:
      override_query_biases = 1;
      if (SavantVerbose) { printf("Handset biases USED\n"); }
      break;

    case USE_TEMPLATE_BIASES:
      override_query_biases = 0;
      if (SavantVerbose) { printf("Handset biases INACTIVE (using template biases instead)\n"); }
      break;
            
    case DB_INFO_COMMAND:
      printf("Version: %s, Number documents: %d\n", REMEM_VERSION_NUMBER, rdi->number_documents_total);
      break;

    case UNKNOWN_COMMAND:
      printf("Unknown command, type ? for help.\n");
      break;
    }
    fflush(stdout);

    if(SavantVerbose || SavantDebug) {
      printf("\nCommand: ");
    }
  }

  /* Deinitialize stuff (damn but I want a real OO language!) */
  for (i=0; i < MAX_NUMBER_FIELDS; i++) {
    free_GBuffer(queryInfo.fields[i]);
    free(queryInfo.fields[i]);
  }
  free(all_sims->listOfBuckets);
  free(total_sims->listOfBuckets);
  free(all_sims);
  free(total_sims);
  free(rdi);

  SavantError(0, "");
}


/*
void check_index_mod_for_reset (Retrieval_Database_Info *rdi) {
  struct stat statbuf;
  char doclocfile[512];

  doclocfile[0] = '\0';
  strcat(doclocfile, rdi->db_dir);
  strcat(doclocfile, "/");
  strcat(doclocfile, DOCLOC_FNAME);

  stat(doclocfile, &statbuf);
  if (statbuf->st_mtime != rdi->modtime) {   
    rdi->number_documents_total = number_documents(rdi->db_dir);
  }
}
*/
