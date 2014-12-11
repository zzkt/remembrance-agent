/* parsers-text.c has all the (hopefully reusable) parser and deparser
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

#include "plugins.h"
#include "plugins-text.h"
#include "conftemplates.h"
#include "stem.h"
#include "stops.h"


/*###################################################################### 
   TEXT PARSING MODULES:
   modules for parsing english prose.
 ######################################################################*/

int upper=0,power=0,num_words=0;
char **comm = NULL;

int by_alpha(const void *in1,
	     const void *in2)
{
  return(strcmp(*(char **)in1, *(char **)in2));
}

/* Do some preprocessing for stop words stuff (comm_local is the list of stop words,
   from stops.h) */
int core_comm_local(void)
{
  comm = comm_local;
  num_words = num_words_local;

  /* for safety's sake */
  qsort(comm, num_words, sizeof(char *), by_alpha);

  upper = 1;
  power = 0;
  while(upper < num_words) {
    upper = upper*2 + 1;  /* upper = one less than highest 2^n < num_words */
    power++;  /* power = log(upper+1)-1 */
  }

  return(0);
}

/* Return 1 if the word is a common word (i.e. is in the stop-list), 0
   otherwise */
int is_common(char *word)
{
  int disp,loc,i,itr,step;

  if (comm == NULL) core_comm_local();   /* Init stopwords if necessary */

  loc = 0;
  itr = power+1;
  step = (1<<power);
  disp = step - 1;
  while(itr--) {
    step = step>>1;
    loc += disp;
    if(loc>=num_words)
      loc = num_words - 1;
    i = strcmp(word,comm[loc]);
    if(i == 0)
      return(1);
    else if(i < 0)
      disp = -step;
    else
      disp = step;
  }

  return(0);
}


unsigned int encode_char(unsigned char c) {
/* return a 6-bit packed representation of a char:
   a-z = 01-1A
   0-9 = 1B-24
   _   = 25
   -   = 26
   !   = 27
anything else gets mapped to ascii(c) & 0x3F (lower 6 bits)
*/
  if ((c >= 'a') && (c <= 'z')) {
    return ((int)c - (int)'a' + 1);
  }
  else if ((c >= '0') && (c <= '9')) {
    return ((int)c - (int)'0' + 0x1B);
  }
  else if (c == '_') {
    return (0x25);
  }
  else if (c == '-') {
    return (0x26);
  }
  else if (c == '!') {
    return (0x27);
  }
  else { return ((int)c & 0x3F); }
}

unsigned char decode_char (unsigned int code) {
  if ((code >= 0x1) && (code <= 0x1A)) {
    return ((char)(code + 'a' - 1));
  }
  else if ((code >= 0x1B) && (code <= 0x24)) {
    return ((char)(code + '0' - 0x1B));
  }
  else if (code == 0x25) {
    return ('_');
  }
  else if (code == 0x26) {
    return ('-');
  }
  else if (code == 0x27) {
    return ('!');
  }
  else if (code == 0) {
    return ((char)0);          /* so the rest of the string is printable */
  }
  else {
    return ((char)(code - 1));
  }
}


