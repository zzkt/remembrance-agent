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

#include "regex.h"
#include "gbuf.h"
#include "conftemplates.h"
#include "parsedoc.h"
#include <sys/stat.h>
#include <unistd.h>

#define READ_BUFFER_AMOUNT 20000 /* num chars we read in one gulp */
#define MAX_DELIMIT_WINDOW 1023  /* Max window of characters we search for a doc delimiter */

/* Is this file a binary file? */
int is_bin_file_p(char *filename)
{
  /* For now, just read in a few K and see if 95% is 
     printable (there may be a better way, but this is fast) */
  char buf[2048];
  FILE *file;
  int i, max, printables=0;

  if((file = fopen(filename,"r")) == NULL) {
    sprintf(buf, "is_bin_file_p:  Cannot open file: %s", filename);
    SavantError(ENOENT, buf);
  }

  max=fread(&buf, sizeof(char), 2048, file);
  
  fclose(file);

  for(i=0;i<max;i++)
    if (isprint(buf[i]) || isspace(buf[i]))
      printables++;
  
  if((max > 0) && (100*printables/max > 95))
    return(0);
  else 
    return(1);
}

int string_present_p(char *string, char **strings)
{
  int i;
  for(i=0; strings[i] != NULL; i++) {
    if (strcmp(string, strings[i]) == 0)
      return(1);
  }
  return(0);
}

/* For each source directory/file (or file_source directory/file,
specified with a -f), expand out from the command line and call
file_search, which then indexes recursively all the files in all the
directories below that point.  */

void get_and_append_filenames(char *sources[], char *excludees[], List_of_Filenames **lof)
{
  int i, isurl=0;
  char cwd[PATH_MAX+2], command[MAX_STRING], *cur_dir, *temp_excl;
  List_of_Filenames *current_filename = NULL;
  
#ifdef HAVE_GETCWD
  getcwd(cwd, PATH_MAX);
#else
  getwd(cwd);
#endif
  strcat(cwd, "/");

  /* expand excludee pathnames */
  for(i=0; excludees[i] != NULL; i++) {
    if(!((strncmp(excludees[i], "http://", 7) == 0) || (strncmp(excludees[i], "ftp://", 6) == 0))) {
      if(excludees[i][0] != '/') {
	temp_excl = excludees[i];
	
	if ((excludees[i] = (char *)malloc(strlen(temp_excl) + strlen(cwd) + 2)) == NULL)
	  SavantError(ENOMEM, "Unable to malloc excludees in imain.c.");
	
	strcpy(excludees[i], cwd);
	strcat(excludees[i], temp_excl);
      }
      if (excludees[i][strlen(excludees[i])-1] == '/') {
	excludees[i][strlen(excludees[i])-1] = '\0';
      }
    }
  }

  /* assign each expanded pathname to cur_dir and find some files */
  for(i=0; sources[i] != NULL; i++) {

    if((strncmp(sources[i], "http://", 7) != 0) && (strncmp(sources[i], "ftp://", 6) != 0)) {
      /* It's not a URL -- put the filename or directory name into cannonical form */
      if(sources[i][0] == '/') {
        cur_dir = sources[i];
      }
      else {          /* It's a relative filename -- add cwd */
        if ((cur_dir = (char *)malloc(strlen(sources[i]) + strlen(cwd) + 2)) == NULL)
          SavantError(ENOMEM, "Unable to malloc cur_dir in parsedoc.c.");        
        strcpy(cur_dir, cwd);
        strcat(cur_dir, sources[i]);
      }
      if (sources[i][strlen(sources[i])-1] == '/') {  /* Remove trailing "/" */
        sources[i][strlen(sources[i])-1] = '\0';
      }
    }

    /* Find end of the list of files given (lof), and append the result of get_files_from_directory */
    if (*lof == NULL) {
      *lof = get_files_from_directory(cur_dir, excludees);
    }
    else {
      for (current_filename = *lof; (current_filename->next != NULL); current_filename = current_filename->next);
      current_filename->next = get_files_from_directory(cur_dir, excludees);
    }
    
    /* Free up memory */
    if(cur_dir != sources[i]) {
      free(cur_dir);
    }
    free(sources[i]);
  }
}


