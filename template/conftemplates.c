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
#include "plugins.h"

/* Function definitions*/

/* these are shortcut macros to make definitions easier down below */
#define atfi(arg1, arg2, arg3, arg4, arg5) add_template_field_info(current_template, arg1, arg2, arg3, arg4, arg5)
#define atfn(arg1, arg2, arg3, arg4, arg5, arg6) add_template_field_name(current_template, arg1, arg2, arg3, arg4, arg5, arg6)
#define caf(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) create_and_add_field(All_Fields, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
#define attl add_template_to_list(All_General_Templates, current_template);
/* Global variables

   All_General_Templates is *the* List_of_General_Templates which contains all the
   templates in the system.  

   All_Fields contain all the fields in the system.
*/
List_of_General_Templates *All_General_Templates;
Array_of_Field_Types *All_Fields;

/* Pull a field out by name instead of index */
Field *get_field_from_allfields(Array_of_Field_Types *aft, char *pname) {
  int i;
  for (i=0; i < aft->num_fields; i++) {
    if (strcmp(aft->field[i]->printname, pname) == 0) {
      return(aft->field[i]);
    }
  }
  return(NULL);
}

/* Create a new template, with an empty Template_Field_Info array.  
   Make sure to strcpy all strings into malloced buffers */
General_Template *create_template (char *printname, char *recognize, char *delimiter, enum Action_Types action,
                                   enum Template_Types templatetype) {
  int i;
  General_Template *result;

  result = (General_Template *) malloc (sizeof(General_Template));

  result->printname = strdup(printname);

  if (recognize != NULL) 
    result->recognize = strdup(recognize);
  else
    result->recognize = NULL;

  if (delimiter != NULL)
    result->delimiter = strdup(delimiter);
  else
    result->delimiter = NULL;

  result->action = action;
  result->templatetype = templatetype;
  result->fields = (Template_Field_Info **) malloc(MAX_NUMBER_FIELDS * sizeof(Template_Field_Info *));

  for (i=0; i<MAX_NUMBER_FIELDS; i++) {
    result->fields[i] = NULL;
  }

  return result;
}


/* Print configuration (for debugging) */
void print_config () {
  int i;
  List_of_General_Templates *t;
  printf("All_Fields: \n");
  for (i=0;i > All_Fields->num_fields; i++) {
    print_field(All_Fields->field[i]);
  }
  printf("All_General_Templates: \n");
  for (t=All_General_Templates; t != NULL; t = t->next) {
    print_template(t->template);
  }
}

void print_field(Field *f) {
  printf ("...%s\n", f->printname);
}

void print_template(General_Template *t) {
  int i;
  Template_Field_Info **f;
  printf ("...%s\n", t->printname);
  printf ("......REC: %s\n", t->recognize);
  printf ("......DEL: %s\n", t->delimiter);
  printf ("......FIELDS: \n");
  for (i=0, f=t->fields; (i < MAX_NUMBER_FIELDS) && (f[i] != NULL); i++) {
    print_template_field_info(f[i]);
  }
}

void print_template_field_info(Template_Field_Info *tfi) {
  printf (".........FIELD: %s\n", tfi->field->printname);
  printf ("............IDRXP: \"%s\"\n", tfi->id_regexp);
  printf ("............IDIND: %d\n", tfi->id_index);
  printf ("............BIAS:  %d\n", tfi->bias);
}


/* This returns a template field info, given a template to search
and the printname for that template.  Returns NULL if not found. */
Template_Field_Info *tfi_from_name (General_Template *template, 
				      char *fieldname) 
{
  char errorstring[256];
  int i=0;
  Template_Field_Info *current_info;
  while ((template->fields[i] != NULL) && 
         (strcmp(template->fields[i]->field->printname, fieldname))) {
    i++;

    if (i>=MAX_NUMBER_FIELDS) {
      sprintf(errorstring, "maximum number of fields already reached");
      SavantError (EOVERFLOW, errorstring);
    }
  }
  return(template->fields[i]);
}

/* This returns a field, given an array to search and the printname
for that template.  Returns NULL if not found.  Pulls fields from the
global "All_Fields". */

Field *field_from_name (char *fieldname) 
{
  int i;
  for (i=0; i < All_Fields->num_fields; i++) {
    if (!strcmp(All_Fields->field[i]->printname, fieldname)) {
      return (All_Fields->field[i]);
    }
  }
  return NULL;
}



/* Add a new template field info line, given a field name 
   it's just a wrapper, really*/
void add_template_field_name (General_Template *template,
			      char *fieldname,
                              char *id_regexp, 
                              int id_index, 
                              char **filter_regexp, 
                              int bias,
                              int title_length)
{
  char errorstring[256];
  Field *field;
  field = field_from_name (fieldname);
  if (field == NULL) {
      sprintf(errorstring, "add_template_field_name: No field %s found in All_Fields", fieldname);
      SavantError (EINVAL, errorstring);
  }
  add_template_field_info (template,
                           field,
			   id_regexp, id_index, filter_regexp, bias, title_length);
}

/* Add a new Template_Field_Info line to a template */
void add_template_field_info (General_Template *template, 
                              Field *field,   
                              char *i_regexp, 
                              int i_index, 
                              char **f_regexp, 
                              int b,
                              int title_length)
{
  int i=0;
  Template_Field_Info *current_info;
  char errorstring[256];

  /*find the first open spot to insert the field info*/
  while (template->fields[i] != NULL) {
    i++;
    /*safety check*/
    if (i>=MAX_NUMBER_FIELDS) {
      sprintf(errorstring, "maximum number of fields already reached");
      SavantError (EOVERFLOW, errorstring);
    }
  }
  
  current_info = (Template_Field_Info *) malloc(sizeof (Template_Field_Info));

  current_info->field = field;
  current_info->id_regexp = i_regexp;
  current_info->id_index = i_index;
  current_info->filter_regexp = f_regexp;
  current_info->bias = b;
  current_info->title_length = title_length;

  template->fields[i] = current_info;
}

/* Take a template and tack it onto the end of a pre-existing list of templates*/
void add_template_to_list(List_of_General_Templates *list, General_Template *template)
{
  List_of_General_Templates *tail;

  if (list->template == NULL){
    template->typenum = 0;
    list->template = template;
    list->next = NULL;
    return;
  }
  else {
    for (tail=list; tail->next != NULL; tail = tail->next);
    template->typenum = (tail->template->typenum + 1);
    tail->next = (List_of_General_Templates *)(List_of_General_Templates *)malloc(sizeof(List_of_General_Templates));
    tail->next->template = template;
    tail->next->next = NULL;
    return;
  }
}


