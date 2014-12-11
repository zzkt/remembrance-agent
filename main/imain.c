/* imain.c: This is the main for Savant 2.0, now with regexp parsing.  It 
   does all the indexing for savant. */

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
#include "conftemplates.h"
#include "plugins.h"
#include <savant.h>
#include <savutil.h>

extern List_of_General_Templates *All_General_Templates;

void instructions(void);
void IndexError (int errcode, char *errstring);
char **source_from_file(char *filename);
int source_count(char *);

int SavantVerbose = 0, SavantDebug = 0, SavantFollowSymlinks = 0;
UserVars Config;       /* Initialized in load_config */

/* IndexError is Index's error handler.  SavantError will
   point to it.  All it does is print the error string
   and exit.  */


void instructions(void)  
{
  fprintf(stderr,"\nusage:\n");
  fprintf(stderr,"  ra-index [--version] [-v] [-d] [-s] <base-dir> <sources> [-e <excludees>]\n");
  fprintf(stderr,"  -v invokes savant indexing verbosely;  -d provides debugging messages; -s means to follow symbolic links. \n");
  fflush(stderr);
}


char **source_from_file(char *filename)
{
  int i=0, j=0, *source_sizes, nsources;
  char *str, c, b, **result, file_path[PATHLEN+1];
  FILE *ifp;

  if(filename[0] != '/') {

#ifdef HAVE_GETCWD
  getcwd(file_path, PATHLEN);
#else
  getwd(file_path);
#endif
  strcat(file_path, "/");
  strncat(file_path, filename, PATHLEN-strlen(file_path));
  }
  else
    strncpy(file_path, filename, PATHLEN);
  file_path[PATHLEN-1] = '\0';
  
  if((ifp = fopen(file_path,"r")) == NULL) {
    SavantError(ENOENT, "source_file: Unable to open file containing source filenames");
  }

  fclose (ifp);

  nsources = source_count(file_path);

  result = (char **) malloc ((nsources+1) * (sizeof(char *)));

  source_sizes = (int *) malloc (nsources * (sizeof(int)));

  for (i = 0; i<(nsources+1); i++) 
    result[i] = NULL;
  
  
  ifp = fopen(filename, "r");
  
  c = getc(ifp);
  
  while ((signed char) c != EOF){
    b = c;
    c = getc(ifp);
    if (isspace(c)) {
      if(isspace(b)) {
	continue;
      }
      else {
	source_sizes[j++] = ++i;
	i = 0;
      }
    }
    else {
      i++;
    }
  }
  
  for (i=0; i<nsources; i++) {
    result[i] = (char *) malloc (source_sizes[i]*(sizeof(char)) + 1);
  }

  free(source_sizes);
  
  rewind(ifp);
  c = getc(ifp);
  i=j=0;
  
  while ((signed char)c != EOF){
    b = c;
    c = getc(ifp);
    if (isspace(c)||(c == '\n')) {
      if(isspace(b)||(b == '\n')) {
	continue;
      }
      else {
	result[j][i++] = b;
	result[j++][i] = '\0';
	i = 0;
      }
    }
    else {
      if (isspace(b)) {
	continue;
      }
      else {
	result[j][i++] = b;
      }
    }
  }
  
  return result;
}

/* see /usr/src/linux-2.0.34/include/asm-i386/errno.h for errnos in linux */
void IndexError (int errcode, char *errstring)
{
  fprintf(stderr, "%s\n", errstring);
  exit(errcode);
}

int source_count(char *filename)
{
  FILE *ifp;
  char c;
  int i = 0;
  
  ifp = fopen(filename, "r");

  c = getc(ifp);

  while ((signed char) c != EOF){
    if ((c=='\n')||(isspace(c))){
      while (isspace(c)) {
	c = getc(ifp);
	}
      i++;
    }  
    c = getc(ifp);
  }

  fclose(ifp);
  return i;
}