/* Recursively go down directories specified, excluding
directories/files on the excludies list, and avoiding symbolic links.
When you get to a real file (as opposed to a directory), call
process_file.  file_search is called by pre_process. */
List_of_Filenames *get_files_from_directory(char *sourcename, char **excludees)
{
  int i, isurl=0;
  char filename[256], *shortname;
  struct stat buf;
  DIR *directory;
  savant_direct *entry;
  List_of_Filenames *list_of_filenames = NULL;
  List_of_Filenames *current_filename = NULL;

  /* initialize filename lists */
  list_of_filenames = (List_of_Filenames *) malloc (sizeof(List_of_Filenames));
  list_of_filenames->next = NULL;

  filename[sizeof(filename)-1] = '\0';

  if ((strncmp(sourcename, "http://", 7) == 0) || (strncmp(sourcename, "ftp://", 6)==0)) {
    /* It's a URL, so it it's not excluded then just return the URL itself */
    isurl=1;
    if(string_present_p(sourcename, excludees)) {
      if(SavantVerbose) {
	printf("Excluding %s.\n", sourcename);
	fflush(stdout);
      }
      free(list_of_filenames);
      return NULL;
    }
    else {
      list_of_filenames->next = NULL;
      list_of_filenames->is_url_p = 1;
      list_of_filenames->filename = (char *) malloc (sizeof(char) * (strlen(sourcename) + 1));
      strcpy(list_of_filenames->filename, sourcename);
      return list_of_filenames;
    }
  }
  else {  /* it's not a URL */
    if(string_present_p(sourcename, excludees)) {
      if(SavantVerbose) {
	printf("Excluding %s.\n", sourcename);
	fflush(stdout);
      }
      free(list_of_filenames);
      return NULL;
    }
    
    shortname = strrchr(sourcename, '/')+1;
    if((shortname[0] == '#') || /* ignore *~ and #*, possibly .* */ 
       (shortname[strlen(shortname)-1] == '~') ||
       (shortname[0] == '.' && !Config.index_dotfiles)) {
      if(SavantVerbose) {
        printf("  Ignoring %s.\n", shortname);
        fflush(stdout);
      }
      free(list_of_filenames);
      return NULL;
    }

    /*check to see if the directory/file is a symlink*/
    lstat(sourcename, &buf);
    if (!SavantFollowSymlinks && (S_ISLNK(buf.st_mode))) { /*ignore symbolic links*/
      if(SavantVerbose) {
        printf("  Ignoring symbolic link:  %s.\n", shortname);
        fflush(stdout);
      }
      free(list_of_filenames);
      return NULL;
    }

    if (NULL == (directory = opendir (sourcename))) {
      /* sourcename is a file, process it */
      list_of_filenames->next = NULL;
      list_of_filenames->is_url_p = 1;
      list_of_filenames->filename = (char *) malloc (sizeof(char) * (strlen(sourcename) + 1));
      strcpy(list_of_filenames->filename, sourcename);
      return list_of_filenames;
    }

    /* sourcename is a directory, recurse */
    /* (note that the first one returned is going to be a dummy) */
    if(SavantVerbose) {
      printf("Searching %s:\n", sourcename);
      fflush(stdout);
    }

    entry = readdir(directory);
    while (entry != NULL) {
      shortname = entry->d_name;
      if(strcmp(shortname,".") && strcmp(shortname,"..")) {
        strncpy(filename, sourcename, sizeof(filename)-2);
        filename[sizeof(filename)-1] = '\0';
        if (filename[strlen(filename)-1] != '/')
          strcat(filename, "/");
        strncat(filename, shortname, sizeof(filename)-1);
        
        /* find the end of the chain of filenames, and recursively append */
        for (current_filename = list_of_filenames; 
             (current_filename->next != NULL); 
             current_filename = current_filename->next);
        current_filename->next = get_files_from_directory (filename, excludees);
      }
      entry = readdir(directory);
    }
    /* Get rid of the dummy entry */
    current_filename = list_of_filenames;
    list_of_filenames = list_of_filenames->next;
    free(current_filename);
    
    if(SavantVerbose) {
      printf("Finished %s.\n", sourcename);
      fflush(stdout);
    }
    closedir(directory);
    
    return list_of_filenames;
  }
}


/* Recognize what kind of file this is.  Target is a string containing the 
   first several lines of the file.
   Find the first template that matches the current file. 
   Note: This only applies to INDEX_TYPE templates.  Ignore QUERY_TYPE.
*/

General_Template *recognize_file(FILE *file, List_of_General_Templates *current_template) {
  int start, end, max;
  char target[RECOGNIZE_LIMIT +1];

  max=fread(&target, sizeof(char), RECOGNIZE_LIMIT, file);
  target[max] = '\0';

  while(current_template != NULL) {
    if (current_template->template->templatetype == INDEX_TYPE) {
      regex_find(current_template->template->recognize, target, max, &start, &end);
      if(start >= 0)
        return current_template->template;
    }
    current_template = current_template->next;
  }
  return NULL;
}