/* encode_text_word: encode a normal english word into a packed, 3-byte format.
   Encoding scheme:
   
   EXTENDED ENCODING SYSTEM (3 unsigned ints):
   
   In the 3-int system, each character is 6 bits, with the most significant 6-bits in the
   first int being the type field.  Bits wrap to the next byte, as follows:
   
   tttttt 111111 222222 333333 444444 55 = 32 bits
   5555 666666 777777 888888 999999 0000
   00 111111 222222 333333 444444 555555 = 15 characters, 6-bits type

   The "type" is equal to the field-number, which is dependent on the order fields are
   inserted into the template structure.  This means that different versions of Savant
   may be incompatible depending on what templates are added.
*/
void encode_text_word(unsigned char *s, 
                      unsigned int * code,
                      DB_UINT field_type)
{
  time_t timestamp;
  struct parsedate *pd;
  const time_t SECONDS_IN_DAY = 86400;
  const time_t SECONDS_IN_WEEK = 604800;
  unsigned int endstr;
  int i,j,k,offset;

/*  printf("encode_word: word = %s\n", s); */

  for (k=0;k<WORD_ENCODE_WIDTH;k++) {
    code[k] = (unsigned int)0;
  }

  /* start with the type bits */
  code[0] = field_type << (sizeof(DB_UINT)*8 - 6);

  /* 12 bits into the code for the start of the next char */
  offset = sizeof(DB_UINT)*8 - 6 - CHARACTER_ENCODE_WIDTH;   

  i=0;
  for(j=0,endstr=0; (j<WORD_ENCODE_WIDTH); j++) {
    while((offset >= 0) && (!endstr)) {
      if (!endstr && (s[i] == 0)) {  
        endstr = 1;  /* at end of the string, don't add more to the code */
      }
      if (endstr == 0) {
        code[j] += (encode_char(s[i]) << offset);
        offset -= CHARACTER_ENCODE_WIDTH;
        i++;
      }
    }
    if (!endstr) {   /* break this char up across int boundaries */
      code[j] += (encode_char(s[i]) >> -offset);
      offset = sizeof(unsigned int)*8 + offset;
    }
  }
}

void decode_text_word(unsigned int *code, char *str, char *fieldname)
{
/* Fill str with a printable representation of code */
  int i, j, offset;
  unsigned int d, carryoverd, type, mask;

  type = code[0] >> (sizeof(unsigned int)*8 - 6);
  
  d = code[0] & 0x3f;
  j = 0;
  carryoverd = 0;
  mask = 0xffffffff >> (8 * 4 - CHARACTER_ENCODE_WIDTH);  /* a mask for one packed character width */
  offset = sizeof(unsigned int)*8 - 6 - CHARACTER_ENCODE_WIDTH;
  for(i=0; i < WORD_ENCODE_WIDTH; i++) {
    while(offset >= 0) {
      d = ((code[i] >> offset) & mask) + carryoverd;
      carryoverd = 0;
      if (d) {
        str[j] = decode_char(d);
        j++;
      }
      offset -= CHARACTER_ENCODE_WIDTH;
    }
    carryoverd = ((code[i] << -offset) & mask);
    offset = sizeof(unsigned int)*8 + offset;
  }
  str[j] = 0;
  strcat (str, ": ");
  strcat (str, fieldname);
}

/* comparison function for wordcodes */
int wordcode_cmp(Wordcode code1,
		 Wordcode code2) /* compare func for qsort */
{
  int j;
  for(j=0; j<WORD_ENCODE_WIDTH; j++) {
    if (((unsigned int *)code1)[j] < ((unsigned int *)code2)[j])
      return -1;
    else if (((unsigned int *)code1)[j] > ((unsigned int *)code2)[j])
      return 1;
  }
  return 0;
}

/* copy function for wordcodes */
int wordcode_cpy(Wordcode dest, Wordcode source)
{
  int j;
  for(j=0; j<WORD_ENCODE_WIDTH; j++) {
    dest[j] = source[j];
  }
}