/* 
The main path is one long calling chain till you get to
vectorize_buffer, which actually does the culling of stop-words,
stemming of words, and encoding into a DV_Tree (document vector tree).
Yenta I think enters this at savant_index (yes?).  When we're changing
the system around to not use templates, we'll have to start in
process_file and there will be some restructuring in all lower levels
(in particular, places where fields are talked about by name).

Calling structure:

Who calls who (templates / with .savantrc method)
   main: handle all the command-line args and initialize variables
-> pre_process:  process directories specified on command line
-> file_search:  recurse through directories, ignoring excludees and symbolic links
-> process_file: recognize a file type (ignore binaries & rejected filetypes), and set processed structure 
                 for all the documents and their fields within this file (start & end locations) by calling 
                 match_pattern 
-> savant_index: reopen the file, call do_vector on the each document specified in the processed structure,
                 and compute the title info (do_title).
-> do_vector:    Called once per document.  Calls vectorize_file on each field type, merges them together, 
                 write some info to disk (DLOFF_FILE, WMAP_FILE, and BIAS_FILE), and call save_dv.  If 
                 windowing is turned on, do the vectorize_file and save_dv multiple times, once for each 
                 window.
-> vectorize_file: actually read from the file (based on offsets provided by do_vector), and call 
                   vectorize_buffer on the read-in text.  Also keep track of where the windows are 
                   (in next_window) so we can be in the right place for next time.
-> vectorize_buffer: actually encode the words, throw out stop words, stem, etc.  Call dvtree_increment to 
                     store the code in the DV_Tree "tree" for this vector, and return that tree.


---------------------------------------

New calling structure:

main: handle all the command-line args and initialize variables
  -> get_filenames (pre_process):  process directories specified on command line.  Produce a linked-list of filenames.
     -> get_files_from_directory (file_search):  recurse through directories, ignoring excludees and symbolic links
  for each file:
     -> recognize_file: recognize a file's type (ignore binaries &
                rejected filetypes).
     for each doc:
        -> find_next_doc: get the start & end point of the next document, put in some structure
        -> find_fields: find each field's data in the document and put into a growbuffer.
        -> parser: run the parser procedure, take the output (e.g. docvecs).
        -> index-store-method: run the index-store routine to convert the parsed output into an internal format.
                               (e.g. from wordvecs to the sorted list of wordvecs).
     at the end, for each field-type we've got lying around:
        -> index-finalize-write: write out the various kinds of fields we've got.
        
*/
int main(int argc,
	  char *argv[])
{
  List_of_Filenames *lof = NULL;
  List_of_Filenames *current_filename = NULL;
  int i, j, sources_done=0, nsources, fsources_done=0, start=0, end=0, numDocsInFile=0, totalNumDocs=0;
  char *config_name=NULL, *input_filename = NULL, **sources, **excludees, *db_name=NULL, 
    db_dir[PATH_MAX+2], *short_name, ***file_sources;
  FILE *file;
  General_Template *template = NULL;
  List_of_General_Templates *lot = NULL;

  Doc_Info docInfo;                /* info on each document as it's processed */
  GBuffer *restDocumentText;    /* Remaining text in document */
  GBuffer *temp;
  
  /* Initialize stuff (damn but I want a real OO language!) */
  docInfo.documentText = (GBuffer *)malloc(sizeof(GBuffer));
  init_GBuffer(docInfo.documentText);
  for (i=0; i < MAX_NUMBER_FIELDS; i++) {
    docInfo.fields[i] = (GBuffer *)malloc(sizeof(GBuffer));
    init_GBuffer(docInfo.fields[i]);
  }
  restDocumentText = (GBuffer *)malloc(sizeof(GBuffer));
  init_GBuffer(restDocumentText);

  /* Set the error printing function to be IndexError */
  SetSavantError(&IndexError);

  sources = (char **) malloc (argc*sizeof(char *));           /* argc is an upper bound */
  excludees = (char **) malloc (argc*sizeof(char *));
  file_sources = (char ***) malloc (argc*(sizeof(char **)));

  for (i=0; i<argc; i++) 
    sources[i] = excludees[i] = NULL;

  for (i=0; i<argc; i++)
    file_sources[i] = NULL;

  j = 0;
  for (i=1; i < argc; i++)
    if (argv[i][0] == '-') 
      switch (argv[i][1]) {
      case 'c':
	if (config_name == NULL) {
	  config_name = argv[++i];
	}
	else {
	  instructions();
          SavantError(EINVAL, "");
	  /*exit(-1);*/
	}
	break;
      case 's':
        SavantFollowSymlinks = 1;
        break;
      case 'f':
	if(sources_done == 0) {
	  sources_done = 1;
	  j = 0;
	}
	else {
	  instructions();
	  SavantError(EINVAL, "");
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
        }
        SavantError(EINVAL, "");
        break;
      case 'e':
	if(fsources_done == 0 ) {
	  sources_done = 1;
	  fsources_done = 1;
	  j = 0;
	}
	else {
	  instructions();
	  SavantError(EINVAL, "");
	  /*exit(-1);*/
	}
	break;
      default:
	instructions();
	SavantError(EINVAL, "");
	/*exit(-1);*/
      }
    else if(db_name == NULL) { /* do the database name */
      db_name = argv[i];
    } 
    else {
      /* do sources and excludees */
      if (sources_done == 0) {
	sources[j++] = strdup(argv[i]);
      }
      else {
	if (fsources_done == 0){
	  file_sources[j++] = source_from_file(argv[i]);
	}
	else {
	  excludees[j++] = argv[i];
	}
      }
    }
  
  if(SavantVerbose || SavantDebug) {
    puts(REMEM_VERSION);
  }
  
  if (db_name == NULL) {
    instructions();
    SavantError(EINVAL, "");
    /*exit(-1);*/
  }
  
  if(db_name[0] != '/') {  
    /* if not absolute pathname, tack it onto cwd */
    /* (is there a better way to do this?) */
#ifdef HAVE_GETCWD
    getcwd(db_dir, PATH_MAX);
#else
    getwd(db_dir);
#endif
    strcat(db_dir, "/");
    strncat(db_dir, db_name, PATH_MAX + 1 - strlen(db_dir));
  }
  else { /* use the absolute pathname */
    strncpy(db_dir, db_name, PATH_MAX + 2);    
  }
  if (db_dir[strlen(db_dir)-1] != '/')
    strcat(db_dir,"/");

  db_dir[PATH_MAX+1] = '\0';
  
  load_config ();
 
  /* this is where the real work gets done */

  get_and_append_filenames(sources, excludees, &lof);   /* sources are disk sources */
  /* [GET THE FILE SOURCES HERE TOO] */

  for (current_filename = lof; current_filename != NULL; current_filename = current_filename->next) {
    if ((strncmp(current_filename->filename, "http://", 7) == 0) ||  
        (strncmp(current_filename->filename, "ftp://", 6) == 0)) 
      short_name = current_filename->filename;
    else 
      short_name = strrchr(current_filename->filename, '/') + 1;
    
    if((file = fopen(current_filename->filename,"r")) == NULL) {
      if(SavantVerbose) {
        printf("%s:  ", short_name);
        for(i=strlen(short_name); i<20; i++) {
          fputc(' ', stdout);
        }
        printf("cannot open file, ignoring.\n");
        fflush(stdout);
      }
      continue;
    }
    else {
      if (is_bin_file_p(current_filename->filename)) {
        if(SavantVerbose) {
          printf("  %s:", short_name);
          for(i=strlen(short_name); i<20; i++) {
            fputc(' ', stdout);
          }
          printf("not text, ignoring.\n");
          fflush(stdout);
        }
        fclose(file);
        continue;
      }
    }
    
    if(SavantVerbose) {
      printf("  %s:", short_name);
      for(i=strlen(short_name); i<30; i++) {
        fputc(' ', stdout);
      }
      fflush(stdout);
    }

    template = recognize_file(file, All_General_Templates);
    if (template != NULL) {
      if (SavantVerbose) {
        printf("%s: ", template->printname);
        fflush(stdout);
      }
    }
    else {
      if(SavantVerbose) {
        printf("not found, ignoring\n");
        fflush(stdout);
      }
      fclose(file);
      continue;
    }

    if (template->action == REJECT_ACTION) {
      if (SavantVerbose) printf("rejecting\n");
      fflush(stdout);
      fclose(file);
      continue;
    }

    rewind(file);          /* So we undo the reading that recognize did */
    strncpy_GBuffer(restDocumentText, "", 0);
    docInfo.filename = current_filename->filename;
    numDocsInFile = 0;

    while ((!feof(file)) || (restDocumentText->tail != 0)) {
      numDocsInFile++;
      totalNumDocs++;
      find_next_doc(restDocumentText, &docInfo, template, file);
      find_fields(&docInfo, template);
      filter_fields(&docInfo, template);
      parse_fields(&docInfo, template);
      
      if (SavantDebug) {
        printf("\n-------------------------NEXT DOC-------------------------\n");
        print_doc_info(&docInfo, template);
      }

      write_doc_info(&docInfo, template, db_dir, 0);
      index_store_fields(&docInfo, template, db_dir, 0);
      cleanup_fields(&docInfo, template);
    }
    if (SavantVerbose) printf("%d\n", numDocsInFile);

    fflush(stdout);
    fclose(file);
  }


  /* Clean up */
  if (SavantVerbose){
    printf("%d documents total, finalizing write....", totalNumDocs);
    fflush(stdout);
  }

  write_doc_info(NULL, NULL, db_dir, 1);     /* Close the doc & titles files */

  /* Finalize write for each template */
  for (lot = All_General_Templates; lot != NULL; lot = lot->next) {
    index_store_fields(NULL, lot->template, db_dir, 1);
  }

  /* Still need to free docInfo.parsedfields -- but not here */
/*
  for (i=0; i < MAX_NUMBER_FIELDS; i++) {
    free(docInfo.parsedfields[i]);
  }
*/
  free(sources);
  free_GBuffer(restDocumentText);
  free(restDocumentText);
  for(i=0; file_sources[i] != NULL; i++) {   /* file_sources are files containing a list of sources */
    get_and_append_filenames(file_sources[i], excludees, &lof);  
    free(file_sources[i]);
  }

  free(file_sources);

  SavantError(0, "");
  
}