/* Recognize what kind of query this is.  query is a GBuffer containing the query.
   Find the first template that matches it.. 
   Note: This only applies to QUERY_TYPE templates.  Ignore INDEX_TYPE.
*/

General_Template *recognize_query(GBuffer query, List_of_General_Templates *current_template) {
  int start, end, max;
  char *target;

  max = query.tail;
  target = strcast_GBuffer(&query);

  while(current_template != NULL) {
    if (current_template->template->templatetype == QUERY_TYPE) {
      regex_find(current_template->template->recognize, target, max, &start, &end);
      if(start >= 0)
        return current_template->template;
    }
    current_template = current_template->next;
  }
  return NULL;
}


/* Find the next doc in a file.  documentText is the text from this
   current doc.  target_template is the template for this particular
   file's type.  file_ptr is the pointer to the file in question.
   start will be the character offset in the file of the first
   character of the delimiter, and end will be the character offset of
   the character just before the dilimiter starting the next
   document. */

void find_next_doc(GBuffer *documentText,      /* The text read in from the document.  Modified to eat docs each run. */
                   Doc_Info *document,        /* we fill in this document into document->documentText */
                   General_Template *target_template, /* Which template is this file? */
                   FILE *file_ptr) {                  /* pointer to the doc file */
  static DB_UINT docnum = 0;     /* The next docnum to assign, starting at 0 */
  int first_start, first_end, second_start, second_end, i, j, dt_len, dt_interim_len, window_start;
  char *dt, *dt_interim;
  char buffer[READ_BUFFER_AMOUNT + 1];
  size_t numbytes;   /* num bytes actually read */
  long fposition;

  /* Set & increment the static document number (find_next_doc is called once per document, so it's easy */
  document->docnum = docnum++;

  /* load the contents of the documentText into dt */
  dt = strcast_GBuffer(documentText);

  /* find the first delimiter */
  regex_find(target_template->delimiter, dt, documentText->tail, &first_start, &first_end);

  /*
    if there is no delimiter in however much is initially passed to
    find_next_doc in documentText->value, then first_start will be set to -1
    by regex_find, and so we have to read in some more from the file,
    and then search again for the delimiter.  if at any time we hit an
    eof, then just set the whole file to be the document and return.
  */
  while (first_start == -1) {
    if (feof(file_ptr) || (documentText->tail > MAX_DOC_LENGTH)) {  /* It's all one file, so just copy it all in */
      strncpy_GBuffer(document->documentText, strcast_GBuffer(documentText), strlen(strcast_GBuffer(documentText)));
      fposition = ftell(file_ptr);
      if (fposition == -1) SavantError(errno, "Error with ftell in find_next_doc");
      document->doc_end = fposition;
      document->doc_start = fposition - document->documentText->tail;

      if (!feof(file_ptr) && SavantVerbose) {
        printf("<WARNING: document too long, stopped indexing this file at %d characters...>", MAX_DOC_LENGTH);
        /* Force EOF */
        fseek(file_ptr, 0, SEEK_END);
        numbytes = fread((void *)buffer, sizeof(char), 1, file_ptr);
        fposition = ftell(file_ptr);
        if (fposition == -1) SavantError(errno, "Error with ftell in find_next_doc");
        document->doc_end = fposition;
      }

      /* Clear documentText so we don't think there's more to read */
      strncpy_GBuffer(documentText, "", 0);
      return;
    }
    if (ferror(file_ptr)) {
      SavantError(ferror(file_ptr), "Error reading file in find_next_doc");
    }

    numbytes = fread((void *)buffer, sizeof(char), READ_BUFFER_AMOUNT, file_ptr);
    buffer[numbytes] = '\0';

    /* I'm pretty sure I'm relying on C string manipulation somewhere in GBuffers, so
       any nulls in the middle of the string screw things up. The right thing to do is
       to not mind having nulls in the middle since GBuffers have the length anyway, 
       but in the meantime I'm just replacing nulls with ^A's */
    while (strlen(buffer) < numbytes) {
      buffer[strlen(buffer)]='\1';
    }

    strncat_GBuffer(documentText, buffer, numbytes);

    dt = strcast_GBuffer(documentText);      /* Not necessary, but a good reminder */

    /* Look again, starting with the last MAX_DELIMIT_WINDOW characters we just tried*/
    dt_len = documentText->tail;
    window_start = dt_len - numbytes - MAX_DELIMIT_WINDOW;
    if (window_start < 0) {
      window_start = 0;
    }

/*
    if ((dt_len - numbytes) > MAX_DELIMIT_WINDOW) {
    dt_interim = &dt[(dt_len - MAX_DELIMIT_WINDOW - numbytes)];
    dt_interim_len = MAX_DELIMIT_WINDOW + numbytes;
    }
    else {
    dt_interim = dt;
    dt_interim_len = dt_len;
    }
*/
    regex_find(target_template->delimiter, &dt[window_start], (dt_len - window_start), &first_start, &first_end);
    if (first_start != -1) {
      first_start = first_start + window_start;
    }
    if (first_end != -1) {
      first_end = first_end + window_start;
    }
  }

  /* find the second delimiter -- we want to search the area that starts
    just *after* the end of the first delimiter.  */
  regex_find(target_template->delimiter, &(dt[first_end + 1]), 
             documentText->tail - first_end, &second_start, &second_end);
  /* second_start and second_end should be relative to the start of dt */
  if (second_start != -1) {
    second_start += (first_end + 1);
    second_end += (first_end + 1);
  }

  /* if the second delimiter isn't there, then we need to read in more.
     if at any point we hit an eof, then that means we just want the
     document from the beginning of the first delimiter to the end of the
     file (i.e. we include the delimiter).  */
  while (second_start == -1) {
    if (feof(file_ptr) || (documentText->tail > MAX_DOC_LENGTH)) {

      /* Return from first_start to eof */
      strncpy_GBuffer(document->documentText,
                      (char *)(dt + first_start), 
                      (documentText->tail - first_start));
      fposition = ftell(file_ptr);
      if (fposition == -1) {
        SavantError(errno, "Error with fseek in find_next_doc");
      }
      document->doc_end = fposition;
      document->doc_start = fposition - document->documentText->tail;

      if (!feof(file_ptr) && SavantVerbose) {
        printf("<WARNING: document too long, stopped indexing this file at %d characters...>", MAX_DOC_LENGTH);
        /* Force EOF */
        fseek(file_ptr, 0, SEEK_END);
        numbytes = fread((void *)buffer, sizeof(char), 1, file_ptr);
        fposition = ftell(file_ptr);
        if (fposition == -1) SavantError(errno, "Error with ftell in find_next_doc");
        document->doc_end = fposition;
      }
      
      /* Clear documentText so we don't think there's more to read */
      strncpy_GBuffer(documentText, "", 0);
      return;
    }

    numbytes = fread((void *)buffer, sizeof(char), READ_BUFFER_AMOUNT, file_ptr);
    buffer[numbytes] = '\0';
    strncat_GBuffer(documentText, buffer, numbytes);
    dt = strcast_GBuffer(documentText);

    if (ferror(file_ptr)) {
      SavantError(ferror(file_ptr), "Error reading file in find_next_doc");
    }

    /* Look again, starting with the last MAX_DELIMIT_WINDOW characters we just tried*/
    dt_len = documentText->tail;
    window_start = dt_len - numbytes - MAX_DELIMIT_WINDOW;
    if (window_start < (first_end + 1)) {
      window_start = first_end + 1;
    }

/*
  if ((dt_len - numbytes) > MAX_DELIMIT_WINDOW) {
  dt_interim = &dt[(dt_len - MAX_DELIMIT_WINDOW - numbytes)];
  dt_interim_len = MAX_DELIMIT_WINDOW + numbytes;
  }
  else {
  dt_interim = &dt[first_end + 1];
  dt_interim_len = dt_len - (first_end + 1);
  }
*/

    regex_find(target_template->delimiter, &dt[window_start],
               (dt_len - window_start), &second_start, &second_end);

    if (second_start != -1) {
      second_start = second_start + window_start;
    }
    if (second_end != -1) {
      second_end = second_end + window_start;
    }
  }

  /* if we get to this point without hitting any of the return markers, then
     just return pointers to the start of the first delimiter and the point
     just *before* the start of the second delimiter.  
     Chop out this document in documentText, so we won't read it next round. */

  /* NOTE: second_start & second_end are NOT relative to first_end+1, but are */
  /* rather relative to the start of dt */
  strncpy_GBuffer(document->documentText,
                  (char *)(dt + first_start), 
                  (second_start - first_start));
  strnchop_GBuffer(documentText, second_start - first_start);

  fposition = ftell(file_ptr);
  if (fposition == -1) {
    SavantError(errno, "Error with fseek in find_next_doc");
  }
  document->doc_end = fposition - documentText->tail + first_start;
  document->doc_start = document->doc_end - document->documentText->tail;
  return;
}