/* Add a wordcode to a DV_Tree

   NOTE: This assumes that we'll be adding a document at a time, and adding all words
   for a particular document at once.  If this isn't true, we'll need to actually scan
   the whole doclist for a docvec node.

   Return the number of bytes malloc'ed this call.  (Jees, doing our
   own memory management and everything.  Why not just get a real
   language and be done with it?)

*/
int dvtree_add_word(DV_Tree **treeptr,
                    Wordcode code,
                    char *printword,
                    unsigned int weight,
                    DB_UINT docnum)
{
  int cmp;
  int i;
  Doc_List *doclist;
  DV_Tree *tree = *treeptr;

  if (tree==NULL) {
    /* create new node */
    if ((tree = (DV_Tree *)malloc(sizeof(DV_Tree))) == NULL)
      SavantError(ENOMEM, "Unable to malloc *tree in parsers.c");

    if ((tree->documents = (Doc_List *)malloc(sizeof(Doc_List))) == NULL)
      SavantError(ENOMEM, "Unable to malloc document list in parsers.c");

    for (i=0; i<WORD_ENCODE_WIDTH; i++) {
      tree->wordcode[i] = code[i];
    }
    tree->documents->weight = weight;
    tree->left = NULL;
    tree->right = NULL;
    strncpy(tree->printword, printword, PRINTWORD_LENGTH);
    tree->documents->next = NULL;
    tree->documents->docnum = docnum;
    *treeptr = tree;
    return(sizeof(DV_Tree) + sizeof(Doc_List));
  }
  
  /* If we've already got a match for this wordcode AND this doc, just
     increment the weight.  If we've got the wordcode but not the doc,
     prepend this document to the front.

     NOTE: This assumes that we'll be adding a document at a time, and adding
     all words for a particular document at once.  If this isn't true, we'll 
     need to actually scan the whole doclist for a docvec node.  */

  cmp = wordcode_cmp(code, tree->wordcode);
  if(cmp==0) {
    if (tree->documents->docnum == docnum) { /* Got word & doc, increment */
      tree->documents->weight += weight;
      return(0);     /* Didn't alloc anything this round */
    }
    else {

      /* Got the word but not doc, add the doc to list 
	 This adds in reverse-doc order, 'cause it's easier. */
      if ((doclist = (Doc_List *)malloc(sizeof(Doc_List))) == NULL) 
        SavantError(ENOMEM, "Unable to malloc doclist in parsers.c");
      doclist->docnum = docnum;
      doclist->weight = weight;
      doclist->next = tree->documents;
      tree->documents = doclist;
      return(sizeof(Doc_List));
    }
  }
  else if(cmp<0) {
    return(dvtree_add_word(&(tree->left), code, printword, weight, docnum));
  }
  else {
    return(dvtree_add_word(&(tree->right), code, printword, weight, docnum));
  }
}

/* add_document_to_dvtree: merge the documentTree into target.
   documentTree is assumed to only contain a single document (one
   entry in the Doc_List per word) -- otherwise we'd have to do more
   checking and scanning.

   NOTE: In this implementation, we're first throwing words into a
   binary tree (DV_Tree) when parsing data.  This happens in
   parse_text, and is done so we can add up wordcounts for duplicates.
   We then merge trees in add_document_dvtree when indexing
   (index_store_text).  An alternate method would be to just collect
   documents in a linked list in parse_text, and only do the
   tree-search once instead of twice.  The downside is we'd have to
   malloc more memory, 'cause we'd have a list entry for every
   occurence of a word whether it's duplicated or not. 

   Return the number of bytes added to target this round, so we can
   keep track of memory usage.  (There really should be a better way.)  */

int add_document_to_dvtree (DV_Tree **target, DV_Tree *documentTree) {
  int mem = 0;    /* memory used */

  if (documentTree == NULL) return(0);      /* nothing to do */

  /* Add everything to the left, add everything to the right */
  if (documentTree->left != NULL) {
    mem += add_document_to_dvtree(target, documentTree->left);
  }
  if (documentTree->right != NULL) {
    mem += add_document_to_dvtree(target, documentTree->right);
  }

  /* I'm now a leaf node, so add & free myself */
  mem += dvtree_add_word(target, 
                         documentTree->wordcode, 
                         documentTree->printword,
                         documentTree->documents->weight, 
                         documentTree->documents->docnum);

/* We used to free things here, but now we're freeing in a top-level call in main. */
/*free(documentTree->documents);
  free(documentTree);
*/
  return(mem);
}  

/*###################################################################*/

/* parse_text: Throws out stop words, stems remaining words, and
   returns a Text_Document_Field, which contains the field typenum,
   the length of this document, and a DV_Tree (DV_Tree = binary
   doc-vector tree sorted by word-code, with weights for this
   document). self is the field being parsed, which we need to figure
   out the type number to encode in each word.  */

