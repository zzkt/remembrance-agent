#ifndef _RMAIN_
#define _RMAIN_
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

#include "gbuf.h"
#include "conftemplates.h"
#include "parsedoc.h"
#include "hash.h"

#define TITLE_LENGTH_MAX 1024
#define RETRIEVAL_SEPERATOR_STRING "."

/* Commands that can be entered in the retrieve command loop */
enum Retrieve_Command {QUIT_COMMAND, 
		       QUERY_COMMAND, 
		       RETRIEVE_COMMAND, 
                       LOCRETRIEVE_COMMAND,
		       HELP_COMMAND,
                       SET_BIAS_COMMAND,
                       PRINT_BIASES,
                       USE_HANDSET_BIASES,
                       USE_TEMPLATE_BIASES,
                       DB_INFO_COMMAND,
                       UNKNOWN_COMMAND};

enum Retrieve_Command get_command (int *argint, char *argstring);

void get_query(GBuffer *query);
void resetDocSims(Remem_Hash_Table *docSims);
Remem_Hash_Table *initDocSims(int num_docs_total);

/* Given a query and it's template, return a biased, sorted array */
Doc_Sim_Totals *rank_docs_for_fields(Remem_Hash_Table *total_sims,
				     Remem_Hash_Table *all_sims, 
				     Doc_Info *queryInfo, 
				     General_Template *template,
				     List_of_General_Templates *All_General_Templates, 
				     Retrieval_Database_Info *rdi,
				     int number_docs_being_printed,
				     int *querybiases);

void bias_sims(Remem_Hash_Table *all_sims, 
               List_of_General_Templates *All_General_Templates, 
               Field *thisfield,
               int *querybiases,
               Retrieval_Database_Info *rdi,
               int newquery_p);

void add_additional_to_doc_sim (Doc_Sim *thisSim, float additional, char *printname);
#endif