void find_fields(Doc_Info *docInfo, General_Template *target_template) {
  char *dt, *errptr, space_delimiter[2];
  int vector[256], error_offset, group_begin, group_end, i=0, errcode;
  size_t sizedt;
  char errortext[256];
  GBuffer good_bits;
  pcre *pattern;

  init_GBuffer(&good_bits);
  strcpy(space_delimiter, " ");

  /* each template has an associated set of fields.  loop through each
     field, pulling them out of the documentText.  */
  for (i=0; (i<MAX_NUMBER_FIELDS) && (target_template->fields[i] != NULL); i++) {

    strncpy_GBuffer(&good_bits, "", 0);   /* clear out the good bits */

    /* compile the id_regex */
/* The version for pcre-1.09 
    if (NULL == (pattern = pcre_compile(target_template->fields[i]->id_regexp, 
                                        PCRE_DOTALL, (const char **)&errptr, &error_offset))) {
*/
    if (NULL == (pattern = pcre_compile(target_template->fields[i]->id_regexp, 
                                        PCRE_DOTALL, (const char **)&errptr, &error_offset, NULL))) {
      sprintf(errortext, "find_and_index_fields: pcre error %s at location %d",
              errptr, error_offset);
      SavantError(ENOEXEC, errortext);
    }

    dt = strcast_GBuffer(docInfo->documentText);
    sizedt = strlen(dt);

    /* loop while there are still matches... */
/* For pcre-1.09 
    while ((sizedt > 0) && (errcode = pcre_exec(pattern, NULL, dt, sizedt, 0, vector, 256)) > 0) {
*/
    while ((sizedt > 0) && (errcode = pcre_exec(pattern, NULL, dt, sizedt, 0, 0, vector, 256)) > 0) {
      /* see pcre documentation for info on how vector works
	 but suffice it to say that group_begin and group end
	 will contain offsets to the good bits. */

      group_begin = vector[2 * target_template->fields[i]->id_index];
      group_end = vector[2 * target_template->fields[i]->id_index + 1];

/* It's possible to match the pattern, but not this particular field.
   If so, just go on to the next one. */
      if ((group_begin == -1) && (group_end == -1)) {
	break;
      }
      if ((group_begin > docInfo->documentText->size) || 
          (group_end > docInfo->documentText->size)) {
        sprintf(errortext, "find_fields: vector out of range for finding fielddata: %s (is id_index correct?)",
                target_template->fields[i]->field->printname);
        SavantError(ENOEXEC, errortext);
      }


      /* put the string described by the group_begin and group_end
	 offsets into a GrowBuffer called good_bits.  Delimit with a CR if
         there's already text in it. */
      if (strlen(strcast_GBuffer(&good_bits)) > 0) 
        strncat_GBuffer(&good_bits, "\n", 1);
      strncat_GBuffer(&good_bits, &(dt[group_begin]), group_end - group_begin);

      /* move the head of the string we are examining to the *end of
         the last group found */
      dt = dt + (group_end * sizeof(char));
      sizedt -= (group_end * sizeof(char)) ;
    }

    /* now dump all the good_bits found into the docinfo structure */
    strncpy_GBuffer (docInfo->fields[i], strcast_GBuffer(&good_bits), good_bits.size);
    free(pattern);
  }

  free_GBuffer(&good_bits);
  return;
} 