void *parse_text (char *fielddata, void *self, DB_UINT docnum) {
  static char word[16], printword[PRINTWORD_LENGTH];
  char *bufptr, *start, *end;
  unsigned int code[WORD_ENCODE_WIDTH];
  DV_Tree *tree = NULL;
  Text_Document_Field *return_value = NULL;
  int i, j;
  DB_UINT numwords = 0;  /* number unique words in this text */
  DB_UINT typenum;
  int punct=0, alpha = 0, midpunct=0;  

  typenum = ((Field *)self)->typenum;

  bufptr = fielddata;           /* fielddata gets freed by whoever calls us */


 /* quit at the end of the fielddata, and don't go over the max number of words */
  while((*bufptr != '\0') &&
        ((numwords < MAX_NUMBER_WORDS_PER_INDEXED_FIELD) || 
         !MAX_NUMBER_WORDS_PER_INDEXED_FIELD)) {
    i = 0;  /* index into word, to keep from going over */
    j = 0;  /* number of characters in this word in the fielddata */
    punct = 0;  /* flag for whether we've got punctuation or other 
		   non-alnum chars (or -,_) in there */
    midpunct = 0; /* flag for whether we have punctuation in the middle of
                     the word (alphanumerics on either side of non "-" or "_" punctuation */
    alpha = 0; /* flag for if the text has letters (i.e. it's a number, 
		  not a phone number / date) */

    while(isspace(*bufptr) && (*bufptr != '\0')) {
      /* skip space at the beginning */
      bufptr++;
    }
    start = bufptr;
    while(!isspace(*bufptr) && (*bufptr != '\0')) {
      /* skim to the next space */
      if (!midpunct &&
          punct &&
          isalnum(*bufptr)) {
        midpunct = 1;
      }
      if (!punct && 
	  !isalnum(*bufptr) && 
	  (*bufptr != '_') && 
	  (*bufptr != '-') && 
	  !isspace(*(bufptr + 1))) {
        punct = 1;
      }
      if (isalpha(*bufptr) || (*bufptr == '_') || (*bufptr == '-')) {
        alpha = 1;
      }
      bufptr++;
    }
    end = bufptr;
    if (midpunct || !alpha || (end - start) > 30) {
      /* This is a bogus word (e.g. uuencode, email header/address,
         number only) -- clear it */
      word[0] = '\0';
    }
    else {
      while(!isalnum(*end) && (start < end)) {
        /* remove non-alpha-numerics from the end of the word */
        end--;
      }
      end++;   /* So that end still points to the character -after- the word */
      while(!isalnum(*start) && (start < end)) {
        /* remove non-alpha-numerics from the start of the word */
        start++;
      }
      for (i=0; ((start + i) < end) && (i < 15); i++) {
        /* Copy the word over */
        word[i] = tolower(*(start + i));
      }
      word[i] = '\0';
    }
    
    if(!is_common(word)) { /* skip "stop" words */
      strncpy(printword, word, PRINTWORD_LENGTH);
      Stem(word);
      if(word[0] != '\0') { /* Stem may stem word to nothing */
        encode_text_word(word, code, typenum);

        /* dvtree_add_word returns the number of bytes allocated.  If this
           is zero, it means we added a word that's already in the tree
           (not unique), so don't increment.  Yes, it's confusing and ugly,
           but it works.
        */
        if (dvtree_add_word(&tree, code, printword, 1, docnum)) {
          numwords++;
        }
      }
    }
  }

  return_value = (Text_Document_Field *)malloc(sizeof(Text_Document_Field));
  return_value->docnum = docnum;
  return_value->length = numwords;
  return_value->fieldtypenum = typenum;
  return_value->tree = tree;
  return((void *)return_value);
}


/* parse_text_nostopstem: Returns a Text_Document_Field, which
   contains the field typenum, the length of this document, and a
   DV_Tree (DV_Tree = binary doc-vector tree sorted by word-code, with
   weights for this document). self is the field being parsed, which
   we need to figure out the type number to encode in each word.

   This version is the same as parse_text, but doesn't stem the word or throw
   out stop-words.  */