/* creates the array of filter regexps. fills it with nulls*/
char **create_filter_regexp()
{
  char **result;
  int i;

  if ((result = (char **) malloc (MAX_FILTER_REGEXPS*sizeof(char *))) == NULL)
    SavantError(ENOMEM, "Unable to malloc filter regexps in conftemplates.c");

  for (i=0; i<MAX_FILTER_REGEXPS; i++) {
    result[i] = NULL;
  }

  return result;

}

void add_filter_regexp(char **filter_array, char *filter_regexp)
{
  int i=0;

  while (filter_array[i] != NULL) {
    i++;
    /*safety check*/
    if (i>=MAX_FILTER_REGEXPS) {
      SavantError(ENOMEM, "maximum number of filter regexps already reached");
    }
  }

  filter_array[i] = strdup(filter_regexp);
}    
  
void free_template_field_info(Template_Field_Info *current_info )
{
  int i;

  free(current_info->id_regexp);
  for (i=0; current_info->filter_regexp[i] != NULL; i++) {
    free(current_info->filter_regexp[i]);
  }
}

/* Free memory pointed to by template (including all sub-fields and strings in the data struct) */
void free_template (General_Template *template)
{
  int i;
  
  free(template->printname);
  free(template->recognize);
  free(template->delimiter);

  for(i=0; template->fields[i]!=NULL; i++) {
    free_template_field_info(template->fields[i]);
  }

  free(template);
}

/* conftemplates.h says that parser wants a pointer to self as well.
   Note: self (in parser) is a pointer to the field being parsed.  We need it 'cause
   the encoder needs to know the field's typenum 

   [Isn't taking in a pointer to the Array_of_Field_Types kind
   of redundant, since won't AoFT be global anyway? -- event]

*/

/* Create a field and add it to fieldArray, choosing the next fieldnum from the array 
   and updating the Array's num_fields */
void create_and_add_field (Array_of_Field_Types *fieldArray,
                           char *printname, 
			   enum Title_Defaults_Type titleDefault,
                           void *(*parser)(char *fielddata, void *self, DB_UINT docnum), 
                           GBuffer *(*deparser)(void *parseddata, void *self), 
                           void (*index_store)(void *parseddata,/* Whatever is returned by parser */
                                                char *dbdir,     /* database directory */
                                                int last_write_p),/* last_write_p == 1 if this is the end 
                                                                     and we just want to finalize the write */ 
                           void *(*nextword)(void *parseddata, 
                                             int reset_p),
                           void (*update_sims_word)(void *word, 
                                                    Remem_Hash_Table *all_sims, 
                                                    void *self,
                                                    Retrieval_Database_Info *rdi),
                           void (*cleanup_parsed)(void *parseddata))

{
  Field *new_field;
  char errorstring[256];
  if (fieldArray->num_fields >= MAX_NUMBER_FIELDS) {
    sprintf(errorstring, "create_and_add_field: already hit max number of fields (%d) when creating %s",
            MAX_NUMBER_FIELDS, printname);
    SavantError(ENOBUFS, errorstring);
  }
  new_field = (Field *) malloc(sizeof(Field));

  new_field->typenum = fieldArray->num_fields;
  new_field->printname = printname;
  new_field->titleDefault = titleDefault;
  new_field->parser = parser;
  new_field->deparser = deparser;
  new_field->index_store = index_store;
  new_field->nextword = nextword;
  new_field->update_sims_word = update_sims_word;
  new_field->cleanup_parsed = cleanup_parsed;

  fieldArray->field[(fieldArray->num_fields)++] = new_field;
}



List_of_General_Templates *create_template_list()
{
  List_of_General_Templates *result;
  
  result = (List_of_General_Templates *) malloc (sizeof(List_of_General_Templates));
  result->template = NULL;
  result->next = NULL;

  return result;
}

Array_of_Field_Types *create_field_array()
{
  int i;
  Array_of_Field_Types *result;

  result = (Array_of_Field_Types *)malloc (sizeof(Array_of_Field_Types));
  result->num_fields = 0;
  result->field = (Field **) malloc (MAX_NUMBER_FIELDS * sizeof (Field *));
  for(i=0; i<MAX_NUMBER_FIELDS; i++) {
    result->field[i] = NULL;
  }

  return result;
}


/* Create the templates, and point the global variable All_General_Templates at it.
   This procedure replaces the work that used to be done by loading .savantrc */