void filter_fields(Doc_Info *docInfo, General_Template *target_template) {
  char errortext[256];
  int i;

  for (i=0; ((i<MAX_NUMBER_FIELDS) && (target_template->fields[i] != NULL)); i++) {
    if (SavantDebug) printf("filter_fields: i = %d, docnum = %d\n", i, docInfo->docnum);
    regex_filter(target_template->fields[i]->filter_regexp, docInfo->fields[i]); 
  }
}

void cleanup_fields(Doc_Info *docInfo, General_Template *target_template) {
  int i;
  /* The cleanup function */
  void (*cleanup_parsed)(void *parseddata) = NULL;   

  for (i=0; ((i<MAX_NUMBER_FIELDS) && (target_template->fields[i] != NULL)); 
       i++) {
    cleanup_parsed = target_template->fields[i]->field->cleanup_parsed;
    if (cleanup_parsed != NULL) {
      cleanup_parsed((void *)(docInfo->parsedfields[i]));
    }
  }
}

void parse_fields(Doc_Info *docInfo, General_Template *target_template) {
  int i;
  /* The parser function */
  void *(*parser)(char *fielddata, void *self, DB_UINT docnum) = NULL;   

  for (i=0; ((i<MAX_NUMBER_FIELDS) && (target_template->fields[i] != NULL)); 
       i++) {
      parser = target_template->fields[i]->field->parser;
      if (parser != NULL) {
        docInfo->parsedfields[i] = 
	  parser((void *)(strcast_GBuffer(docInfo->fields[i])),
                                          target_template->fields[i]->field,
                                          docInfo->docnum);
      }
      else {
        docInfo->parsedfields[i] = NULL;
      }
  }
}