void *parse_text_nostopstem (char *fielddata, void *self, DB_UINT docnum) {
  static char word[16], printword[PRINTWORD_LENGTH];
  char *bufptr, *start, *end;
  unsigned int code[WORD_ENCODE_WIDTH];
  DV_Tree *tree = NULL;
  Text_Document_Field *return_value = NULL;
  int i, j;
  DB_UINT numwords = 0;  /* number unique words in this text */
  DB_UINT typenum;
  int punct=0, alpha = 0;  

  typenum = ((Field *)self)->typenum;

  bufptr = fielddata;           /* fielddata gets freed by whoever calls us */
  while(*bufptr != '\0') { /* quit at the end of the fielddata */
    i = 0;  /* index into word, to keep from going over */
    j = 0;  /* number of characters in this word in the fielddata */
    punct = 0;  /* flag for whether we've got punctuation or other 
		   non-alnum chars (or -,_) in there */
    alpha = 0; /* flag for if the text has letters (i.e. it's a number, 
		  not a phone number / date) */

    while(isspace(*bufptr) && (*bufptr != '\0')) {
      /* skip space at the beginning */
      bufptr++;
    }
    start = bufptr;
    while(!isspace(*bufptr) && (*bufptr != '\0')) {
      /* skim to the next space */
      if (!punct && 
	  !isalnum(*bufptr) && 
	  (*bufptr != '_') && 
	  (*bufptr != '-') && 
	  !isspace(*(bufptr + 1))) {
        punct = 1;
      }
      if (isalpha(*bufptr) || (*bufptr == '_') || (*bufptr == '-')) {
        alpha = 1;
      }
      bufptr++;
    }
    end = bufptr;
    if (punct || !alpha || (end - start) > 30) {
      /* This is a bogus word (e.g. uuencode, email header/address,
         number only) -- clear it */
      word[0] = '\0';
    }
    else {
      while(!isalnum(*end) && (start < end)) {
        /* remove non-alpha-numerics from the end of the word */
        end--;
      }
      end++;   /* So that end still points to the character -after- the word */
      while(!isalnum(*start) && (start < end)) {
        /* remove non-alpha-numerics from the start of the word */
        start++;
      }
      for (i=0; ((start + i) < end) && (i < 15); i++) {
        /* Copy the word over */
        word[i] = tolower(*(start + i));
      }
      word[i] = '\0';
    }
    
    strncpy(printword, word, PRINTWORD_LENGTH);
    if(word[0] != '\0') { 
      encode_text_word(word, code, typenum);
      /* dvtree_add_word returns the number of bytes allocated.  If this
         is zero, it means we added a word that's already in the tree
         (not unique), so don't increment.  Yes, it's confusing and ugly,
         but it works.
      */
      if (dvtree_add_word(&tree, code, printword, 1, docnum)) {
        numwords++;
      }
    }
  }

  return_value = (Text_Document_Field *)malloc(sizeof(Text_Document_Field));
  return_value->docnum = docnum;
  return_value->length = numwords;
  return_value->fieldtypenum = typenum;
  return_value->tree = tree;
  return((void *)return_value);
}


/* deparse_text: looks at a parsed text field (a Text_Document_Field
   contained in fielddata) and returns a GBuffer with some printable
   text describing that text.  Right now we're assuming the DV_Tree
   only contains words from a single document.  If this becomes not
   true, we'll have to actually check docnumbers and/or scan document
   lists. */

GBuffer *deparse_text_from_dvtree (DV_Tree *tree, void *self,
                                   int *total_weight, int *total_numwords) {
  GBuffer *left, *right;
  char decoded[100], word[60], codes[60], tempbuf[60];
  int i;

  codes[0]   = '\0';
  word[0]    = '\0';
  decoded[0] = '\0';
  tempbuf[0] = '\0';

  if (tree == NULL) {
    return(NULL);
  }

  left = deparse_text_from_dvtree(tree->left, self, total_weight, total_numwords);
  right = deparse_text_from_dvtree(tree->right, self, total_weight, total_numwords);

  /* If we've got something to our left, just reuse it instead of
     using more memory.  Otherwise, malloc it. */
  if (left == NULL) {     
    left = (GBuffer *)malloc(sizeof(GBuffer));
    init_GBuffer(left);
  }

  /* Figure what to print & put it in the decoded string */
  decode_text_word(tree->wordcode, word, ((Field *)self)->printname);
  for(i=0; i<WORD_ENCODE_WIDTH; i++) {
    sprintf(tempbuf, "%08x ", tree->wordcode[i]);
    strcat(codes, tempbuf);
  }
  sprintf (decoded, "%s (%s): %d\n", codes, word, tree->documents->weight);
  *total_weight += tree->documents->weight;
  *total_numwords += 1;

  strncat_GBuffer(left, decoded, strlen(decoded));
  if (right != NULL) {
    strncat_GBuffer(left, strcast_GBuffer(right), right->size);
  }
  free_GBuffer(right);
  free(right);
  return(left);
}