void load_config()
{
  int i;
  General_Template *current_template;
  Field **fields;
  char **current_filter_regexps;
  char **email_body_filter_regexps;
  char **email_subject_filter_regexps;
  char **html_filter_regexps;
  char **latex_filter_regexps;
  char **email_from_filter_regexps;
  char **inspec_author_filter_regexps;
  char **globe_author_filter_regexps;
  char **globe_body_filter_regexps;

  /*Initializing the field and template stuff */
  All_Fields = create_field_array();
  All_General_Templates = create_template_list();

/* Initialize Configuration stuff */
  Config.ellipses = 1;
  Config.source_field_width = 20;
  Config.index_dotfiles = 0;

  /*Initializing the template stuff*/

  /* Add in all the fields */
  /*  printname   parser         deparser    index_store    nextword     word_similarity  cleanup_parsed */
  /* PLUG-IN API:

------------------------
     parser: void *parser (char *fielddata, void *self, DB_UINT docnum)
         Parse a field from an indexed document or from a query.  Turn it into machine readable format.

         fielddata: a string of data for a field as it comes from the document (after filtering).  E.g. the raw text
	            from the body of an email message.
	 self:      actually of type (Field *).  This can be used to encode different fields differently, or ignored.  E.g.
	            in text we encode the top 6 bits as a typenum, so we don't compare BODY text with SUBJECT text.
	 docnum:    The document number.  This is a serial number, one per doc.

         RETURNS:   the parser returns a void *, which contains some machine readable structure used by deparser, index_store,
                    nextword, and cleanup_parsed.

------------------------
     deparser: GBuffer *deparser (void *parsedata, void *self) 
         Take field data output from the parser and turn it into printabe text (for debugging / feedback).

         parsedata: a pointer to parsed data.  This is of the type returned by parser.
	 self:      actually of type (Field *).  This is the field of this data (and can be ignored if not needed).

	 RETURNS:   a GBuffer (growable string type) containing human-readable text representing this parsed field data.
	            This is mainly used for debugging strings (printed with the -d option).

------------------------
    index_store: void index_store (void *parsedata, char *dbdir, int last_write_p)
         Take field data output from the parser and store it to disk.

         parsedata:    a pointer to parsed data.  This is of the type returned by parser, and contains the info to be indexed.
	 dbdir:        a string containing the fully expanded directory name of the index files.  This is the directory where all
	               the files should be written.  Temporary scratch files can be written here as well, though they should be 
		       deleted after the final write.
	 last_write_p: this is set to 1 if this is the final write (allowing you to close files, merge checkpoint files, delete
	               scratch files, etc).

	 RETURNS:      nothing.  However, this function should write this field info to disk in whatever format is most suited
	               for fast later retrieval.

------------------------
     nextword: void *nextword (void *parsedata, int reset_p)
         An iterator: Take field data output from the parser and return the next "word" from the parsed output.
	 Word is loosly defined as a single element, e.g. a single word, GPS location, date, person's name, etc.
	 nextword is only called on a query field during retrieval, not on indexed document fields.

         parsedata:    a pointer to parsed data.  This is of the type returned by parser. 
	 reset_p:      if reset_p == 1, start with the first word of this parsed data.  Otherwise, return the next one.
	               nextword is responsible for keeping it's own place (via static memory, presumably).  
		       Yes, it's icky, but it works.

	 RETURNS:      The word type includes any info that might be needed by the retrieval system, e.g. word weight, 
	               machine readable version of the word, normalization info, etc.  The word might also contain a 
		       human-readable version of the word, for filling into the top contributors keyword list by 
		       update_sims_word.  The return value is used by update_sims_word during retrieval.  Return NULL 
                       when there are no more "words."

------------------------
      update_sims_word: void update_sims_word (void *word, Remem_Hash_Table *all_sims, void *field, Retrieval_Database_Info *rdi)
         A proceedure that takes a word at a time and updates (adds to) an array of document similarities.  This proceedure
	 can be any algorithm so long as it can handle updating a document similarity one word at a time, without seeing all
	 the other words.  (This is for the interests of speed.  If you actually need global knowledge, like say the length of
	 a query or document for normalization, you can squirrel it away either in the index files format for this type or in
	 the value returned by nextword.)  Called during query retrieval, not indexing.
         update_sims_word also needs to update the Top_Contributors list, which contains the similarity and printname of each
         word that contributed the most to choosing this document.

	 word:     the single word that is potentially adding weight to a document.  Of the type returned by nextword.
	 all_sims: an array of similarities, indexed by docnum.  Similarities include an array of "top contributors" to 
	           a document being chosen (used for user feedback of why this document was chosen).
	 field:    actually of type (Field *).  This is the field of this data (and can be ignored if not needed).
	           This might be useful to grab other function pointers. 
	 rdi:      Document info that might be useful to an information-retrieval algorithm.  Things like total number
	           of documents plus the expanded path for the index files so they can be opened.

------------------------
       cleanup_parsed: void cleanup_parsed (void *parseddata)
         Free all the memory allocated by the parser routine.
	 
	 parsedata: a pointer to parsed data.  This is of the type returned by parser.
*/
  caf("BODY", BLANK_TITLE, &parse_text, &deparse_text, &index_store_text, &nextword_text, &update_sims_word_text_okapi, &free_parsed_text);


/*  caf("BODY", BLANK_TITLE, &parse_text, &deparse_text, &index_store_text, &nextword_text, &update_sims_word_text_tfidf, &free_parsed_text); */
  caf("LOCATION", BLANK_TITLE, &parse_text_nostopstem, &deparse_text, &index_store_text, &nextword_text, &update_sims_word_text_tfidf, &free_parsed_text);

/*  caf("DATE", MODTIME_TITLE, &parse_date, &deparse_date, &index_store_date, &nextword_date, NULL, &free_parsed_date);*/
/*  caf("DATE", MODTIME_TITLE, NULL, NULL, NULL, NULL, NULL, NULL); */
  caf("DATE", BLANK_TITLE, NULL, NULL, NULL, NULL, NULL, NULL); 
  caf("TIME", MODTIME_TITLE, NULL, NULL, NULL, NULL, NULL, NULL);
  caf("DAY", BLANK_TITLE, NULL, NULL, NULL, NULL, NULL, NULL);
  caf("SUBJECT", FILENAME_TITLE, &parse_text, &deparse_text, &index_store_text, &nextword_text, &update_sims_word_text_okapi, &free_parsed_text);
  caf("PERSON", OWNER_TITLE, &parse_text_nostopstem, &deparse_text, &index_store_text, &nextword_text, &update_sims_word_text_tfidf, &free_parsed_text);

  /* Make a nice alias for the actual array of fields */
  fields = All_Fields->field;

  /* 
     Template definition:


     current_template = create_template(<name>,
                                        <recognition regex>,
                                        <document delimiter>, 
                                        <REJECT_ACTION | INDEX_ACTION>,
                                        <INDEX_TYPE | RETRIEVE_TYPE>);

     current_filter_regexps = create_filter_regexp();
     add_filter_regexp(current_filter_regexps, <filter regexp 1>);
     add_filter_regexp(current_filter_regexps, <filter regexp 2>);
     ...
     add_filter_regexp(current_filter_regexps, <filter regexp n>);
     atfn(<field name>, <identification regex>, <index into identification regex>, current_filter_regexps, <bias>);

     current_filter_regexps = create_filter_regexp();
     add_filter_regexp(current_filter_regexps, <filter regexp 1>);
     add_filter_regexp(current_filter_regexps, <filter regexp 2>);
     ...
     add_filter_regexp(current_filter_regexps, <filter regexp n>);
     atfn(<field name>, <identification regex>, <index into identification regex>, current_filter_regexps, <bias>);

     attl;

     ... next template
     current_template = create_template(<name>, 
                                        <recognition regex>, 
                                        <document delimiter>,
                                        <REJECT_ACTION | INDEX_ACTION>,
                                        <INDEX_TYPE | RETRIEVE_TYPE>);

     ... etc ...
  */
  
  /* the following comment might be useful in some location or other */
  /*   Field      id_regexp          id_index  filter_regexp  bias*/ 

  /* Make the templates*/

  /*******************************/
  /*       RMAIL template        */
  /*******************************/
  current_template = create_template("RMAIL",
                                     "BABYL OPTIONS", 
                                     "(^|\n)",
                                     ACCEPT_ACTION,
                                     INDEX_TYPE);

  email_body_filter_regexps = create_filter_regexp();

  /* Filters for email bodies */

  /* Included headers in forwarded email */
  /* Note the {0,100} at the end -- this sanity check keeps pcre from blowing up if, say, 
     you're given an email body with thousands of lines of code all indented so it thinks
     it's all one header */
  add_filter_regexp(email_body_filter_regexps, "(^|\n)(\\ |\\t|\\>)*[a-zA-Z]+[a-zA-Z\\-\\;]*:([^\n]*)(\n(\\ |\\t|\\>)([^\n]*)){0,100}");

  /* Signature files.  
     The standard is "\n-- \n". I'm including forwards-indented versions as well, plus some
     non-standards like a line of 5+ non-text, followed by 1-5 lines whatever, 
     followed by another line of non-text */
  
  add_filter_regexp(email_body_filter_regexps, "(^|\n)--( ){0,1}\n([^\n]*(\n|$)){1,8}([^\n]*$)");
  add_filter_regexp(email_body_filter_regexps, "(^|\n)[>\\s]+--( ){0,1}\n([^\n]*(\n|$)){1,8}([^\n]*$)");
  add_filter_regexp(email_body_filter_regexps, "(^|\n)[^a-zA-Z0-9\n]{5,}\n([^\n]*(\n|$)){1,8}([^a-zA-Z0-9\n]{5,}\n)");
  add_filter_regexp(email_body_filter_regexps, "(^|\n)[^a-zA-Z0-9\n]{5,}\n([^\n]*(\n|$)){1,8}([^\n]*$)");

  /* A few specific useless phrases */
  add_filter_regexp(email_body_filter_regexps, "^EMACS REMEMBRANCE QUERY MODE:.*?($|\n)");
  add_filter_regexp(email_body_filter_regexps, "(^|\n)(\\W)*Unsent message follows(\\W)*?\n");
  add_filter_regexp(email_body_filter_regexps, "(^|\n)(\\W)*This is a multi-part message in MIME format\\.(\\W)*?\n");
  add_filter_regexp(email_body_filter_regexps, "(^|\n)[^a-zA-Z0-9\n]*(B|b)egin (F|f)orwarded[^\n]*\n");
  add_filter_regexp(email_body_filter_regexps, "(^|\n)[^a-zA-Z0-9\n]*(E|e)nd (F|f)orwarded[^\n]*\n");
  add_filter_regexp(email_body_filter_regexps, "(^|\n)This message is in MIME format. Since your mail reader does not understand\nthis format, some or all of this message may not be legible.");

  /* ------ headers ------               */
  /* and                                 */
  /* -----------94C4424FBB52619DF028A003 */
  /* and                                 */
  /* *** EOOH ***                        */
  add_filter_regexp(email_body_filter_regexps, 
      "(^|\n)(\\ |\\t|\\>)*(\\-|\\*)+([\\w|\\ |\\t]*)([^a-zA-Z0-9\n])*?\n");
  add_filter_regexp(email_body_filter_regexps, 
      "(^|\n)(\\ |\\t|\\>)*(\\-|\\*|\\+|\\=){3,}([\\w|\\ |\\t]*)([a-zA-Z0-9])*[\\ |\\t|\\-|\\*|\\+|\\=]*\n");

/* And any other sort of headers that start:
   Path: here
   or
   X-Bla-Bla: do do do
*/
  add_filter_regexp(email_body_filter_regexps, "(^|\n)[A-Z]([a-zA-Z\\-])*: [^\n]*");

  atfn("BODY", "\n\n(.*)$", 1, email_body_filter_regexps, 4, 0);

  email_subject_filter_regexps = create_filter_regexp();
  add_filter_regexp(email_subject_filter_regexps, "^(\\ )*\\[[\\ a-zA-Z0-9\\.]*?\\@[a-zA-Z0-9\\.]*?\\: ");
  add_filter_regexp(email_subject_filter_regexps, "^\\{.*?\\} ");
  atfn("SUBJECT", "(^|\n)Subject:\\s*(.*?\n)\\S", 2, email_subject_filter_regexps, 1, 90);

  email_from_filter_regexps = create_filter_regexp();
  add_filter_regexp(email_from_filter_regexps, "^[^<]*<");
  add_filter_regexp(email_from_filter_regexps, "@[^@]*");
  atfn("PERSON", "(^|\n)From:\\s*(.*?\n)\\S", 2, email_from_filter_regexps, 1, 50);
  atfn("DATE", "(^|\n)Date:\\s*(.*?\n)\\S", 2, NULL, 1, 40);
  attl;

  /**********************/
  /*       GLOBE        */
  /**********************/

  current_template = create_template("Globe",
                                     "^Title: ",
				     NULL,
                                     ACCEPT_ACTION,
                                     INDEX_TYPE);

  atfn("SUBJECT", "^Title:\\s+(.*?)\n", 1, NULL, 2, 90);

  globe_author_filter_regexps = create_filter_regexp();
  add_filter_regexp(globe_author_filter_regexps, "By ");
  add_filter_regexp(globe_author_filter_regexps, "Globe Staff");
  add_filter_regexp(globe_author_filter_regexps, "\\(");
  add_filter_regexp(globe_author_filter_regexps, "\\)");
  add_filter_regexp(globe_author_filter_regexps, "Associated Press");
  atfn("PERSON", "\nSource:\\s+(.*?)\n", 1, globe_author_filter_regexps, 1, 50);

  current_filter_regexps = create_filter_regexp();
  add_filter_regexp(current_filter_regexps, " \\d{9,9}");
  atfn("DATE", "\nDate:\\s+(.*?)\n", 1, current_filter_regexps, 1, 15);

  globe_body_filter_regexps = create_filter_regexp();
  add_filter_regexp(globe_body_filter_regexps, "Title:.*?\n");
  add_filter_regexp(globe_body_filter_regexps, "Date:.*?\n");
  add_filter_regexp(globe_body_filter_regexps, "SOURCE:.*?\n");
  add_filter_regexp(globe_body_filter_regexps, "KEYWORDS:.*?\n");
  atfn("BODY", "(.*)", 1, globe_body_filter_regexps, 2, 0);

  attl;   /* add_template_to_list */


  /********************************************/
  /*        Tech article HTML template        */
  /********************************************/
  current_template = create_template("tech_article",
				     "<META RATYPE=\"Tech article\">\n",
				     NULL,
                                     ACCEPT_ACTION,
                                     INDEX_TYPE);

  current_filter_regexps = create_filter_regexp();
  add_filter_regexp(current_filter_regexps, "<[^>]*>");             /* HTML Tags (<i>, <b> mostly */
  add_filter_regexp(current_filter_regexps, "&(\\x23\\d+|\\w+);");     /* &nbsp; and the like */
  atfn("SUBJECT", "(<title>|<TITLE>)(.*?)(</title>|</TITLE>)(.*)", 2, current_filter_regexps, 1, 90);

  atfn("PERSON", "^$", 1, NULL, 1, 20);   /* Never matches, but forces the title to use a default */

  current_filter_regexps = create_filter_regexp();
  add_filter_regexp(current_filter_regexps, "Monday\\s*,\\s*");
  add_filter_regexp(current_filter_regexps, "Tuesday\\s*,\\s*");
  add_filter_regexp(current_filter_regexps, "Wednesday\\s*,\\s*");
  add_filter_regexp(current_filter_regexps, "Thursday\\s*,\\s*");
  add_filter_regexp(current_filter_regexps, "Friday\\s*,\\s*");
  add_filter_regexp(current_filter_regexps, "Saturday\\s*,\\s*");
  add_filter_regexp(current_filter_regexps, "Sunday\\s*,\\s*");
  add_filter_regexp(current_filter_regexps, "Monday\\s*\\.\\s*");
  add_filter_regexp(current_filter_regexps, "Tuesday\\s*\\.\\s*");
  add_filter_regexp(current_filter_regexps, "Wednesday\\s*\\.\\s*");
  add_filter_regexp(current_filter_regexps, "Thursday\\s*\\.\\s*");
  add_filter_regexp(current_filter_regexps, "Friday\\s*\\.\\s*");
  add_filter_regexp(current_filter_regexps, "Saturday\\s*\\.\\s*");
  add_filter_regexp(current_filter_regexps, "Sunday\\s*\\.\\s*");
  add_filter_regexp(current_filter_regexps, "^ ");
  atfn("DATE", "This (story||comic||photograph) was( originally||) published on (.*?)\\.", 3, current_filter_regexps, 1, 20);

  current_filter_regexps = create_filter_regexp();
  add_filter_regexp(current_filter_regexps, "Copyright [^\n]*The Tech. All rights reserved.*");
  add_filter_regexp(current_filter_regexps, "This (story||comic||photograph) was( originally||) published on.*");
  add_filter_regexp(current_filter_regexps, "<!--.*?-->");             /* Comments */
  add_filter_regexp(current_filter_regexps, "<[^>]*>");                /* Tags */
  add_filter_regexp(current_filter_regexps, "&(\\x23\\d+|\\w+);");     /* &nbsp; and the like */
  
  /* We can't rely on valid HTML with a body tag, so just do the whole thing and throw out the tags */
  atfn("BODY", "(.*)", 1, current_filter_regexps, 1, 0);
  attl;



  /*******************************/
  /*        HTML template        */
  /*******************************/
  current_template = create_template("HTML",
				     "<title>|<TITLE>|<html>|<HTML>|<a href=|<A HREF=|<p>|<P>",
				     NULL,
                                     ACCEPT_ACTION,
                                     INDEX_TYPE);

  html_filter_regexps = create_filter_regexp();
  add_filter_regexp(html_filter_regexps, "<!--.*?-->");             /* Comments */
  add_filter_regexp(html_filter_regexps, "<[^>]*>");                /* Tags */
  add_filter_regexp(html_filter_regexps, "&(\\x23\\d+|\\w+);");     /* &nbsp; and the like */
  
  add_filter_regexp(html_filter_regexps, "(^|\n)EMACS REMEMBRANCE QUERY MODE:.*?($|\n)");
  add_filter_regexp(html_filter_regexps, "(^|\n)WEBRA REMEMBRANCE QUERY MODE.*?($|\n)");

  atfn("SUBJECT", "(<title>|<TITLE>)(.*?)(</title>|</TITLE>)(.*)", 2, html_filter_regexps, 1, 90);

  atfn("PERSON", "^$", 1, NULL, 1, 20);   /* Never matches, but forces the title to use a default */

  /* We can't rely on valid HTML with a body tag, so just do the whole thing and throw out the tags */
  atfn("BODY", "(.*)", 1, html_filter_regexps, 1, 0);
  attl;



  /************************************/
  /*       Unix Email template        */
  /************************************/
  current_template = create_template("unix_email",
                                     "(^|\n)From ",
                                     "(^|\n)From ",
                                     ACCEPT_ACTION,
                                     INDEX_TYPE);


/* mime, html, pgp, vcard (ie?  Netscape?), attachement */

  atfn("BODY", "(\r\n\r\n|\n\n)(.*)$", 2, email_body_filter_regexps, 4, 0);  /* \r's so it can handle DOS-files */
  atfn("SUBJECT", "\nSubject:\\s*(.*?\n)\\S", 1, email_subject_filter_regexps, 1, 90);

  atfn("PERSON", "\nFrom:\\s*(.*?)\n\\S", 1, email_from_filter_regexps, 1, 50);
  atfn("DATE", "(^|\n)From \\S*\\s*(.*?\n)", 2, NULL, 1, 40);

  attl;


  /************************************/
  /*       Athena Email template      */
  /************************************/
  current_template = create_template("athena_email",
				     "Received: from",
				     NULL,
				     ACCEPT_ACTION,
				     INDEX_TYPE);

  atfn("DATE", "\nDate: (.*?)(\\s*?)\n", 1, NULL, 1, 50);
  atfn("PERSON", "\nFrom:(.*?)\n", 1, NULL, 1, 50);
  atfn("SUBJECT", "Subject:(.*?)\n", 1, email_subject_filter_regexps, 1, 50);
  atfn("BODY", "\n\n(.*)$", 1, email_body_filter_regexps, 4, 0);
  attl;

  /*******************************/
  /*       Jimminy template      */
  /*******************************/
  current_template = create_template("Jimminy", 
	                             "------------------------(^|\n)Jimminy-header <", 
				     "(^|\n)Jimminy-header <", 
                                     ACCEPT_ACTION,
                                     INDEX_TYPE);

  atfn("LOCATION", "(^|\n)Jimminy-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 2, NULL, 1, 20);
  atfn("PERSON", "(^|\n)Jimminy-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 3, NULL, 1, 50);
  atfn("SUBJECT", "(^|\n)Jimminy-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 4, NULL, 1, 50);
  atfn("DATE", "(^|\n)Jimminy-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 5, NULL, 1, 40);
  atfn("DAY", "(^|\n)Jimminy-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 5, NULL, 1, 0);
  atfn("TIME", "(^|\n)Jimminy-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 5, NULL, 1, 0);
  atfn("BODY", "(^|\n)Jimminy-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>(.*)$", 6, NULL, 1, 20);

  attl;   /* add_template_to_list */



  /*******************************/
  /*  Edupage Archive template   */
  /*******************************/
  current_template = create_template("Edupage-Archive",
				     "<TITLE>Edupage|<TITLE>EDUCAUSE",
				     "<A NAME=\\\".*?\\\">",
                                     ACCEPT_ACTION,
                                     INDEX_TYPE);

  atfn("SUBJECT", "<B>(.*?)</B>", 1, NULL, 1, 50);
  atfn("DATE", "(\\d+ [A-Z][a-z][a-z] \\d+)", 1, NULL, 1, 12);

  current_filter_regexps = create_filter_regexp();
  add_filter_regexp(current_filter_regexps, "<!--.*?-->");
  add_filter_regexp(current_filter_regexps, "<[^>]*>");
  add_filter_regexp(current_filter_regexps, "&(\\x23\\d+|\\w+);");
  add_filter_regexp(current_filter_regexps, "Edupage is written by.*");

  atfn("BODY", "(.*)", 1, current_filter_regexps, 1, 0);
  attl;


  /**********************/
  /*        LaTeX       */
  /**********************/

  current_template = create_template("LaTeX",
                                     "\\\\documentstyle|\\\\documentclass",
				     NULL,
                                     ACCEPT_ACTION,
                                     INDEX_TYPE);

  atfn("SUBJECT", "\\\\title\\{(.*?)\\}", 1, NULL, 1, 20);

  latex_filter_regexps = create_filter_regexp();
  add_filter_regexp(latex_filter_regexps, "\\\\(\\S*?)\{(.*?)\\}");
  add_filter_regexp(latex_filter_regexps, "^\\%\n");
  add_filter_regexp(latex_filter_regexps, "(^|\n)EMACS REMEMBRANCE QUERY MODE:.*?($|\n)");
  atfn("BODY", "\\\\begin\\{document\\}(.*)\\\\end\\{document\\}", 1, latex_filter_regexps, 1, 0);
  attl;


  /**********************/
  /*       INSPEC       */
  /**********************/

  current_template = create_template("INSPEC",
                                     "^<\\d+>\nAccession Number",
				     "(^|\n)<\\d+>\n",
                                     ACCEPT_ACTION,
                                     INDEX_TYPE);

/*  Remove initials (commented out right now) */
  inspec_author_filter_regexps = create_filter_regexp();
  add_filter_regexp(inspec_author_filter_regexps, " [^\\s]+[;.,]");

  atfn("PERSON", "\nAuthor\n  (.*?)\n\\S", 1, inspec_author_filter_regexps, 1, 50);
  atfn("SUBJECT", "\nTitle\n  (.*?)\n\\S", 1, NULL, 1, 90);
  atfn("LOCATION", "\nSource\n  (.*?)\n\\S", 1, NULL, 1, 0);
  atfn("DATE", "\n(Conference Information|Source)\n  (.*?)(Jan\\S*\\s+[12][90]\\d\\d|Feb\\S*\\s+[12][90]\\d\\d|Mar\\S*\\s+[12][90]\\d\\d|Apr\\S*\\s+[12][90]\\d\\d|May\\S*\\s+[12][90]\\d\\d|Jun\\S*\\s+[12][90]\\d\\d|Jul\\S*\\s+[12][90]\\d\\d|Aug\\S*\\s+[12][90]\\d\\d|Sep\\S*\\s+[12][90]\\d\\d|Oct\\S*\\s+[12][90]\\d\\d|Nov\\S*\\s+[12][90]\\d\\d|Dec\\S*\\s+[12][90]\\d\\d)(.*?)\n\\S", 3, NULL, 1, 15);
  atfn("BODY", "\nAbstract\n  (.*)", 1, NULL, 4, 0);

  attl;   /* add_template_to_list */

  /**********************/
  /*        ACM         */
  /**********************/

  current_template = create_template("ACM",
                                     "^ACM Electronic Guide to Computing Literature April 1998\n\n",
				     "(^|\n)Type: ",
                                     ACCEPT_ACTION,
                                     INDEX_TYPE);

  atfn("PERSON", "\nAuthor:\\s+(.*?)\n", 1, NULL, 1, 50);
  atfn("SUBJECT", "\nTitle:\\s+(.*?)\n", 1, NULL, 1, 90);
  atfn("BODY", "\n(Keywords|Title):\\s+(.*?)\n", 2, NULL, 1, 0);

  atfn("DATE", "\nDate:\\s+(.*?)\n", 1, NULL, 1, 15);
  attl;   /* add_template_to_list */


  /**********************/
  /*    PostScript      */
  /**********************/

  current_template = create_template("PostScript",
                                     "^\\%\\!PS",
				     NULL,
                                     REJECT_ACTION,
                                     INDEX_TYPE);
  attl;

  /**********************/
  /*    Framemaker      */
  /**********************/

  current_template = create_template("FrameMaker",
                                     "^<MIFFile ",
				     NULL,
                                     REJECT_ACTION,
                                     INDEX_TYPE);
  attl;

  /**********************/
  /*       PDF          */
  /**********************/

  current_template = create_template("PDF",
                                     "^\\%PDF-",
				     NULL,
                                     REJECT_ACTION,
                                     INDEX_TYPE);
  attl;

  /**********************/
  /*       HQX          */
  /**********************/

  current_template = create_template("HQX",
                                     "^\\(This file must be converted with BinHex ",
				     NULL,
                                     REJECT_ACTION,
                                     INDEX_TYPE);
  attl;

  /**********************/
  /*     RCS-CONTROL    */
  /**********************/

  current_template = create_template("RCS-Control",
                                     "^head(.*?)\naccess(.*?)\n(.*)symbols(.*)\n",
				     NULL,
                                     REJECT_ACTION,
                                     INDEX_TYPE);

  attl;

  /***************************/
  /*   Plain Text (default)  */
  /***************************/

  current_template = create_template("plain_text",
				     ".",             /* Always recognize (default) */
				     NULL,            /* No delimiter */
                                     ACCEPT_ACTION,
                                     INDEX_TYPE);

  atfn("BODY", "(.*)", 1, NULL, 1, 20);
  atfn("SUBJECT", "^$", 1, NULL, 1, 20);  /* Never matches, but forces the title to use a default */
  atfn("DATE", "^$", 1, NULL, 1, 20);     /* Never matches, but forces the title to use a default */
  atfn("PERSON", "^$", 1, NULL, 1, 20);   /* Never matches, but forces the title to use a default */
  attl;

  /***************** Query Templates *******************/

  /*********************************************/
  /*        RMAIL / VM Query Template          */
  /*********************************************/

  current_template = create_template("RmailorVMQuery",
				     "^EMACS REMEMBRANCE QUERY MODE: (rmail|vm)",
				     NULL,         
                                     ACCEPT_ACTION,
                                     QUERY_TYPE);

  atfn("BODY", "(.*)", 1, email_body_filter_regexps, 4, 10);     /* Grab it all, 'cause we might not be at the top anyway */
  atfn("SUBJECT", "Subject:(.*?)\n", 1, email_subject_filter_regexps, 1, 50);
  atfn("PERSON", "From:(.*?)\n", 1, email_from_filter_regexps, 1, 50);
  atfn("DATE", "Date:(.*?)\n", 1, NULL, 1, 40);
  attl;

  /*********************************************/
  /*            MAIL Query Template            */
  /*********************************************/

  current_template = create_template("MailQuery",
				     "^EMACS REMEMBRANCE QUERY MODE: mail",
				     NULL,         
                                     ACCEPT_ACTION,
                                     QUERY_TYPE);

  atfn("BODY", "(.*)", 1, email_body_filter_regexps, 4, 10);  
  atfn("SUBJECT", "Subject:(.*?)\n", 1, email_subject_filter_regexps, 1, 50);
  atfn("DATE", "Date:(.*?)\n", 1, NULL, 1, 40);

  current_filter_regexps = create_filter_regexp();
  add_filter_regexp(current_filter_regexps, "@[^@ \\,\n]*");
  add_filter_regexp(current_filter_regexps, "\\(.*?\\)");
  add_filter_regexp(current_filter_regexps, "[^<\\,\n]*<");
  atfn("PERSON", "To:(.*?)\n\\S", 1, current_filter_regexps, 1, 50);
  attl;

  /*********************************************/
  /*            GNUS Query Template           */
  /*********************************************/

  current_template = create_template("GnusQuery",
				     "^EMACS REMEMBRANCE QUERY MODE: gnus",
				     NULL,         
                                     ACCEPT_ACTION,
                                     QUERY_TYPE);

  atfn("BODY", "(.*)", 1, email_body_filter_regexps, 4, 10);     /* Grab it all, 'cause we might not be at the top anyway */
  atfn("SUBJECT", "Subject:(.*?)\n", 1, email_subject_filter_regexps, 1, 50);
  atfn("PERSON", "From:(.*?)\n", 1, email_from_filter_regexps, 1, 50);
  atfn("DATE", "Date:(.*?)\n", 1, NULL, 1, 40);
  attl;

  /*************************************/
  /*    Webra HTML Query template      */
  /*************************************/
  current_template = create_template("WebraHTMLQuery",
				     "^WEBRA REMEMBRANCE QUERY MODE",            
				     NULL,
                                     ACCEPT_ACTION,
                                     QUERY_TYPE);

  /* These might be partial HTML (i.e. one section), so section headers count for "subject" */
  /* However, since sub-headers tend to be mundain things like "project list" instead of useful
     info, we're only looking at title and h1's. */
  current_filter_regexps = create_filter_regexp();
  add_filter_regexp(current_filter_regexps, "[Hh]ome");  /* "Home page" are stop words for this domain */
  add_filter_regexp(current_filter_regexps, "[Pp]age");  /* "Home page" are stop words for this domain */
  atfn("SUBJECT", "(<title>|<TITLE>|<h1>|<H1>|<h2>|<H2>)(.*?)(</title>|</TITLE>|</h1>|</H1>|</h2>|</H2>)", 2, current_filter_regexps, 1, 20);

  /* If there's a mailto, grab the email address.  Of course, we're running into an ontology
     problem here -- person might mean email address, or it might mean a full name (like in 
     the INSPEC database).  If query and indexed file mismatch it doesn't work.  Proper thing
     to do is have a translator like with ZWrap -- I'm just punting. */
  atfn("PERSON", "(mailto:)(\\s*)([\\w\\.]*?)(@[^@\\s]*)", 3, NULL, 1, 10);

  /* We can't rely on valid HTML with a body tag, so just do the whole thing and throw out the tags */
  /* As with email, body is more important than other fields because it's a fuzzier match.
     Subject is also not a real good match for web pages, as the title often is incomplete. */
  atfn("BODY", "(.*)", 1, html_filter_regexps, 5, 0);
  
  attl;


  /*************************************/
  /*        HTML Query template        */
  /*************************************/
  current_template = create_template("HTMLQuery",
				     "^EMACS REMEMBRANCE QUERY MODE: html-mode",            
				     NULL,
                                     ACCEPT_ACTION,
                                     QUERY_TYPE);

  atfn("SUBJECT", "(<title>|<TITLE>)(.*?)(</title>|</TITLE>)(.*)", 2, NULL, 1, 20);

  /* We can't rely on valid HTML with a body tag, so just do the whole thing and throw out the tags */
  atfn("BODY", "(.*)", 1, html_filter_regexps, 1, 0);
  
  attl;


  /************************************/
  /*        LaTeX Query template      */
  /************************************/

  current_template = create_template("LaTeXQuery",
				     "^EMACS REMEMBRANCE QUERY MODE: latex-mode",            
				     NULL,
                                     ACCEPT_ACTION,
                                     QUERY_TYPE);

  atfn("SUBJECT", "(\\\\title|\\\\section|\\\\subsection)\\{(.*?)\\}", 2, NULL, 1, 20);
  atfn("BODY", "(.*)", 1, latex_filter_regexps, 1, 0);     /* Grab it all, 'cause we might not be at the top anyway */
  attl;


  /***************************/
  /*       INSPEC Query      */
  /***************************/

  current_template = create_template("INSPECQuery",
                                     "^<\\d+>\nAccession Number",
                                     NULL,
                                     ACCEPT_ACTION,
                                     QUERY_TYPE);

  atfn("PERSON", "\nAuthor\n  (.*?)\n\\S", 1, NULL, 1, 50);
  atfn("SUBJECT", "\nTitle\n  (.*?)\n\\S", 1, NULL, 1, 90);
  atfn("LOCATION", "\nSource\n  (.*?)\n\\S", 1, NULL, 1, 0);
  atfn("DATE", "\n(Conference Information|Source)\n  (.*?)(Jan\\S*\\s+[12][90]\\d\\d|Feb\\S*\\s+[12][90]\\d\\d|Mar\\S*\\s+[12][90]\\d\\d|Apr\\S*\\s+[12][90]\\d\\d|May\\S*\\s+[12][90]\\d\\d|Jun\\S*\\s+[12][90]\\d\\d|Jul\\S*\\s+[12][90]\\d\\d|Aug\\S*\\s+[12][90]\\d\\d|Sep\\S*\\s+[12][90]\\d\\d|Oct\\S*\\s+[12][90]\\d\\d|Nov\\S*\\s+[12][90]\\d\\d|Dec\\S*\\s+[12][90]\\d\\d)(.*?)\n\\S", 3, NULL, 1, 15);
  atfn("BODY", "\nAbstract\n  (.*)", 1, NULL, 4, 0);

  attl;   /* add_template_to_list */

  /*************************************/
  /*       Jimminy Query template      */
  /*************************************/
  current_template = create_template("JimminyQuery", 
	                             "^JIMMINY REMEMBRANCE QUERY MODE", 
				     NULL,
                                     ACCEPT_ACTION,
                                     QUERY_TYPE);

  current_filter_regexps = create_filter_regexp();
  add_filter_regexp(current_filter_regexps, "(^|\n)Jimminy-header <.*?\n");
  add_filter_regexp(current_filter_regexps, "(^|\n)Jimminy-query-header <.*?\n");
  add_filter_regexp(current_filter_regexps, "^JIMMINY REMEMBRANCE QUERY MODE");

  atfn("LOCATION", "(^|\n)Jimminy-query-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 2, NULL, 1, 20);
  atfn("PERSON", "(^|\n)Jimminy-query-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)", 3, NULL, 1, 20);
  atfn("SUBJECT", "(^|\n)Jimminy-query-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 4, NULL, 1, 50);
  atfn("DATE", "(^|\n)Jimminy-query-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 5, NULL, 1, 40);
  atfn("DAY", "(^|\n)Jimminy-query-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 5, NULL, 1, 0);
  atfn("TIME", "(^|\n)Jimminy-query-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 5, NULL, 1, 0);
  atfn("BODY", "(.*)", 1, current_filter_regexps, 4, 20);
  attl; 

/* This is the old query format using just the normal jimminy header. Now if you're using
   Jimminy you should be using the special hacks that put the current physical context in
   a unique format, so it can be different than the header of whatever you're reading. */
/*
  current_template = create_template("JimminyQuery", 
	                             "------------------------(^|\n)Jimminy-header <", 
				     NULL,
                                     ACCEPT_ACTION,
                                     QUERY_TYPE);

  atfn("LOCATION", "(^|\n)Jimminy-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 2, NULL, 1, 20);
  atfn("PERSON", "(^|\n)Jimminy-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 3, NULL, 1, 20);
  atfn("SUBJECT", "(^|\n)Jimminy-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 4, NULL, 1, 50);
  atfn("DATE", "(^|\n)Jimminy-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 5, NULL, 1, 40);
  atfn("DAY", "(^|\n)Jimminy-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 5, NULL, 1, 0);
  atfn("TIME", "(^|\n)Jimminy-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>", 5, NULL, 1, 0);
  atfn("BODY", "(^|\n)Jimminy-header <(.*?)\\|(.*?)\\|(.*?)\\|(.*?)>(.*?)($|\nJimminy-header)", 6, NULL, 4, 20);
  attl; 

*/


  /*********************************************/
  /*            Emacs Manual Query             */
  /*********************************************/
  current_template = create_template("Emacs_Manual_Query",
				     "^EMACS REMEMBRANCE QUERY MODE: remem-query-mode",            
				     NULL,            /* No delimiter */
                                     ACCEPT_ACTION,
                                     QUERY_TYPE);

  atfn("SUBJECT", "(^|\n)Subject: (.*?)(\nPerson: )", 2, NULL, 1, 10);
  atfn("PERSON", "(^|\n)Person: (.*?)(\nLocation: )", 2, NULL, 1, 10);
  atfn("LOCATION", "(^|\n)Location: (.*?)(\nDate: )", 2, NULL, 1, 10);
  atfn("DATE", "(^|\n)Date: (.*?)(\nBody: )", 2, NULL, 1, 10);
  atfn("TIME", "(^|\n)Date: (.*?)(\nBody: )", 2, NULL, 1, 10);
  atfn("DAY", "(^|\n)Date: (.*?)(\nBody: )", 2, NULL, 1, 10);
  atfn("BODY", "(^|\n)Body: (.*)", 2, NULL, 4, 10);
  attl;

  /*********************************************/
  /*            Emacs Field Query              */
  /*********************************************/
  current_template = create_template("Emacs_Field_Query",
				     "^EMACS REMEMBRANCE FIELD QUERY:",            
				     NULL,            /* No delimiter */
                                     ACCEPT_ACTION,
                                     QUERY_TYPE);

  current_filter_regexps = create_filter_regexp();
  add_filter_regexp(current_filter_regexps, "(^|\n)EMACS REMEMBRANCE FIELD QUERY:");
  atfn("BODY", "(^|\n)\\!BODY:(.*)($|\n\\!)", 2, current_filter_regexps, 4, 10);
  atfn("LOCATION", "(^|\n)\\!LOCATION:(.*)($|\n\\!)", 2, current_filter_regexps, 1, 10);
  atfn("DATE", "(^|\n)\\!DATE:(.*)($|\n\\!)", 2, current_filter_regexps, 1, 10);
  atfn("TIME", "(^|\n)\\!TIME:(.*)($|\n\\!)", 2, current_filter_regexps, 1, 10);
  atfn("DAY", "(^|\n)\\!DAY:(.*)($|\n\\!)", 2, current_filter_regexps, 1, 10);
  atfn("SUBJECT", "(^|\n)\\!SUBJECT:(.*)($|\n\\!)", 2, current_filter_regexps, 1, 10);
  atfn("PERSON", "(^|\n)\\!PERSON:(.*)($|\n\\!)", 2, current_filter_regexps, 1, 10);
  attl;

  /***************************************************/
  /*        Generic Emacs Text Query Template        */
  /***************************************************/
  current_template = create_template("Unknown_Emacs_Query",
				     "^EMACS REMEMBRANCE QUERY MODE: ",            
				     NULL,            /* No delimiter */
                                     ACCEPT_ACTION,
                                     QUERY_TYPE);

  current_filter_regexps = create_filter_regexp();
  add_filter_regexp(current_filter_regexps, "(^|\n)EMACS REMEMBRANCE QUERY MODE:.*?\n");
  atfn("BODY", "(.*)", 1, current_filter_regexps, 4, 10);                  
  attl;

  /*********************************************/
  /*        Generic Text Query Template        */
  /*********************************************/
  current_template = create_template("TextQuery",
				     ".",             /* Always recognize (default) */
				     NULL,            /* No delimiter */
                                     ACCEPT_ACTION,
                                     QUERY_TYPE);
  atfn("BODY", "(.*)", 1, NULL, 1, 10);
  attl;

}