/* Write the index info to disk.  If final_write_p is 0, close up the files.  
   WARNING: This frees docInfo->parsedfields[i] as it goes. 
*/
void index_store_fields(Doc_Info *docInfo, General_Template *target_template, 
			char *dbdir, int final_write_p) {
  int i;
  void (*index_store)(void *parsedata, char *dbdir, int last_write_p) = NULL;

  for (i=0; ((i<MAX_NUMBER_FIELDS) && 
	     (target_template->fields[i] != NULL)); i++) {
      index_store = target_template->fields[i]->field->index_store;
      if (index_store != NULL) {
        if (docInfo != NULL) {
          index_store(docInfo->parsedfields[i], dbdir, final_write_p);
        }
        else {
          index_store(NULL, dbdir, final_write_p);
        }
      }
  }
}


/* Return the template number for a particular document number.  Get
   the info from cache.  The whole cache is loaded from the DLOFFS
   file in one load, to avoid multiple seeks.  This is memory
   intensive (30 * num docs bytes total), so we may have to do this in
   chunks if it becomes a problem */

DB_INT docloc_templateno (Retrieval_Database_Info *rdi, DB_UINT docnum) {
  static DB_INT *template_number_cache = NULL;
  FILE *DLOFFS_FILE = NULL;
  DB_INT *template_precache_storage = NULL;
  int i;

  if (template_number_cache == NULL) {
    DLOFFS_FILE = open_or_die(rdi->db_dir, DLOFFS_FNAME, "r");
    template_number_cache = (DB_INT *)malloc(sizeof(DB_INT) * rdi->number_documents_total);
    template_precache_storage = (DB_INT *)malloc(sizeof(DB_INT) * rdi->number_documents_total * 5);
    if ((template_precache_storage == NULL) || (template_number_cache == NULL)) {
      SavantError(ENOMEM, "Unable to malloc docloc_templateno cache in parsedoc.c.");
    }
    fread_big(template_precache_storage, sizeof(DB_INT), rdi->number_documents_total * 5, DLOFFS_FILE);
    for (i=0; i < rdi->number_documents_total; i++) {
      template_number_cache[i] = template_precache_storage[5 * i + 4];
    }
    free(template_precache_storage);
    fclose(DLOFFS_FILE);
  }
  return(template_number_cache[docnum]);
}



/* Write the document position info (doc_start and doc_end), doc filename, and docloc offset
   info to file.  This is the same regardless of file type or fields.  Also write title info,
   which is dependent on the file template.

   File format:
      doc_locs (DOCLOC_OFFS): ascii string representation of full
              expanded filename, one line per unique file, in document order.

      docloc_offs (DLOFFS_FILE): Four ints per document.
              (DB_INT)   (DB_INT)    (DB_INT)   (DB_INT)   (DB_INT)
              offset     end_offset  doc_start  doc_end    template number   
              (repeat)

              offset = file offset into doc_locs for this file.  We
              can do multiple docs with the same offset, since
              multiple docs can be in one file.

              end_offset = file offset into doc_locs for the end of the filename.

              doc_start = number of characters from the start of the
              file that this doc starts.  Beginning of file = 0.

              doc_end = number of characters from the start of the
              file that this doc ends.

              template number = template typenum (also = the order in All_General_Templates)

      titles: plaintext summary of a document.  The contents are
      template-specific.

      title_offs: offsets into the titles file.  One int per doc, for the start of the title.

              (DB_INT)
              offset-low   */
