
## Remembrance Agent, circa 2021

As of 2021, this code hasn’t been updated to work with recent versions of emacs or received much in the way of maintenance. The project links are inactive and it appears abandoned by the author. Mainly kept for historical purposes.

If you are looking for a knowledge management system for emacs that is currently maintained, have a look at [org-mode](https://orgmode.org/) or [org-roam](https://www.orgroam.com/) or one of the [zettelkasten](https://melpa.org/#/?q=zettelkasten) modes…

## Remembrance Agent, version 2.12

The Remembrance Agent is the PhD work of [Bradley Rhodes](https://www.bradleyrhodes.com), and includes long hours of coding by an great team of undergraduates.

Copyright (C) 1996-2003 Massachusetts Institute of Technology and Bradley Rhodes
Released under the GNU General Public License (GPL)

### Contents

0.  Introduction
1.  Building the Remembrance Agent
2.  Creating a database from a collection of text
3.  Using the Remembrance Agent in Emacs
4.  Commands (using the RA)
6.  Savant's document template system and other customizations
7.  What's New?
8.  License information

### 0.  Introduction

The Remembrance Agent is one of the projects being developed by the MIT Media Lab's software agents group. Given a collection of the user's accumulated email, usenet news articles, papers, saved HTML files and other text notes, it attempts to find those documents which are most relevant to the user's current context. That is, it searches this collection of text for the documents which bear the highest word-for-word similarity to the text the user is currently editing, in the hope that they will also bear high conceptual similarity and thus be useful to the user's current work. These suggestions are continuously displayed in a small buffer at the bottom of the user's emacs buffer. If a suggestion looks useful, the full text can be retrieved with a single command.

The Remembrance Agent works in two stages. First, the user's collection of text documents is indexed into a database saved in a vector format. After the database is created, the other stage of the Remembrance Agent is run from emacs, where it periodically takes a sample of text from the working buffer and finds those documents from the collection that are most similar. It summarizes the top documents in a small emacs window and allows you to retrieve the entire text of any one with a keystroke.


Thanks for using the Remembrance Agent, and I hope you find it useful.

### 1.  Building the Remembrance Agent

The RA back-end has two executables, "ra-index" and "ra-retrieve", which together make the system called Savant. Since you are reading this file, I assume you've already uncompressed and untarred the RA distribution. To build Savant, cd to the RAdirectory type:

./configure; make

This should analyze your system, and then make appropriate binaries of ra-index and ra-retrieve. If you have trouble, make sure you are using the GNU version of make ("make --version" should produce something sensible). It might also be called "gmake" on your system. Once the compilation is finished, the code you willneed is:

     main/ra-index:            Binary file for indexing documents
     main/ra-retrieve:         Binary file for retrieving documents
     other/remem.el:           Emacs interface to Savant

Move ra-index and ra-retrieve to wherever you normally keep executables. Copy remem.el to wherever you keep emacs-lisp files. If you don't have a place for elisp files, you can just create a directory called "elisp" off of your main directory and put them there. You can then delete the source files if you like.


### 2.  Creating a database from a collection of text

Savant has two executables, one which indexes documents into databases, and one which performs interactive retrievals from these databases. To index, you must have a set of source text-files, and a directory Savant can put database filesinto.

Usage:
    ra-index [-v] [-d] [-s] <base-dir> <source1> [<source2>] ...
             [-e <excludee1> [<excludee2>] ...]

The <source> arguments may be files or directories. If a directory is in the list, Savant will use all its contents, recursing into all subdirectories. Non-text files and backup files (those appended with ~ or prepended with #) are ignored. It also ignores dot-files (those starting with .) and unless you specify the "-s" flag it will also ignore symbolic links. Any files or directories specified after the optional -e flag will be excluded. Savant will use any files it finds to create a database in the specified base directory, which must already exist. The optional -v argument (verbose) will direct Savant to keep you updated on its progress. So for example, ra-index -v ~/RA-indexes/mail ~/RMAIL ~/Rmail-files -e ~/Rmail-files/Old-files will build a database in the ~/RA-indexes/mail directory, made up of emails from my RMAIL file plus all files and subdirectories of ~/Rmail-files, excluding files and directories in ~/Rmail-files/Old-files. The optional -d flag turns on debugging information.

***IMPORTANT***: Savant can build databases in any directory you like, but the emacs interface for the Remembrance Agent expects a particular structure. For each database you want to make, you should create a directory, and all these directories should live in the same parent directory. For example, for my own use I have a directory ~/RA-indexes/, and within that are the directories ~/RA-indexes/mail/, ~/RA-indexes/papers/, etc. which actually contain the database files.

To see how Savant interacts with emacs while the remembrance agent is running, try running ra-retrieve with the command 'ra-retrieve -v <base-dir>' after creating a database using index. You can use ra-retrieve as a simple search engine, but usually ra-retrieve is only called by the emacs front-end.

### 3.  Using the Remembrance Agent in Emacs

You can load the Remembrance Agent automatically every time you run emacs by adding this line to your .emacs file in your homedir:

        (load "remem.el")

This assumes that you put remem.el into a directory that is in your emacs load-path. To set that loadpath to include the directory ~/elisp/, you can use the line:

        (setq load-path (append (list (expand-file-name "~/elisp")) load-path))

Before the Remembrance Agent can be used, several variables must be configured. These are listed at the top of remem.el, and are documented there as well. You will either want to set these in your .emacs file or create a file called "remem-custom.el" and load that file after loading remem.el. You probably don't want to set them in remem.el, unless you are the only person using this copy of the remembrance agent.

        (setq remem-prog-dir <prog-dir-string>)
        (setq remem-database-dir <database-dir-string>)
        (setq remem-scopes-list <scope1, scope2, ...>)

<prog-dir-string> should be the full name of the directory where you put the ra-retrieve executable, enclosed in double quotes. This defaults to "/usr/local/bin". On some systems, you need to make sure not to end the directory with a slash ("/").

<remem-database-dir> should be the full name of the directory which holds your database directories, enclosed in double quotes. Note that this is the name of a directory containing directories, not the directory containing the database files themselves. This defaults to "~/RA-indexes". On some systems, you need to make sure not to end the directory with a slash ("/").


remem-scopes-list: you can display several different "scopes," all with separate processes performing retrievals simultaneously. Scopes can have different databases, update frequencies, and ranges of words it uses for a "query." Each scope looks like this:

    (DIRN NUM-LINES UPDATE-TIME QUERY-RANGE)

     DIRN is the subdirectory of remem-database-dir with the desired database
             (This is the name of a sub-directory in remem-database-dir)
     NUM-LINES is the number of lines that you want the scope to show (initially)
     UPDATE-TIME is the time between scope updates (in seconds)
     QUERY-RANGE number of words around your cursor that you want the scope to query on

For example, my own customization shows this:

    (setq remem-scopes-list '(("my-email" 6 5 500)
                              ("my-notes" 2 10 500)))

This sets up two scopes. The first displays six suggestions from the index directory "/home/rhodes/RA-indexes/my-email/", updated every five seconds, based on the last 500 words around my cursor (where a word is considered 5 characters, so that's really just the last 500 X 5 = 2500 characters). The second scope displays two lines from "/home/rhodes/RA-indexes/my-email/", updated every ten seconds, also based on 2500 characters.

The rest of the customizations set colors, logging, display of suggestions, display thresholds and the prefix for RA commands. The default colors are set for a light background, but shift if you set hilit-background-mode to 'dark. They're all documented in remem.el -- play around with them.

Okay! After these customizations are made, you can start the Remembrance Agent by typing C-c r t. It will create its window and after a moment or two begin to display suggestions like:

  1   + rhodes       RA presence paper stuff        04/14/97  papers           suggestions, ra
  2     tara         Questions digest for IA class  09/15/97  agents-class     remembrance, agent
  3  ++ rhodes       RA Projects Internal           03/09/99  ra-bugs-archive  window, display

This can be summarized as

   <Line#>  <rating>  <author or file owner>  <subject>  <date>  <filename>  <keywords>

The rating is a measure from zero to two plus signs (0 to 1 if you set remem-print-exact-relevance-p to t) and indicate how relevant the document is to a sample of your current buffer. If you have remem-print-even-bad-relevance-p set to t then suggestions below the minimum threshold will print with a "-" sign, otherwise they'll show up as "No Sugestion." To see a suggested document, type C-c r <ID# of document>. The keywords are the top five keywords that contributed to making this suggestion, in order.

### 4.  Commands (using the RA)

C-c r t (Control-c r t): Toggle Remem
   Toggle the remembrance agent on and off.  You can also do this by
   evaluating the function (remem-toggle).

C-c r v (Control-c r v): View
   Updates the suggestion list by running queries in all scopes now.
   Bypasses the timer.

C-c r # (Control-c r <number>): Show Suggestion
   Bring up the full text of a suggested document.  If you set
   remem-load-original-suggestion to t (the default), then the full file is
   loaded.  Otherwise a copy is displayed.  If your files are big and
   contain lots of documents (e.g. large mail archives), or if you don't
   want the buffers lying around then you might want to set this to nil.
   If you set remem-non-r-number-keys to t, the keys C-c # will also work,
   thus saving you a keystroke.

C-c r r # (Control-c r r <number>): Rate
   Rate the last suggestion.  This allows you to rate the last suggestion
   followed with a number from 1 (bad suggestion) to 5 (great suggestion).
   The rating information is currently used by me to evaluate the software
   and figure out how to improve it, though eventually it may be used to
   give feedback for a machine learning system that will improve future
   suggestions.

C-c r f (Control-c r f): Field Search
   Perform a search on a particular field (Body, Location, Date, Subject, or
   Person).

C-c r q (Control-c r q): Query
   Bring up a query buffer to do a manual search.  This allows you to fill
   in several fields.  When the data is entered, hit C-c r v to view the
   results.

C-c r d (Control-c r d): Database change
   Prompt for a new database, from the indexes in remem-database-dir.  If
   more than one scope is active, it will prompt for which scope to replace
   as well.  Remem will restart with the new database(s) at the original
   size.

Left mouse-click on lineno: Show Suggestion
   Clicking the middle mouse-button on a line number is the same as doing
   C-c r # for that number.

Left mouse-click on a field: Search field
   Clicking the middle mouse button on a field does a search on that field.
   For example, if I middle-click on the subject field "RA presence paper
   stuff", a special query will be launched just on those words in other
   subject fields.  This is field dependent -- if I middle click on the
   username "tara", it does a query for other documents associated with her
   username.  Clicking on a date will work as soon as we start indexing
   dates, but it doesn't yet.

Middle or Right mouse-click: Keywords
   Clicking with the middle or right mouse button pops up keywords
   associated with this particular suggestion.  These are the top five
   words that led to the document being suggested, in order that they
   contributed.  These words will also be on your display line if you set
   remem-terminal-mode to t.

Resize window: Resize
   If you resize the *remem-display* window it will fill to the new size on
   the next update.  If you resize it too small, it will stop the RA (same
   as performing a remem-toggle).


### 5.  Logging

Because this is a research project, the emacs front-end logs generic data such as how often queries are generated, how many suggestions are followed, and how you rate the usefulness of followed suggestions. Logging is on by default, to turn off logging add the following to your .emacs or remem-custom.el file:

    (setq remem-log-p nil)

All logs are kept in the file ~/.remem-log-file, or wherever specified by the remem-logfile variable. Logs are kept local, though I will be sending out email asking for a copy of log-files so I can evaluate how the RA gets used. Such participation is voluntary, and I encourage you to use the RA even if you don't keep logging info.

Here's the complete list of information that gets logged:
     RA Version number
     timestamp
     Number of documents indexed per scope
     Scope name (e.g. "notes")
     Number of unique suggestions proposed
     Number of lines per scope displayed
     Ratings of followed suggestions
     Number of suggestions followed, their relevance scores (broken down by
          field type), docnums, and whether it was an automatic, mouse, or
          manual query

No information about the content of documents or suggestions is kept.


### 6.  Savant's document template system and other customizations

As of Savant v.2.00, the template structure is compiled instead of set in a file. It is much more powerful than previous versions and allows for Perl-style regular expressions to pull out fields, do filtering, etc.

This still has to be documented. It's pretty cool though :). Feel free to check the comments in the source-code -- they're pretty good. All the template stuff happens in RA/templates/conftemplates.c.

### 7.  What's New?

This is release v2.0, which is a huge rewrite.  Here are some changes:
     Back-end (Savant):
          Redesigned from the ground up and is much cleaner now.
          Now uses the Okapi algorithm for text
          Template-defined filters for different data types (e.g. simple
                           heuristics to remove sigs from email)
          Templates to recognize different queries
          Setable biases (weights) for fields of different document and query types
          More robust

     Front-end (Emacs RA):
          Redesigned from the ground up and is much cleaner now.
          Pretty Colors!  (And useful too -- they separate fields)
          Mouseable queries for different fields
          View keywords feedback
          More robust

See savant.h for a full changelog (yes, I really should break that out into
a "changelog" file).

### 8.  License

Some files and libraries in this distribution carry their own copyright
(notably the parsedate and zlib directories and libraries in savutil).  In
the future some plugins might carry different copyrights and licenses as
well as people write their own.  All other files and code is under the
following copyright and license:

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