GBuffer *deparse_text (void *fielddata, void *self) {
  GBuffer *ret;  
  int total_weight=0, total_numwords=0;
  char tmpstr[256];
  Text_Document_Field *fd;
  DV_Tree *tree;
  fd = (Text_Document_Field *)fielddata;
  if (fd == NULL) {
    return(NULL);
  }
  tree = fd->tree;
  ret = deparse_text_from_dvtree(tree, self, &total_weight, &total_numwords);
  if (ret != NULL) {
    sprintf (tmpstr, "Total weight = %d, Total num words = %d\n", total_weight, total_numwords);
    strncat_GBuffer(ret, tmpstr, strlen(tmpstr));
  }
  return(ret);
}


/* Return node with the minimum wordcode that's still > wordcode.  If 
   return_smallest == 1, just return the minimum wordcode */
DV_Tree *word_greater_than(unsigned int *wordcode, DV_Tree *root, int return_smallest) {
  DV_Tree *potential_answer = NULL;
  int comparison;

  if (root == NULL) return(NULL);

  comparison = return_smallest ? 1 : wordcode_cmp(root->wordcode, wordcode);
  if (comparison > 0) {     /* our node > wordcode, look for smaller */
    potential_answer = word_greater_than(wordcode, root->left, return_smallest);
    return((potential_answer == NULL) ? root : potential_answer);
  }
  else {    /* our node <= wordcode, look for bigger */
    return(word_greater_than(wordcode, root->right, return_smallest));
  }
}


/* Free a DV_Tree */
void free_dvtree (DV_Tree *tree) {
  Doc_List *dl, *freeme;
  if (tree != NULL) {
    free_dvtree(tree->left);
    free_dvtree(tree->right);
    dl = tree->documents;
    while (dl != NULL) {
      freeme = dl;
      dl = dl->next;
      free(freeme);
    }
    free(tree);
  }
}
  
/* Free a parsed field (destructor) 
   Takes a Text_Document_Field *, cast as a void *   */
void free_parsed_text (void *parseddata) {
  Text_Document_Field *tdf;

  if (parseddata == NULL) return;

  tdf = (Text_Document_Field *)parseddata;
  free_dvtree(tdf->tree);
  free(tdf);
}

/* Return next word in a Text_Document_Field.  If reset_p != 0, start
   over.  Right now, we're just re-searching the tree every time to
   find the next word.  Not a big deal since we're only using this for
   queries, and we expect those to be relatively small.

   We're bundling the word, weight, and length (for normalization) all
   into the Text_Word_Info.  This will get freed by
   update_sims_word. */
   
void *nextword_text (void *fielddata, int reset_p) {
  static DV_Tree *stateptr=NULL;   /* ptr to the previous word, so we can find the next. */
  Text_Document_Field *fd;
  Text_Word_Info *twi;
  
  if (fielddata == NULL) return(NULL);
  if (stateptr == NULL) reset_p = 1;
 
  fd = (Text_Document_Field *)fielddata;
  if (reset_p) stateptr = fd->tree;
  stateptr = word_greater_than(stateptr->wordcode, fd->tree, reset_p);
  if (stateptr == NULL) return(NULL);

  twi = (Text_Word_Info *)malloc(sizeof(Text_Word_Info));   /* Gets freed by update_sims_word */
  twi->weight = stateptr->documents->weight;
  twi->word = stateptr->wordcode;
  strncpy(twi->printword, stateptr->printword, PRINTWORD_LENGTH);
  twi->length = fd->length;

  return(twi);
}