void write_doc_info(Doc_Info *docInfo, General_Template *target_template, char *db_dir, int final_write_p) {
  static FILE *DOCLOC_FILE = NULL;
  static FILE *DLOFFS_FILE = NULL;
  static FILE *TITLES_FILE = NULL;
  static FILE *TOFFS_FILE = NULL;
  static char previous_filename[PATH_MAX] = "";
  static long dl_fpos = 0;
  DB_INT dl_fpos_writeme = 0;
  DB_INT dl_fpos_end = 0;
  
  int typenum;
  char *titlearray[MAX_NUMBER_FIELDS];
  int titlelengtharray[MAX_NUMBER_FIELDS];
  enum Title_Defaults_Type titledefaultsarray[MAX_NUMBER_FIELDS];
  char errortext[256];
  GBuffer *fieldtext = NULL;
  int i;
  char *titlestring;
  DB_INT titles_fpos = 0;
  struct stat statbuf;
  struct passwd *passbuf;

  if (DOCLOC_FILE == NULL) DOCLOC_FILE = open_or_die(db_dir, DOCLOC_FNAME, "w");
  if (DLOFFS_FILE == NULL) DLOFFS_FILE = open_or_die(db_dir, DLOFFS_FNAME, "w");
  if (TITLES_FILE == NULL) TITLES_FILE = open_or_die(db_dir, TITLES_FNAME, "w");
  if (TOFFS_FILE == NULL)  TOFFS_FILE =  open_or_die(db_dir, TOFFS_FNAME, "w");

  if (docInfo != NULL) {

    /* If we've got a new file, update DOCLOC_FILE & our offset into it */
    if (strcmp(previous_filename, docInfo->filename)) {        
      strcpy(previous_filename, docInfo->filename);
      
      dl_fpos = ftell(DOCLOC_FILE);
      if (dl_fpos == -1) {
        sprintf(errortext, "write_doc_info: error %d doing ftell on file %s%s", errno, db_dir, DOCLOC_FILE);
        SavantError(EIO, errortext);
      }
      
      fprintf(DOCLOC_FILE, "%s\n", docInfo->filename);
    }
    
    dl_fpos_writeme = (DB_INT)dl_fpos;
    dl_fpos_end = dl_fpos_writeme + strlen(docInfo->filename);
    fwrite_big(&(dl_fpos_writeme), sizeof(DB_INT), 1, DLOFFS_FILE);
    fwrite_big(&(dl_fpos_end), sizeof(DB_INT), 1, DLOFFS_FILE);
    fwrite_big(&(docInfo->doc_start), sizeof(DB_UINT), 1, DLOFFS_FILE);
    fwrite_big(&(docInfo->doc_end), sizeof(DB_UINT), 1, DLOFFS_FILE);
    fwrite_big(&(target_template->typenum), sizeof(DB_INT), 1, DLOFFS_FILE);
    
    /* Handle Titles stuff.  This is a placeholder right now. */
    titlestring = (char *)malloc(sizeof(char) * MAX_NUMBER_FIELDS * 
				 Config.source_field_width + 1);
    titlestring[0] = '\0';

    for (i=0; i<MAX_NUMBER_FIELDS; i++) {
      titlearray[i] = NULL;
      titlelengtharray[i] = 0;
    }
    for (i=0; (i<MAX_NUMBER_FIELDS && target_template->fields[i] != NULL); i++) {
      typenum = target_template->fields[i]->field->typenum;
      titlearray[typenum] = strcast_GBuffer(docInfo->fields[i]);
      titlelengtharray[typenum] = target_template->fields[i]->title_length;
      titledefaultsarray[typenum] = target_template->fields[i]->field->titleDefault;
    }
    for (i=0; i<MAX_NUMBER_FIELDS; i++) {
      if (titlelengtharray[i] > 0) {
        if (strlen(titlearray[i]) > 0) {
          strncat(titlestring, titlearray[i], titlelengtharray[i]);
        }
        else if (titledefaultsarray[i] == FILENAME_TITLE) {
          if (rindex(docInfo->filename, '/') != NULL) 
            strncat(titlestring, rindex(docInfo->filename, '/')+1, titlelengtharray[i]);
          else 
            strncat(titlestring, docInfo->filename, titlelengtharray[i]);
        }
        else if (titledefaultsarray[i] == OWNER_TITLE) {
          stat(docInfo->filename, &statbuf);
          passbuf = getpwuid(statbuf.st_uid);
          if(passbuf != NULL) {
            strncat(titlestring, passbuf->pw_name, titlelengtharray[i]);
          } 
        } 
        else if (titledefaultsarray[i] == MODTIME_TITLE) {
          stat(docInfo->filename, &statbuf);
          strncat(titlestring, ctime(&statbuf.st_mtime), titlelengtharray[i]);
        }
      }  
      strcat(titlestring, "|");
    }

    /* Include the filename in the title sequence, at the end */
    if (rindex(docInfo->filename, '/') != NULL) 
      strncat(titlestring, rindex(docInfo->filename, '/')+1, titlelengtharray[i]);
    else 
      strncat(titlestring, docInfo->filename, titlelengtharray[i]);
    strcat(titlestring, "|");

    for (i=0; titlestring[i] != '\0'; i++) {      /* Get rid of CR/LF */
      if (titlestring[i] == '\n') {
        titlestring[i] = ' ';
      }
    }
    
    titles_fpos = (DB_INT)ftell(TITLES_FILE);
    if (titles_fpos == -1) {
      sprintf(errortext, "write_doc_info: error %d doing ftell on file %s%s", 
	      errno, db_dir, TITLES_FILE);
      SavantError(EIO, errortext);
    }
    fprintf(TITLES_FILE, "%s\n", titlestring);
    fwrite_big(&(titles_fpos), sizeof(DB_INT), 1, TOFFS_FILE);

    /* Not sure why I have to do this -- might be because of the
       static storage.  Purify compiains if I don't though. */
    
    free(titlestring);
  }

  if (final_write_p) {
    if (DOCLOC_FILE != NULL) fclose(DOCLOC_FILE);
    if (DLOFFS_FILE != NULL) fclose(DLOFFS_FILE);
    if (TITLES_FILE != NULL) fclose(TITLES_FILE);
    if (TOFFS_FILE != NULL) fclose(TOFFS_FILE);

    DOCLOC_FILE = DLOFFS_FILE = TITLES_FILE = TOFFS_FILE = NULL;
  }
}

/* Print parsed document info (mainly for debugging purposes) */
void print_doc_info (Doc_Info *docInfo, General_Template *template) {
  GBuffer *deparsed = NULL;
  int i;

  printf("Template Type: %s\n", template->printname);
  if (template->templatetype == INDEX_TYPE) {
    printf("Document #%d, file %s, char position %d to %d\n\n", 
           docInfo->docnum, docInfo->filename, docInfo->doc_start, docInfo->doc_end);
  }
  for (i=0; (i<MAX_NUMBER_FIELDS) && (template->fields[i] != NULL); i++) {
    printf("\n********* Field %s:\n%s\n",  
           template->fields[i]->field->printname,
           strcast_GBuffer(docInfo->fields[i]));
    if ((docInfo->parsedfields[i] != NULL) && (template->fields[i]->field->deparser != NULL)) {
      deparsed = template->fields[i]->field->deparser(docInfo->parsedfields[i], 
                                                      (void *)(template->fields[i]->field));
      printf("Deparsed:\n%s\n", strcast_GBuffer(deparsed));
      fflush(stdout);
      free_GBuffer(deparsed);
      free(deparsed);
    }
  }
}

/* Comparison function for top_contributors (for qsort) 
   (sort high-to-low) */
int top_contributors_cmp(Top_Contributors *tl1, Top_Contributors *tl2) {
  if (tl1->sim > tl2->sim) 
    return(-1);
  if (tl1->sim < tl2->sim) 
    return(1);
  return(0);
}
    
int top_contributors_cmp_qsort(const void *tl1, const void *tl2) {
  return(top_contributors_cmp((Top_Contributors *)tl1, 
                              (Top_Contributors *)tl2));
}


/* Update a top_contributors with a potential replacement */
void add_potential_top_contributor (Top_Contributors *topList, float additional, char *printname) {
  int i, minIndex=0;
  float minSim = 0.0;
  for (i=0; ((i < NUMBER_CONTRIBUTORS_TRACKED) &&
             (topList[i].sim > 0.0)); i++) {
    if ((i==0) || (minSim > topList[i].sim)) {
      minSim = topList[i].sim;
      minIndex = i;
    }
  }
  if (i < NUMBER_CONTRIBUTORS_TRACKED) {
    minIndex = i;
    minSim = 0.0;
  }
  if (minSim < additional) {   /* We've got someone to replace */
    topList[minIndex].sim = additional;
    strncpy(topList[minIndex].printword, printname, PRINTWORD_LENGTH);
  }
}

/* Add additional similarity to a Doc_Sim, and update the top_contributors list
   if this is a big contributer */
void add_additional_to_doc_sim (Doc_Sim *thisSim, float additional, char *printname) {
  thisSim->sim += additional;
  add_potential_top_contributor (thisSim->top_contributors, additional, printname);
}

/* Add additional similarity to a Doc_Sim, and update the top_contributors list
   if this is a big contributer */
void add_maximum_to_doc_sim (Doc_Sim *thisSim, float newsim, char *printname) {
  if (thisSim->sim < newsim) {
    thisSim->sim = newsim;
    add_potential_top_contributor (thisSim->top_contributors, newsim, printname);
  }
}

/* Merge two top contributor lists, giving the best hits of both.  Modifies list #1 */
void merge_top_contributors (Top_Contributors *tc1, Top_Contributors *tc2) {
  int i;
  for (i=0; i < NUMBER_CONTRIBUTORS_TRACKED; i++) {
    add_potential_top_contributor (tc1, tc2[i].sim, tc2[i].printword);
  }
}
