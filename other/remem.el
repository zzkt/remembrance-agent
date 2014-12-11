;remem.el -- implements emacs front-end for the remembrance agent
;
;All code included in versions up to and including 2.09:
;   Copyright (C) 2001 Massachusetts Institute of Technology.
;
;All modifications subsequent to version 2.09 are copyright Bradley
;Rhodes or their respective authors.
;
;Developed by Bradley Rhodes at the Media Laboratory, MIT, Cambridge,
;Massachusetts, with support from British Telecom and Merrill Lynch.
;
;This program is free software; you can redistribute it and/or modify
;it under the terms of the GNU General Public License as published by
;the Free Software Foundation; either version 2 of the License, or (at
;your option) any later version.  For commercial licensing under other
;terms, please consult the MIT Technology Licensing Office.
;
;This program may be subject to the following US and/or foreign
;patents (pending): "Method and Apparatus for Automated,
;Context-Dependent Retrieval of Information," MIT Case No. 7870TS. If
;any of these patents are granted, royalty-free license to use this
;and derivative programs under the GNU General Public License are
;hereby granted.
;
;This program is distributed in the hope that it will be useful, but
;WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;General Public License for more details.
;
;You should have received a copy of the GNU General Public License
;along with this program; if not, write to the Free Software
;Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
;USA.

;; Version info:
;;   Num    Date     Who                     Comments
;;   ------ -------- ----------------------- -----------------
;;   2.10   3/15/01  rhodes@alum.mit.edu     Fixed clobbering of C-xo bug that I introduced in
;;                                           2.09, plus fixed remem-query-extra-string-previous
;;                                           bug (should be remem-query-extra-text-string-previous)
;;                                           Thanks to Keith Amidon for these fixes.
;;   2.09   1/15/01  rhodes@alum.mit.edu     Added stuff that lets other programs customize remem 
;;                                           (e.g. jimminy). This includes remem-start-hook, remem-stop-hook,
;;                                           remem-query-oneshot-preamble-string, remem-query-preamble-string
;;                                           Added check for whether scrollbar-width is defined in XEmacs.
;;                                           Changed to GPL licence.
;;   2.08   2/29/00  rhodes@media.mit.edu    Made *remem-document-output* read-only, added 
;;                                           major-mode and retrieved document type to rating log.
;;                                           Toggling remem off and on while banging on the keys
;;                                           used to cause emacs to wedge, presubably because of sit-for
;;                                           instead of sleep-for. Fixed that (I think).
;;                                           Finally, it now defaults to printing hash marks for how
;;                                           relevant a hit is, rather than the numbers. Also can set it
;;                                           to not show hits below a certain threshold. Added the
;;                                           remem-mode-db-alist change database based on major mode.
;;                                           Added better multi-frame support (only put *remem-display* in
;;                                           one of them). Moved "C-cr" prefix to a customizable parameter.
;;   2.07   9/25/99  rhodes@media.mit.edu    Added help function C-crh, fixed(?) it so getting rid of 
;;                                           remem window respawns it, while getting rid of buffer kills it.
;;                                           Pop to *remem-display* after a field-query, so it won't update
;;                                           on you while you're looking.  Also added check for updated
;;                                           index files -- restart ra-retrieve if the index files
;;                                           have been modified.
;;   2.06   9/2/99   rhodes@media.mit.edu    Sanity check for database subdir existing
;;                                           Added remem-change-database, fixed remem-grab-query 
;;                                           for xemacs, added timestamping for logs,
;;                                           added a full-page query mode, changed all the quick-keys
;;                                           to start in C-cr, fixed "not leaving remem buffer" bug
;;   2.05   7/19/99  rhodes@media.mit.edu    Fixed mouse left-click for Xemacs, added template type
;;                                           output for savant, made a better date print format,
;;                                           added the filename to the default output format, and
;;                                           now allow a per-scope formatting through remem-format-alist
;;   2.04   6/16/99  rhodes@media.mit.edu    Added a "require timer" and "remem-grab-query" (C-cf)
;;   2.03   5/20/99  rhodes@media.mit.edu    Added some sanity checks
;;   2.02   4/8/99   rhodes@media.mit.edu    Added logging
;;   2.01   3/10/99  rhodes@media.mit.edu    Got rid of stupid "require jimminy" that broke it,
;;                                           fixed it for xemacs, and got rid of remem-mouse.el
;;   2.00   3/6/99   rhodes@media.mit.edu    Release 2.00

(provide 'remem)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CUSTOMIZATIONS
;;   These variables can (and probably should) be overridden for each user
;;   in their own .emacs file, or in a remem-custom.el file which gets loaded
;;   after remem.el does.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Required Customizations: 
;; You must set remem-prog-dir, remem-database-dir, and remem-scopes-list.
;; The defaults given are probably not correct for your individual databases
;; and scopes.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;
;; This is the directory where savant is located (fully expanded)
(defvar remem-prog-dir (expand-file-name "/usr/local/bin")
  "This is the directory where savant is located (fully expanded)")

;;;;;;;;;;;;;;;;
;; This is the base directory containing all of the database directories (fully expanded)
;; NOTE: This is a directory containing *directories*, which in turn contain index files.
(defvar remem-database-dir (expand-file-name "~/RA-indexes")
  "This is the base directory containing all of the database directories (fully expanded)")

;;;;;;;;;;;;;;;;
;; The scopes list...  This is a list in the form:
;;
;;  (scope1 scope2 scope3 ...)
;;
;; Where scope is of the form:
;;
;;  (DIRN NUM-LINES UPDATE-TIME QUERY-RANGE)
;;
;; DIRN is the subdirectory of remem-database-dir with the desired database 
;;             This is the name of a sub-directory in remem-database-dir
;; NUM-LINES is the number of lines that you want the scope to return (initially)
;; UPDATE-TIME is the time between scope updates (in seconds)
;; QUERY-RANGE number of lines around point that you want the scope to query on
(defvar remem-scopes-list '(("mail" 6 5 500)
                            ("notes" 2 5 500))
  "The list of scopes, where each scope is (DIRN NUM-LINES UPDATE-TIME QUERY-RANGE)")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Optional Customizations: 
;;   remem-log-original-suggestion, remem-terminal-mode, remem-logging-p, 
;;   remem-use-major-mode-templates, remem-non-r-number-keys and the font 
;;   customizations are set to reasonable defaults but can be overridden to fit 
;;   your tastes.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;
;; Whether or not to retrieve the original document or to make a copy for viewing
;; Useful for email-style files (narrowed), and when you want to subsequently edit 
;; the suggested file 
(defvar remem-load-original-suggestion nil
      "When seeing an entire suggestion, just go to the file rather than a copy")

;;;;;;;;;;;;;;;;
;; Turns on logging.  The default log-file is ~/.remem-log-file
(defvar remem-log-p t
  "If t, log when suggestions are followed to remem-logfile")
(defvar remem-logfile (expand-file-name "~/.remem-log-file")
  "File where remembrance agent info gets logged")

;;;;;;;;;;;;;;;;
;; Set to t if you want to use the major-mode variable to automatically parse
;; fields from your document, based on the template structure defined in 
;; Savant.
(defvar remem-use-major-mode-templates t
  "If t, send major-mode info to Savant to help templates")

;;;;;;;;;;;;;;;;
;; Set remem-non-r-number-keys to t if you want C-c1 through C-c9 to bring up
;; a suggestion.  These keys are already set to C-cr1 through C-cr9, but this
;; saves a keystroke.
(defvar remem-non-r-number-keys nil
  "If t, set C-c1 through C-c9 to bring up suggestions just like C-cr1 through C-cr9 do now")

;;;;;;;;;;;;;;;;
;; Set remem-terminal-mode to t if you want the keywords to show up on the 
;; screen rather than on mouse-click.  Otherwise, set to nil.
;; Actually, I take it back. This option is like the "close door" button on 
;; the elevator. You can play with it, but in reality it doesn't do a darned thing.
(defvar remem-terminal-mode t
  "Set to t if you want keywords shown. Not implemented right now.")

;;;;;;;;;;;;;;;;
;; Set remem-print-exact-relevance-p to t if you want numbers instead of plus signs
;; for the relevance score.
(defvar remem-print-exact-relevance-p nil)

;;;;;;;;;;;;;;;;
;; Set remem-print-even-bad-relevance-p to t if you want all suggestions shown,
;; regardless of how bad the relevance.
(defvar remem-print-even-bad-relevance-p nil)

;;;;;;;;;;;;;;;;
;; Set remem-print-exact-relevance-p to t if you want numbers instead of plus signs
;; for the relevance score.
(defvar remem-relevance-plus-plus-threshold 40) ;; Relevance score to get a "++" rating
(defvar remem-relevance-plus-threshold 30)      ;; Relevance score to get a "+" rating
(defvar remem-relevance-normal-threshold 10)    ;; Relevance score minimum before getting a "-" rating,
                                                ;; or no suggestion if remem-print-even-bad-relevance-p = nil

;;The Prefix used for RA commands. Default is control-c r
(defvar remem-command-prefix "\C-cr")

;;;;;;;;;;;;;;;;
;; Keys We want to start with before running the RA
(global-set-key (concat remem-command-prefix "t") 'remem-toggle)
(global-set-key (concat remem-command-prefix "h") 'remem-create-help-page)

;;;;;;;;;;;;;;;;
;; Mode aware changing
;; These three variables let you automatically change mode based on the *major-mode*
;; variable. For example, you could set them to these values:
;; (setq remem-mode-aware-changing t)
;; (setq remem-mode-db-alist (list (cons 'mail-mode (("mail" 5 10 500)))
;;                                 (cons 'rmail-mode (("mail" 5 10 500)))
;;                                 (cons 'latex-mode (("inspec" 5 10 500)))))
;; (setq remem-buffname-db-alist (list (cons "my-diary" (("daily-notes" 5 10 500)))
;;                                     (cons "my-homework" (("code" 5 10 500)))))
;;
;; The first value turns on mode-aware database selection.
;; The second specifies that when you visit a mail-mode or rmail-mode buffer, switch to the 
;;       "mail" database with the specified number of lines, seconds between updates and
;;       number of words looked at. In latex-mode it switches to the "inspec" database. 
;;       Any other mode doesn't change.
;; The third line is for buffer-specific changes. Here, the buffer named "my-dairy" gets
;;       "daily-notes" regardless of mode, and "my-homework" gets the database named "code".
;;       If you change databases in a specific buffer, your new selection becomes sticky for 
;;       the rest of the session.
;;
;; Now that wasn't so bad, was it? 

(defvar remem-mode-aware-changing nil)
(defvar remem-buffname-db-alist nil) ;;; Set in user customization if used
(defvar remem-mode-db-alist nil)  ;;; Set in user customization if used

;;;;;;;;;;;;;;;;
;; Font customizations.  
;; Every odd column
;; Fonts
;;
;; These are the colour customizations
;; colour for every alternating column
(make-empty-face 'remem-odd)
(make-empty-face 'remem-even)
(make-empty-face 'remem-hilite)      ;; current line
(make-empty-face 'remem-hilite2)     ;; current field
(make-empty-face 'remem-odd-scope)                    
(make-empty-face 'remem-even-scope)

(cond ((and (boundp 'hilit-background-mode)
           (equal hilit-background-mode 'dark))
       (set-face-foreground 'remem-odd  "Thistle")
       (set-face-foreground 'remem-even  "MediumSeaGreen")         ;; Every even column
       (set-face-foreground 'remem-odd-scope  "Goldenrod")         ;; Every odd scope (numbers)
       (set-face-foreground 'remem-even-scope  "CornflowerBlue")   ;; Every even scope (numbers)
       (set-face-foreground 'remem-hilite2 "OrangeRed")            ;; Selected line
       (set-face-foreground 'remem-hilite "Aquamarine")            ;; Current line (on mouse-over)
       ;; (set-face-font 'remem-hilite "-misc-fixed-bold-r-normal--15-140-75-75-c-90-iso8859-1")
       )
      (t
;       (set-face-foreground 'remem-odd  "ForestGreen")
       (set-face-foreground 'remem-odd  "Black")
       (set-face-foreground 'remem-even  "MediumBlue")         ;; Every even column
       (set-face-foreground 'remem-odd-scope  "Blue")     ;; Every odd scope (numbers)
       (set-face-foreground 'remem-even-scope  "DarkSlateGray")        ;; Every even scope (numbers)
       (set-face-foreground 'remem-hilite2 "Red")              ;; Selected line
       (set-face-foreground 'remem-hilite "Red")        ;; Current line (on mouse-over)
       ))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; END OF CUSTOMIZATIONS
;;   Beyond this point you're in the bowels of the
;;   code and you're on your own :)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defvar running-xemacs (string-match "XEmacs\\|Lucid" emacs-version))

;; xemacs hacks
(cond (running-xemacs
       (require 'itimer)
       (defun event-x-y (event)
         (cons (event-x event) (event-y event)))
       (defun run-at-time (time repeat function &rest args)
	 (apply 'start-itimer "remem-timer" function time repeat nil t args))
       (defun remem-cancel-timer (timer)
	 (cond (timer
                (delete-itimer timer))))
       (defun x-popup-menu (event list)
         (popup-menu list event))
       (defun frame-first-window (&rest args)
         (frame-highest-window args))
;       (defun sit-for-nodist (seconds)
;         (sit-for seconds t))
       (defun marker-position-nonil (marker)
         (let ((mark (marker-position marker)))
         (cond (mark mark)
               (t 0))))
       (require 'overlay))
      (t
       (require 'timer)
       (defvar scrollbar-width)          ;; To shut the compiler up
       (defvar scrollbar-height)         ;; To shut the compiler up
       (defvar mouse-track-click-hook)   ;; To shut the compiler up
       (defun remem-cancel-timer (timer) (cancel-timer timer))
       (defun marker-position-nonil (marker)
         (let ((mark (marker-position marker)))
         (cond (mark mark)
               (t 0))))
       (defun window-displayed-height (&optional window)
         (- (window-height window) 1))
;       (defun sit-for-nodist (seconds)
;         (sit-for seconds 0 t))
       (defun event-x-y (event)
         (posn-x-y (event-end event)))
       (defun event-window (event)
         (posn-window (event-end event)))
       (defun event-point (event)
         (posn-point (event-end event)))))
       



;;; --------------
;;; GLOBAL OPTIONS
;;; --------------

;; Other customizations

;; I've not tested remem-hide-display in awhile -- It might not work.
(defvar remem-hide-display nil
  "If t, don't display the remem window (but keep updating it anyway)")
(setq remem-hide-display nil)

;; I've not tested remem-autojump-suggestion in awhile -- It might not work.
(defvar remem-autojump-suggestion nil
  "If a value, jump automatically to that lineno when it's updated")
(setq remem-autojump-suggestion nil)

(defvar remem-keyword-field-start-number 28
  "Field where most relevant keywords are kept")

(defvar remem-sim-breakdown-field-start-number 29
  "Field where field similarity breakdown is kept")

(defvar remem-query-preamble-string ""
  "String that gets catenated before every Savant query command.
Change this to give special commands like setting biases and using
hand-set biases.")

(defvar remem-query-preamble-string-previous ""
  "Previous value of remem-query-preamble-string, used to keep track of when a query changes")

(defvar remem-query-oneshot-preamble-string ""
  "Like remem-query-preamble-string, but gets reset to the empty string after every query.")

(defvar remem-query-extra-text-string ""
  "String that gets catenated just before the query text (after the query command).")

(defvar remem-query-extra-text-string-previous ""
  "Previous value of remem-query-extra-text-string, used to keep track of when a query changes")

(defvar remem-query-oneshot-extra-text-string ""
  "Like remem-query-extra-text-string, but gets reset to the empty string after every query.")

;;; ---------------------
;;; DEFAULT LOCAL OPTIONS
;;; ---------------------

(defvar remem-format-default
  '((0 2 (field 0 mouse-face remem-hilite2) nil)                   ; Number
    (1 2 (face remem-even field 1) nil)                            ; sim
    (9 12 (face remem-odd field 9 mouse-face remem-hilite) nil)    ; person
    (8 30 (face remem-even field 8 mouse-face remem-hilite) nil)   ; subject
    (5 8 (face remem-odd field 5 mouse-face remem-hilite) remem-date-print-filter)    ; date
    (27 13 (face remem-even field 4 mouse-face remem-hilite) nil)   ; filename
;   (4 10 (face remem-even field 4 mouse-face remem-hilite) nil)   ; location
    (28 50 (face remem-odd field 28 mouse-face remem-hilite) nil))); keywords

(defvar remem-format-nokeys ;field-number field-width field-text-properties print-filter-function
  '((0 2 (field 0 mouse-face remem-hilite2) nil)                   ; Number
    (1 2 (face remem-even field 1) nil)                            ; sim
    (9 12 (face remem-odd field 9 mouse-face remem-hilite) nil)    ; person
    (8 30 (face remem-even field 8 mouse-face remem-hilite) nil)   ; subject
    (5 8 (face remem-odd field 5 mouse-face remem-hilite) remem-date-print-filter)    ; date
    (4 50 (face remem-even field 4 mouse-face remem-hilite) nil))) ; location

(defvar remem-format-inspec
  '((0 2 (field 0 mouse-face remem-hilite2) nil)                   ; Number
    (1 2 (face remem-even field 1) nil)                            ; sim
    (9 12 (face remem-odd field 9 mouse-face remem-hilite) nil)    ; person
    (5 12 (face remem-even field 5 mouse-face remem-hilite) remem-date-print-filter)    ; date
    (8 50 (face remem-odd field 8 mouse-face remem-hilite) nil)   ; subject
    (28 50 (face remem-even field 28 mouse-face remem-hilite) nil))); keywords

(defvar remem-format-jimminy
  '((0 2 (field 0 mouse-face remem-hilite2) nil)                   ; Number
    (1 2 (face remem-even field 1) nil)                            ; sim
    (9 12 (face remem-odd field 9 mouse-face remem-hilite) nil)    ; person
    (8 24 (face remem-even field 8 mouse-face remem-hilite) nil)   ; subject
    (5 8 (face remem-odd field 5 mouse-face remem-hilite) remem-date-print-filter)    ; date
    (28 50 (face remem-even field 28 mouse-face remem-hilite) nil))); keywords

(defvar remem-format remem-format-default)
(defvar remem-format-mail remem-format-default)

(defvar remem-format-alist nil)   ;;; Set in user customization if used

;;format list for the keyword field

(defvar remem-keyword-field 
  '(28 40 (face remem-even field 28)))

;;If remem-mode-db-alist is t:
;;   if buffname is in remem-buffname-db-alist, switch to scopes lists
;;   elseif major-mode is in remem-mode-db-alist, switch to scopes lists
;;   else don't change databases.
;;
;;   current buffer is automatically added to remem-buffname-db-alist if remem-change-database is called.

;;; ---------
;;; INTERNALS
;;; ---------

;; Note: these are the fields that savant returns, with one exception:
;; since the line number that savant returns may not correspond with
;; the line number in the remem display (duplicate lines, multiple scopes
;; in the same buffer), lineno is tacked on by remem-render-processed
;; num-relevance contains the savant line number (as well as the relevance)
;; Also, we're labeling keywords as in the body for search purposes.
;; These are all used for doing searches on fields when they're clicked.
(defvar remem-savant-field-names 
  '(lineno num-relevance docnum "!BODY: " "!LOCATION: " "!DATE: " "!TIME: " "!DAY" "!SUBJECT: " "!PERSON: "
           "" "" "" "" "" "" "" "" "" ""
           "" "" "" "" "" "" "" "" "!BODY:" ))

;; This is another helper list... to ensure that the interactive call asking for 1-5 returns a 1-5 in 
;; the field choice
(defvar remem-field-array
  '(("Body" . 3) ("Location" . 4) ("Date" . 5) ("Subject" . 8) ("Person" . 9)
    ("body" . 3) ("location" . 4) ("date" . 5) ("subject" . 8) ("person" . 9)))

(defvar remem-total-scope-lines 0)
(defvar remem-display-buffer-height 4)
(defvar remem-docnum-field 2) ; the third
(defvar remem-relevance-field 1) ; the second
(defvar remem-waiting-for-query-return-timeout 10)
(defvar remem-document-buffer-name "*remem-document-output*")
(defvar remem-query-buffer-name "*remem-query*")
(defvar remem-help-buffer-name "*remem-help*")
(defvar remem-old-C-x1 nil) ; holds function bound to C-x-1
(defvar remem-old-C-xo nil) ; holds function bound to C-x-0
(defvar remem-kind-of-query nil)  ; one of 'auto, 'mouse, 'manual-text, 'manual-page, 'manual-field, 
                                  ; or 'bypass for what the last query was
(defvar remem-versionstring-temp "")   ; Temporary global storage for versionstring info
(defvar remem-working-scope nil)   ; Global storage for scope being worked on right now 



;; Program Variables

(defvar remem-buffer-name "*remem-display*"
  "The name of the remembrance display buffer.")

(defvar remem-display-running nil
  "is the remem-display already running?")

(defvar remem-scopes nil)
(make-variable-buffer-local 'remem-scopes) ; each buffer has its own set of scopes

(defvar remem-scope-percentages nil)

(defvar remem-global-timer nil)

(defvar remem-buffers nil) ; a list of all buffers containing remem scopes
                           ;; this is necessary for the process filter

;; selection related

(defvar remem-selection-line nil)
(make-variable-buffer-local 'remem-selection-line)
(defvar remem-selection-line-contents "")
(make-variable-buffer-local 'remem-selection-line-contents)
(defvar remem-selection-field nil)
(make-variable-buffer-local 'remem-selection-field)

(defvar remem-selection-line-overlay nil)
(make-variable-buffer-local 'remem-selection-line-overlay)
(defvar remem-selection-field-overlay nil)
(make-variable-buffer-local 'remem-selection-field-overlay)

(defvar remem-last-followed-docnum nil)   ;; Docnum of document followed last

;;; ----------
;;; SCOPE DATA
;;; ----------

;; We keep the data about a remembrance agent together in 
;; the scope data structure

; [dir, lines, update time, range, proc, history, query, processed, in progress?, 
;  timer, raw, buffer, querycounter, Database info, Database modtime]

(defun remem-scope-directory (scope)
  (aref scope 0)) ; subdir of remem-database-dir
(defun remem-scope-number-lines (scope)
  (aref scope 1)) ; number of lines to display for this scope
(defun remem-scope-update-time (scope)  ; how often to update (0 = never)
  (let ((ut (aref scope 2)))
    (cond ((eq ut 0) nil)
          (t ut))))
(defun remem-scope-range (scope)
  (aref scope 3)) ; how many words to look at
(defun remem-scope-proc (scope)
  (aref scope 4)) ; the process running this agent
(defun remem-scope-history (scope)
  (aref scope 5)) ; a list of past queries -- currently disabled since it grows without bound.
(defun remem-scope-query (scope)
  (aref scope 6)) ; the current query -- also used to only initiate new queries (i.e. don't do anything when idle)
(defun remem-scope-processed (scope)
  (aref scope 7)) ; the results of the last query
(defun remem-scope-in-progress (scope)
  (aref scope 8)) ; is there a query in progress? How long until it times out?
(defun remem-scope-timer (scope)
  (aref scope 9)) ; timer object for this scope
(defun remem-scope-raw (scope)
  (aref scope 10)) ; the unprocessed results of the last query
(defun remem-scope-buffer (scope)
  (aref scope 11)) ; the buffer which contains this scope
(defun remem-scope-querycounter (scope)
  (aref scope 12)) ; number queries done in this scope since the last log checkpoint
(defun remem-scope-dbinfo (scope)
  (aref scope 13)) ; info on the database queried in this scope (version # and num docs)
(defun remem-scope-dbmodtime (scope)
  (aref scope 14)) ; modtime of the database queried in this scope (so we know to reset if it's been changed out from under us)

(defun remem-set-scope-directory (scope value)
  (aset scope 0 value)) ; subdir of remem-database-dir
(defun remem-set-scope-number-lines (scope value)
  (aset scope 1 value)) ; number of lines to display for this scope
(defun remem-set-scope-update-time (scope value)
  (aset scope 2 value)) ; how often to update (0 = never)
(defun remem-set-scope-range (scope value)
  (aset scope 3 value)) ; how many words to look at
(defun remem-set-scope-proc (scope value)
  (aset scope 4 value)) ; the process running this agent
(defun remem-set-scope-history (scope value)
  (aset scope 5 value)) ; a list of past queries -- currently disabled since it grows without bound.
(defun remem-set-scope-query (scope value)
  (aset scope 6 value)) ; the current query -- also used to only initiate new queries (i.e. don't do anything when idle)
(defun remem-set-scope-processed (scope value)
  (aset scope 7 value)) ; the results of the last query
(defun remem-set-scope-in-progress (scope value)
  (aset scope 8 value)) ; is there a query in progress? How long until it times out?
(defun remem-set-scope-timer (scope value)
  (aset scope 9 value)) ; timer object for this scope
(defun remem-set-scope-raw (scope value)
  (aset scope 10 value)) ; the unprocessed results of the last query
(defun remem-set-scope-buffer (scope value)
  (aset scope 11 value)) ; the buffer which contains this scope
(defun remem-set-scope-querycounter (scope value)
  (aset scope 12 value)) ; number queries done in this scope since the last log checkpoint
(defun remem-set-scope-dbinfo (scope value)
  (aset scope 13 value)) ; info on the database queried in this scope (version # and num docs)
(defun remem-set-scope-dbmodtime (scope)
  (aset scope 14 value)) ; modtime of the database queried in this scope (so we know to reset if it's been changed out from under us)

(defun remem-decrement-scope-in-progress (scope)
  (aset scope 8 (and (aref scope 8)
                     (> (aref scope 8) 0)
                     (- (aref scope 8) 1)))) ; if not nil or zero, subtract 1, else nil

;;; Debugging function
;;; (defun remem-break-here () nil)

;;; --------
;;; QUERYING
;;; --------

;;; all query functions should call this

; (defun remem-initiate-query (scope &optional query nomodep)
;   (cond ((and (remem-scope-in-progress scope)
;               (not (y-or-n-p
;                     "There is already a query in progress. Use this one instead? "))))
;         (t ;This was originally giving me the problem of destroying all minibuffer messages, so I
; 	   ;blocked it out.
; 	;(message nil) ; don't leave the y-or-n question hanging around
	 	 
; 	 (cond (query ; is there a new query here? then make a note of it
; 		(remem-set-scope-query scope query)
; ;;;		(remem-set-scope-history scope (cons query (remem-scope-history scope)))
;                 ))
; 	 (let ((query-text
;                 (concat
;                  "query " (format "%d" (+ 6 (* 2 (remem-scope-number-lines scope)))) "\n"
;                  (cond ((not nomodep)
;                         (concat "EMACS REMEMBRANCE QUERY MODE: " (format "%s" major-mode) "\n"))
;                        (t nil))
;                  (remem-scope-query scope) "\n"
;                  "\004")))
             
;            (set-text-properties 0 (length query-text) nil query-text)  ;; Get rid of emacsisms
;            (if (get-buffer "*remem-log*")
;                (print query-text (get-buffer "*remem-log*")))

;            (set-process-filter (remem-scope-proc scope) 'remem-process-filter) 
; 					; we may have set this to the display filter elsewhere
; 	   (process-send-string (remem-scope-proc scope)
; 				query-text)
; 	   (remem-set-scope-in-progress scope remem-waiting-for-query-return-timeout)))))



;;;Same thing as above without the yes or no questions and progress check
(defun remem-initiate-query-nonverbose (scope &optional query use-modep use-preambles use-extra-text)
  (cond ((remem-restart-on-outdated-index scope))
        (t
         (cond (query ; is there a new query here? then make a note of it
                (remem-set-scope-query scope query)
                (setq remem-query-extra-text-string-previous remem-query-extra-text-string)
                (setq remem-query-preamble-string-previous remem-query-preamble-string)
                ;;	 (remem-set-scope-history scope (cons query (remem-scope-history scope)))
                ))
         (let ((query-text
                (concat remem-query-oneshot-preamble-string remem-query-preamble-string 
                 "query " (format "%d" (+ 6 (* 2 (remem-scope-number-lines scope)))) "\n"
                 (cond (use-modep
                        (concat "EMACS REMEMBRANCE QUERY MODE: " (format "%s" major-mode) "\n")))
                 (cond (use-preambles remem-query-oneshot-extra-text-string))
                 (cond (use-extra-text remem-query-extra-text-string))
                 (remem-scope-query scope) "\n"
                 "\004")))
           
           (setq remem-last-query-mode (format "%s" major-mode))
           (set-text-properties 0 (length query-text) nil query-text)  ;; Get rid of emacsisms
           (if (get-buffer "*remem-log*")
               (print query-text (get-buffer "*remem-log*")))
           
           (remem-set-scope-querycounter scope (+ (remem-scope-querycounter scope) 1))  ;; Count queries for logs
           (cond ((>= (remem-scope-querycounter scope) 25)
                  (remem-log-checkpoint-scope scope 'at-25)))
           
           (set-process-filter (remem-scope-proc scope) 'remem-process-filter) 
					; we may have set this to the display filter elsewhere
           (process-send-string (remem-scope-proc scope)
                                query-text)
           (remem-set-scope-in-progress scope remem-waiting-for-query-return-timeout))
         (setq remem-query-oneshot-preamble-string "")
         (setq remem-query-oneshot-extra-text-string ""))))

(defun remem-default-scope-list ()
  "Returns the remem-scopes for the remem buffer that is currently defaulted"
  (or remem-scopes               ; first one in the active buffer
      (and remem-buffers (buffer-live-p (car remem-buffers))
	   (save-excursion
             (set-buffer (car remem-buffers))
             remem-scopes))))

(defun remem-default-scope ()
  "Returns the scope that is currently defaulted"
  (let ((default-scopes (remem-default-scope-list)))
    (cond (default-scopes (car default-scopes))
          nil)))

(defun remem-all-scopes ()
  "Return all scopes in all remem buffers"
  (apply 'append 
         (mapcar '(lambda (buf)
                    (cond ((buffer-live-p buf)
                           (set-buffer buf)
                           remem-scopes)      ; a buffer local variable
                          (t nil)))
                 remem-buffers)))

;; run a query in the default scope
; (defun remem-query (query)
;   (interactive "sQuery on text:")
;   (let ((scope (remem-default-scope)))
;     (cond (scope (remem-initiate-query scope query)
;                  (message "Query initiated.")
;                  (setq remem-kind-of-query 'manual-text))
;           (t (message "No active remembrance agent.")))))


(defun remem-last-several-words (count)
  "String from count words back to current point.  (word = 5 chars)"
  ;; If we're in a summary mode (e.g. RMAIL-summary, VM's "mailbox Summary", or 
  ;; gnus "*Summary alt.sweedish.cheff.bork.bork.bork*)" then
  ;; grab the query from where we really should be.
  (let ((orig-window (get-buffer-window (current-buffer)))
        (end) (beg) (retval) (realbufname))
    (save-excursion
      (cond ((string-match "-summary" (buffer-name (current-buffer)))
             (setq realbufname (substring (buffer-name (current-buffer))
                                          0
                                          (string-match "-summary" (buffer-name (current-buffer))))))
            ((string-match " Summary" (buffer-name (current-buffer)))
             (setq realbufname (substring (buffer-name (current-buffer))
                                          0
                                          (string-match " Summary" (buffer-name (current-buffer))))))
            ((string-match "*Summary " (buffer-name (current-buffer)))
             (setq realbufname "*Article*")))

      (cond ((and realbufname (get-buffer-window realbufname))
             (select-window (get-buffer-window realbufname))))

                                        ; place start of sample count words before current position
                                        ; or as far back as possible
      (setq beg (- (point) (* 5 count)))
      (if (< beg (point-min)) 
          (setq beg (point-min)))
      ; and place end of sample count words ahead of beginning
      (setq end (+ beg (* 5 count)))
      (if (> end (point-max)) 
	  (setq end (point-max)))
      (setq retval (concat " " (buffer-substring beg end)))
;      (while (string-match "\n\\.\n" retval)
;        (setq retval (replace-match "\n .\n" t t retval)))
      )
    (select-window orig-window)
    (setq remem-debug-retval retval)
    retval))

(defun remem-query-now ()
  "Updates the results shown by running a query NOW.  Bypasses the timer"
  (interactive)
  (mapcar (lambda (scope)
            (cond ((not (remem-scope-proc scope)))
                  ((string-match "^Minibuf" (buffer-name (current-buffer))))
                  ((get-buffer-window remem-buffer-name t)
                   (cond ((string-match (buffer-name (current-buffer))
                                        remem-query-buffer-name)
                          (remem-log-checkpoint (remem-default-scope-list) 'manual-page-query)
                          (setq remem-kind-of-query 'manual-page))
                         (t (setq remem-kind-of-query 'now)))
                   (let ((exheight (window-displayed-height (get-buffer-window remem-buffer-name t))))
                     (remem-redistribute-scopes exheight)
                     (remem-initiate-query-nonverbose scope 
                                                      (remem-last-several-words (remem-scope-range scope))
                                                      remem-use-major-mode-templates
                                                      t  ;; Use preambles
                                                      t  ;; Use extra text
                                                      )))
                  (t
                   (remem-initiate-query-nonverbose scope 
                                                    (remem-last-several-words (remem-scope-range scope))
                                                    remem-use-major-mode-templates
                                                    t  ;; Use preambles
                                                    t  ;; Use extra text
                   ))))
          (remem-default-scope-list)))

 
(defun remem-query-on-keywords (keywords)
  (mapcar (lambda (scope)
	    (cond ((not (remem-scope-proc scope)))
		  ((string-match "^Minibuf" (buffer-name (current-buffer))))
		  ((get-buffer-window remem-buffer-name t)
		   (let ((exheight (window-displayed-height (get-buffer-window remem-buffer-name t))))
		     (remem-redistribute-scopes exheight)
		     (remem-initiate-query-nonverbose scope keywords nil)
		     (remem-cancel-timer (remem-scope-timer scope))
		     (remem-set-scope-timer scope
					    (run-at-time 10 (remem-scope-update-time scope) 'remem-around-point scope))
		     ))
		  (t
		   (remem-initiate-query-nonverbose scope keywords nil)
		   (remem-cancel-timer (remem-scope-timer scope))
		   (remem-set-scope-timer scope
					  (run-at-time 10 (remem-scope-update-time scope) 'remem-around-point scope))
		   )))
          (remem-default-scope-list)))


;;; This never seems to be called...
; (defun remem-query-on-field-mouse (field field-text)
;   (let ((query
; 	 (concat "EMACS REMEMBRANCE FIELD QUERY:\n"
;                  (elt remem-savant-field-names field)
; 		 field-text)))
;     (setq remem-kind-of-query 'mouse)
;     (remem-log-checkpoint (remem-default-scope-list) 'mouse-field-query)
;     (mapcar (lambda (scope)
; 	      (cond ((not (remem-scope-proc scope)))
; 		    ((string-match "^Minibuf" (buffer-name (current-buffer))))
; 		    ((get-buffer-window remem-buffer-name t)
; 		     (let ((exheight (window-displayed-height (get-buffer-window remem-buffer-name t))))
; 		       (remem-redistribute-scopes exheight)
; 		       (remem-initiate-query-nonverbose scope query nil)
; 		       (remem-cancel-timer (remem-scope-timer scope))
; 		       (remem-set-scope-timer scope
; 					      (run-at-time 10 (remem-scope-update-time scope) 'remem-around-point scope))
; 		       ))
; 		    (t
; 		     (remem-initiate-query-nonverbose scope query nil)
; 		     (remem-cancel-timer (remem-scope-timer scope))
; 		     (remem-set-scope-timer scope
; 					    (run-at-time 10 (remem-scope-update-time scope) 'remem-around-point scope))
; 		     )))
;             (remem-default-scope-list))))




(defun remem-query-on-field (field field-text query-type)
  (let ((query
	 (concat "EMACS REMEMBRANCE FIELD QUERY:\n"
                 (elt remem-savant-field-names field)
		 field-text)))
    (setq remem-kind-of-query query-type)
    (remem-log-checkpoint (remem-default-scope-list) (concat (symbol-name query-type) "-query"))
    (mapcar (lambda (scope)
	      (cond ((not (remem-scope-proc scope)))
		    ((string-match "^Minibuf" (buffer-name (current-buffer))))
		    ((get-buffer-window remem-buffer-name t)
		     (let ((exheight (window-displayed-height (get-buffer-window remem-buffer-name t))))
		       (remem-redistribute-scopes exheight)
		       (remem-initiate-query-nonverbose scope query nil)
		       (remem-cancel-timer (remem-scope-timer scope))
		       (remem-set-scope-timer scope
					      (run-at-time 10 (remem-scope-update-time scope) 'remem-around-point scope))
		       ))
		    (t
		     (remem-initiate-query-nonverbose scope query nil)
		     (remem-cancel-timer (remem-scope-timer scope))
		     (remem-set-scope-timer scope
					    (run-at-time 10 (remem-scope-update-time scope) 'remem-around-point scope))
		     )))
            (remem-default-scope-list))))  


(defun remem-grab-query (fieldname field-text)
  "Run a query on a given field"
  (interactive
   (list (let (val)
	   (setq val (completing-read "Which Field? (Body Location Date Subject Person): "
				      remem-field-array
				      'consp t))
	   (cdr (assoc val remem-field-array)))
	 (read-string "Enter Text: ")))
  (remem-query-on-field fieldname field-text 'manual-field)
  (pop-to-buffer remem-buffer-name))


(defun remem-insert-query-page-text (fieldstring)
  "Insert a string as the next field for a query page, with appropriate read-only font"
  (let ((len (length fieldstring)))
    (cond ((not running-xemacs)
           (add-text-properties 0 len '(read-only t face remem-odd-scope) fieldstring)
           (add-text-properties (- len 1) len '(rear-nonsticky t) fieldstring)
           (insert fieldstring))
          (t        ;; Xemacs has a broken rear-nonsticky, so we use extents instead.
           (let ((extent (make-extent
                          (point)
                          (progn (insert fieldstring) (point)))))
             (set-extent-face extent 'remem-odd-scope)
             (set-extent-property extent 'read-only t)
             (cond ((= (point-min) (point))
                    (set-extent-property extent 'start-open nil)
                    (set-extent-property extent 'end-open t))
                   (t 
                    (set-extent-property extent 'start-open t)
                    (set-extent-property extent 'end-open t))))))))

(defun remem-create-query-page ()
  "Create a nice form-like buffer to fill in a manual remem query"
  (interactive)
  (remem-log-checkpoint (remem-default-scope-list) 'new-query-page)
   (cond ((get-buffer remem-query-buffer-name)
         (kill-buffer remem-query-buffer-name)))
  (pop-to-buffer (get-buffer-create remem-query-buffer-name))   ;; Make/get the query buffer
  (setq major-mode 'remem-query-mode)
  (kill-region (point-min) (point-max))                         ;; Clear the buffer
  (remem-insert-query-page-text "Remem Query Form:\nEnter one or more fields below and hit C-crv to view the results\n\nSubject: ")
  (remem-insert-query-page-text "\nPerson: ")
  (remem-insert-query-page-text "\nLocation: ")
  (remem-insert-query-page-text "\nDate: ")
  (remem-insert-query-page-text "\nBody: ")
  (goto-char (point-min))
  (end-of-line 4))

(defun remem-create-help-page ()
  "Display the help page for the RA"
  (interactive)
  (let ((help-string 
         (cond ((equal remem-command-prefix "\C-cr") 
                "Remembrance Agent Commands:\n\nC-c r t (Control-c r t):            Toggle Remem\nC-c r v (Control-c r v):            View updated results\nC-c r # (Control-c r <number>):     Show Suggestion\nC-c r r # (Control-c r r <number>): Rate this document\nC-c r f (Control-c r f):            Field Search\nC-c r q (Control-c r q):            Query\nC-c r d (Control-c r d):            Database change\nLeft mouse-click on lineno:         Show Suggestion\nLeft mouse-click on a field:        Search field\nMiddle or Right mouse-click:        Keywords\nResize window:                      Resize\n")
               (t (concat "Remembrance Agent Commands:\n\n"
                          remem-command-prefix
                          "t:            Toggle Remem\n"
                          remem-command-prefix
                          "v:            View updated results\n"
                          remem-command-prefix
                          "#:     Show Suggestion number #\n"
                          remem-command-prefix
                          "r#: Rate document number #\n"
                          remem-command-prefix
                          "f:            Field Search\n"
                          remem-command-prefix
                          "q:            Query\n"
                          remem-command-prefix
                          "d:            Database change\nLeft mouse-click on lineno:         Show Suggestion\nLeft mouse-click on a field:        Search field\nMiddle or Right mouse-click:        Keywords\nResize window:                      Resize\n")))))
                
  (pop-to-buffer (get-buffer-create remem-help-buffer-name))
  (kill-region (point-min) (point-max))
  (insert help-string)
  (goto-char (point-min))))

(defun remem-switch-to-mode-specific-db ()
  "Switch database to the one specified in remem-mode-db-alist"
  (let ((new-remem-scopes-list (or (cdr (assoc (buffer-name (current-buffer)) remem-buffname-db-alist))
                                   (cdr (assoc major-mode remem-mode-db-alist)))))
    (cond ((and new-remem-scopes-list
                (not (equal remem-scopes-list new-remem-scopes-list)))
           (setq remem-scopes-list new-remem-scopes-list)
           (remem)))))
         
(defun remem-around-point (scope)
  (remem-decrement-scope-in-progress scope)
  (cond ((not (remem-scope-proc scope))) ; if the process is dead, abort 
        ((remem-scope-in-progress scope))
					; if we're already running a query here, don't supercede
        ((string-match "^*remem-" (buffer-name (current-buffer))))
					; don't search results, the display area, or *remem-log*
	((string-match "*Minibuf" (buffer-name (current-buffer))))
					; don't search the Minibuffer

        ((and remem-mode-aware-changing (remem-switch-to-mode-specific-db)))  ;; Switch to new DB if major-mode change

	((get-buffer-window remem-buffer-name t)
	 (let ((exheight (window-displayed-height (get-buffer-window remem-buffer-name t)))
               (query (remem-last-several-words (remem-scope-range scope))))
	   (remem-redistribute-scopes exheight)                               ;; Handle resizes
           (cond ((or (not (string= query (remem-scope-query scope)))         ;; Only query if things have changed
                      (not (equal remem-query-oneshot-preamble-string ""))
                      (not (equal remem-query-oneshot-extra-text-string ""))
                      (not (string= remem-query-preamble-string remem-query-preamble-string-previous))
                      (not (string= remem-query-extra-text-string remem-query-extra-text-string-previous)))
                  (setq remem-kind-of-query 'auto)
                  (remem-initiate-query-nonverbose scope query remem-use-major-mode-templates t t)))))
        (t ; actually remem!
         (let ((query (remem-last-several-words (remem-scope-range scope))))
           (cond ((or (not (string= query (remem-scope-query scope)))         ;; Only query if things have changed
                      (not (equal remem-query-oneshot-preamble-string ""))
                      (not (equal remem-query-oneshot-extra-text-string ""))
                      (not (string= remem-query-preamble-string remem-query-preamble-string-previous))
                      (not (string= remem-query-extra-text-string remem-query-extra-string-previous)))
                  (setq remem-kind-of-query 'auto)
                  (remem-initiate-query-nonverbose scope query remem-use-major-mode-templates t t)))))))

;;; Function to keep the number of lines displayed by each scope proportional to the original distribution
(defun remem-redistribute-scopes (newtotal)
  "This function changes the number of display lines shown by each scope so that the remem window is filled.  Makes use of the percentage property to retain the same proportions across scopes."
  (cond ((buffer-live-p (get-buffer remem-buffer-name))
         (setq remem-total-scope-lines 0)
         (save-excursion
           (set-buffer remem-buffer-name)
           (remem-add-scope-lines remem-scopes)
           (cond ((not (eq remem-total-scope-lines newtotal))
                  (remem-log-checkpoint remem-scopes 'resizing)
                  (mapcar (lambda (x)
                            (remem-set-scope-number-lines x (round (* newtotal
                                                                      (cdr (assq (remem-scope-proc x) 
                                                                                 remem-scope-percentages))))))
                          remem-scopes)))))))
  
;;; The total number of lines displayed by all scope scopes in a list
(defun remem-add-scope-lines (scope-list)
  (cond ((null (car scope-list)))
	(t
	 (setq remem-total-scope-lines (+ (remem-scope-number-lines (car scope-list)) remem-total-scope-lines))
	 (remem-add-scope-lines (cdr scope-list)))))


;;; -------
;;; DISPLAY
;;; -------

;; accessors
;; get the format for column n
(defun remem-column-format (n)
  (elt remem-format n))

;; get the field number for column n
(defun remem-column-field (n)
  (elt (remem-column-format n) 0))

;; get the column width for column n
(defun remem-column-width (n)
  (elt (remem-column-format n) 1))

;; get the text properties for column n
(defun remem-column-props (n)
  (elt (remem-column-format n) 2))

;; get the print filter for column n
(defun remem-column-printfilter (n)
  (elt (remem-column-format n) 3))

;; return the column associated with a given field
(defun remem-field-column (n)
  (let ((i 0))
    (while (and (< i (length remem-format))
		(not (= (remem-column-field i) n)))
      (setq i (+ 1 i)))
    i))

(defun remem-enlarge-remem-display (num-lines)
  (interactive "nNumber Lines to enlarge (negative to shrink): ")
  (let ((current-window (get-buffer-window (current-buffer))))
    (select-window (get-buffer-window remem-buffer-name))
    (enlarge-window num-lines)
    (select-window current-window)))

(defun remem-enlarge-remem-display-by-1 ()
  (remem-enlarge-remem-display 1))

(defun remem-shrink-remem-display-by-1 ()
  (remem-enlarge-remem-display -1))

(defun remem-substring-equal (str1 start1 end1 str2 start2 end2 &optional ignore-case)
  (let ((compstr1 (substring str1 start1 end1))
        (compstr2 (substring str2 start2 end2)))
    (cond (ignore-case
           (setq compstr1 (downcase compstr1))
           (setq compstr2 (downcase compstr2))))
    (string-equal compstr1 compstr2)))

(defun remem-date-print-filter (datestring)
  (let ((monthname nil)
        (month nil)
        (day nil)
        (year nil))

          ;; Wed, 20 Mar 1996 19:42:37 -0500
          ;; Wed, 16 Jul 97 10:18:20 est
    (cond ((string-match "\\([0-9]+\\) +\\([A-Z][a-z][a-z]\\) +\\([0-9][0-9][0-9]?[0-9]?\\)" datestring)
           (setq day (substring datestring (match-beginning 1) (match-end 1)))
           (setq monthname (substring datestring (match-beginning 2) (match-end 2)))
           (setq year (substring datestring (match-beginning 3) (match-end 3))))

          ;; Mon Dec  4 11:28:24 -0500 1995
          ((string-match "[A-Z][a-z][a-z] +\\([A-Z][a-z][a-z]\\) +\\([0-9]+\\) +[0-9][0-9]?:[0-9][0-9]:[0-9][0-9] +[-0-9]* *\\([0-9][0-9][0-9][0-9]\\)"
                         datestring)
           (setq monthname (substring datestring (match-beginning 1) (match-end 1)))
           (setq day (substring datestring (match-beginning 2) (match-end 2)))
           (setq year (substring datestring (+ (match-beginning 3) 2) (match-end 3))))

          ;; September 3, 1985
          ((string-match "\\([A-Z][a-z][a-z]\\)[a-z]* +\\([0-9]+\\) *, *\\([0-9][0-9][0-9][0-9]\\)" datestring)
           (setq monthname (substring datestring (match-beginning 1) (match-end 1)))
           (setq day (substring datestring (match-beginning 2) (match-end 2)))
           (setq year (substring datestring (+ (match-beginning 3) 2) (match-end 3)))))

    (cond (monthname
           (cond ((remem-substring-equal monthname 0 2 "Jan" 0 2 t)
                  (setq month "01"))
                 ((remem-substring-equal monthname 0 2 "Feb" 0 2 t)
                  (setq month "02"))
                 ((remem-substring-equal monthname 0 2 "Mar" 0 2 t)
                  (setq month "03"))
                 ((remem-substring-equal monthname 0 2 "Apr" 0 2 t)
                  (setq month "04"))
                 ((remem-substring-equal monthname 0 2 "May" 0 2 t)
                  (setq month "05"))
                 ((remem-substring-equal monthname 0 2 "Jun" 0 2 t)
                  (setq month "06"))
                 ((remem-substring-equal monthname 0 2 "Jul" 0 2 t)
                  (setq month "07"))
                 ((remem-substring-equal monthname 0 2 "Aug" 0 2 t)
                  (setq month "08"))
                 ((remem-substring-equal monthname 0 2 "Sep" 0 2 t)
                  (setq month "09"))
                 ((remem-substring-equal monthname 0 2 "Oct" 0 2 t)
                  (setq month "10"))
                 ((remem-substring-equal monthname 0 2 "Nov" 0 2 t)
                  (setq month "11"))
                 ((remem-substring-equal monthname 0 2 "Dec" 0 2 t)
                  (setq month "12")))))
    (cond ((and day month year)
           (cond ((= (length year) 4) (setq year (substring year 2 4))))
           (cond ((< (length day) 2) (setq day (concat "0" day))))
           (cond ((< (length month) 2) (setq month (concat "0" month))))
           (concat month "/" day "/" year))
          (t 
           datestring))))

(defun remem-format-line (line format-list scope &optional extra-format)
  "takes a list of fields, extracts the desired ones (specified in
   format-list), and inserts them into the current buffer with the
   desired spacing. The point should be at the beginning of a line."
  ;(insert (format "%S   " fields))
;  (print fields (get-buffer "*scratch*"))
  (let ((fields (car line))
	(scope-marker (car (cdr (cdr line))))
        (remem-relevance-score 100))
    (if (stringp fields)
	(insert fields)
      (mapcar '(lambda (format)
		 (let ((string (elt fields (elt format 0)))
		       (width (elt format 1))
                       (printfilter (elt format 3))
		       (props (append (list 'scope scope 
					    'docnum (string-to-int (elt fields remem-docnum-field))
                                            'relevance (elt fields remem-relevance-field)
					    'keywords (elt fields remem-keyword-field-start-number)
                                            'sims-breakdown (elt fields remem-sim-breakdown-field-start-number)
					    'orig-source (elt fields (elt format 0)))
				      (elt format 2)))
		       (old-point (point)))


		   ;;; This is specifically for the relevance field
		   (cond ((= (elt format 0) 0)
			  (if (= 0 scope-marker)
			      (setq props (append (list 'face 'remem-even-scope) props))
			    (setq props (append (list 'face 'remem-odd-scope) props))))
			 ((= (elt format 0) 1)
			 ;;;funny bug where the
                         ;;;"lineno    relevance" field looked like
		         ;;;"^Jlineno    relevance"  This takes care of the newline
			  (if (string-match "\n" string)
			      (setq string (substring string 1)))
			 ;;;show only the relevance part
			  (setq string (substring string 6))
                          (setq remem-relevance-score (string-to-number string))
                          
                          (cond ((not remem-print-exact-relevance-p)
                                 (setq string 
                                       (cond ((or (> remem-relevance-score remem-relevance-plus-plus-threshold)
                                                  (= remem-relevance-score 0))  ; if 0, it's actually 1.00
                                              "++")
                                             ((> remem-relevance-score remem-relevance-plus-threshold) "+ ")
                                             ((> remem-relevance-score remem-relevance-normal-threshold) "  ")
                                             (t "- ")))))
                          ))

                   ;;; Do the print filter, if there is one
                   (cond ((or remem-print-even-bad-relevance-p 
                              (not (eq remem-kind-of-query 'auto))
                              (= remem-relevance-score 0)         ; if 0, it's actually 1.00
                              (> remem-relevance-score remem-relevance-normal-threshold))
                          (cond (printfilter
                                 (setq string (eval (list printfilter string)))))
                          
                          (cond ((< width (length string))
                                 (insert (substring string 0 width))) ; truncate
                                (t
                                 (cond (string (insert string)))
                                 (insert-char ?\  (- width (length string))))) ; or pad
                          (add-text-properties old-point (point) props)
                          (insert-char ?\  1))
                         ((= (elt format 0) 1)
                          (insert "-  No suggestion")))
                   ))
              format-list))
    (insert-char ?\n 1 t)))
  
  ;; format the processed results from each scope, removing duplicate lines
  ;; Duplicate lines are only based on database document number
  ;; This function contains a hack: it assumes that the first field contains the lineno
(defun remem-render-processed ()
  (setq inhibit-read-only t)
  (erase-buffer)
  (let ((unique-lines nil) (overall 0) (scope-marker 0))
    (mapcar
     '(lambda (scope)
        (let ((i 0) (used 0) (allotted (remem-scope-number-lines scope))
	      (available (length (remem-scope-processed scope)))
	      (processed (remem-scope-processed scope)))
	  (if (eq scope-marker 0)             ;use 1's and 0's to colour scopes differently
	      (setq scope-marker 1)
	    (setq scope-marker 0))	  
	  (while (< used allotted)
            (cond
	     ((>= i available) ; we've exhausted the responses    
	      (setq used (+ used 1))
	      (setq overall (+ overall 1))
	      (setq unique-lines 
		    (cons (list (format "%d No suggestion." overall) scope scope-marker) ; this assumes lines start with the lineno...
			  unique-lines)))
	     ((stringp (elt processed i)) ;; "" is not a valid response
	      (setq i (+ i 1)))
	      ( ; check for uniqueness using the docnum

	      (not (member (elt (elt processed i) (- remem-docnum-field 1))
			   ;; this mapcar creates a list of all of the docnums of the lines in unique-lines
			   ;; that are from the same database. Docnums from different databases are not
			   ;; comparable
			   (mapcar '(lambda (e)
				      (cond ((stringp (car e)) nil) ; ignore "No Suggestion" lines
					    ((not (equal (remem-scope-directory scope)
							 (remem-scope-directory (elt e 1)))) nil) ; can't be the same if dif databases
					    (t (elt (car e) remem-docnum-field))))
				   unique-lines)))
	      ;; a new line, add it
	      (setq used (+ used 1))
	      (setq overall (+ overall 1))
	      (setq unique-lines 
		    (cons (list (cons (int-to-string overall) (elt processed i)) scope scope-marker) 
		  ;; add the lineno to the beginning. see comments for remem-savant-field-names
			  unique-lines))
	      (setq i (+ i 1)))
	     (t ; a duplicate
	      (setq i (+ i 1)))))))
     remem-scopes)
    (mapcar '(lambda (line) 
               (let ((format-for-line (assoc (remem-scope-directory (car (cdr line)))
                                             remem-format-alist)))
                 (cond ((not format-for-line)
                        (setq format-for-line remem-format-default))
                       (t (setq format-for-line (car (cdr format-for-line)))))
                 (remem-format-line line format-for-line (elt line 1))))
            (reverse unique-lines)))
  (setq inhibit-read-only nil))


;;; ------------------
;;; PROCESSING RESULTS
;;; ------------------

(defun remem-double-newline (string)
  "looks for a double newline.  Used to be used for finding end of 
   a query result but now use remem-period-on-line-by-itself."
  (let* ((dn nil)
	 (len (- (length string) 1))
	 (i len))
    (while (> i 0)
      (cond ((not (= (aref string (- i 1)) 10))
	     (setq i (- i 2)))
	    ((= (aref string i) 10)
	     (setq dn t)
	     (setq i 0))
	    (t
	     (setq i (- i 1)))))
    dn))

(defun remem-period-on-line-by-itself (string)
  "looks for a period on a line by itself.  used to delimit end of query result."
  (cond ((string-match "\n\\.\n" string))
        ((string-match "^.\n" string))
        (t nil)))

(defun remem-split-string (string split-char eat)
  "splits a <split-char> delimited string into a list of fields
   eats the delimiter if eat"
  (let ((i 1) (last-match 0) (fields nil))
    (while (< i (length string))
      (cond ((eq (elt string i) split-char)
	     (setq fields (cons (substring string last-match 
					   (if eat i (+ i 1))) fields))
	     (setq last-match (+ i 1))))
      (setq i (+ i 1)))
    ; the field after the last delimiter
    (setq fields (cons (substring string last-match) fields)) 
    (reverse fields))) ; we cons'ed it up backwards

(defun remem-split-savant-output (string)
  "Splits into lines and pipe-delimited fields"
  (delete nil (mapcar '(lambda (s) (if (or (string= s "") (string= s "\n") (string= s "."))
				       nil
				     (remem-split-string s ?\| t)))
		      (remem-split-string string ?\n t))))

;; this should only be used in the buffer containing the scope to be sorted
(defun remem-sort-scope (&optional field scope)
  (interactive)
  (if (not field) (setq field remem-selection-field))
  (if (not scope) (setq scope (car remem-scopes)))
  ;; sort-columns eats text properties, so it's easier to rebuild 
  ;; the whole thing from the split results
  (remem-set-scope-processed scope
   (sort (remem-scope-processed scope)
         (lambda (a b) (string< (elt a field) (elt b field)))))
  (remem-render-processed)
  ;; reselect the selected line if any
  (cond (remem-selection-line
         (goto-char (point-min))
         (cond ((string= "" remem-selection-line-contents)
                (remem-selection-line-overlay-update 0))
               ((re-search-forward 
                 (concat "^" remem-selection-line-contents "$") nil t)
                (remem-selection-line-overlay-update (count-lines 1 (point))))
               (t
                (remem-selection-line-overlay-update 0)))
         ;; center the selected line on the screen
         (recenter))))

(defun remem-process-filter (process string)
  ;; first figure out which scope this is for
  (save-excursion
    (let ((flag t)
	  ;; this is a list of all scopes in all buffers
	  (scopes (remem-all-scopes))
	  (scope nil))
      (while (and flag scopes)
	(cond ((eq process (remem-scope-proc (car scopes)))
	       (setq scope (car scopes))
	       (setq flag nil))
	      (t (setq scopes (cdr scopes)))))
      (cond ((not flag) ; if it belongs to some scope...
	     ;; tack the new string on to the rest of the results
	     (if string (remem-set-scope-raw scope (concat (remem-scope-raw scope)
						string))
	       (message "Savant returned a nil response to query %s" (remem-scope-query scope)))
	     ;; if we've got them all, go to work
	     (cond ((remem-period-on-line-by-itself (remem-scope-raw scope))
		    (remem-set-scope-processed 
		     scope
		     (remem-split-savant-output (remem-scope-raw scope)))
		    (remem-set-scope-raw scope "")
		    ;(remem-sort-scope scope) ; we're still in the right buffer
		    (remem-render-processed) ; when sort works, this is unnecessary
		    (remem-set-scope-in-progress scope nil)
		    (force-mode-line-update))))))))

;;; --------------
;;; SELECTION LINE
;;; --------------

;; these operate on buffer local variables, 
;; so make sure they're in the right buffer

;;;keymap related
(defvar remem-mode-map nil "Local keymap for remem display buffers.")

(if remem-mode-map
    nil
  (let ((map (make-keymap)))
    (suppress-keymap map)
    (cond (running-xemacs
           ;(define-key map [button1up] 'remem-mouse-select)
           (define-key map [button2] 'remem-mouse-popup)
           (define-key map [button3] 'remem-mouse-popup)
           )
          (t
           (define-key map [mouse-1] 'remem-mouse-select)
           (define-key map [down-mouse-2] 'remem-mouse-popup)
           (define-key map [down-mouse-3] 'remem-mouse-popup)))
    (setq remem-mode-map map)))

(defvar remem-output-mode-map nil "Local keymap for remem output buffer (*remem-document-output*).")

(defun remem-kill-output-buffer ()
  (kill-buffer remem-document-buffer-name))

(if remem-output-mode-map
    nil
  (let ((map (make-keymap)))
    (suppress-keymap map)
    (define-key map "1" 'remem-log-rating-1)
    (define-key map "2" 'remem-log-rating-2)
    (define-key map "3" 'remem-log-rating-3)
    (define-key map "4" 'remem-log-rating-4)
    (define-key map "5" 'remem-log-rating-5)
    (define-key map " " 'scroll-up)
    (define-key map "d" 'remem-kill-output-buffer)
    (setq remem-output-mode-map map)))

(defun remem-leave-remem-window ()
  "Leave the remem-display window if you're there.  This is especially for
   XEmacs, 'cause I can't figure out how to keep it from putting the cursor
   in that buffer.  Good sanity check too."
  (interactive)
  (let ((buf (buffer-name (current-buffer))))
    ;(setq remem-debug-bufname buf)
    (cond ((equal buf remem-buffer-name)
	   ;;;;  (print remem-buffer-name (get-buffer "*scratch*"))
           (select-window (frame-first-window))))))

(defun remem-selection-line-overlay-update (&optional n)
  (if n (setq remem-selection-line n))
  (if remem-selection-line ; something selected
    (goto-line remem-selection-line)
    (let ((start (point)))
      (end-of-line)
      (cond ((not remem-selection-line-overlay) ; if no overlay, make one
             (setq remem-selection-line-overlay 
                   (make-overlay start (point)))
             (overlay-put remem-selection-line-overlay 'priority 2)
             (overlay-put remem-selection-line-overlay 'face 'remem-hilite))
            (t ; the overlay exists already
             (move-overlay remem-selection-line-overlay
                           start (point))))
      ;; store the contents of the selection for the sort
      (setq remem-selection-line-contents 
            (buffer-substring start (point))))
    (remem-selection-field-overlay-update)))

; for ease of binding
;(defun remem-overview-selection-next-line (&optional n)
;  (interactive)
;  (let ((lines (or n 1)))
;    (remem-overview-selection-line-overlay-update
;     (+ lines remem-overview-selection-line))))

;(defun remem-overview-selection-prev-line (&optional n)
;  (interactive)
;  (let ((lines (or n 1)))
;    (remem-overview-selection-line-overlay-update
;     (- remem-overview-selection-line lines))))

(defun remem-selection-field-overlay-update (&optional n)
  (if n (setq remem-selection-field n))
  (cond ((and remem-selection-line remem-selection-field (> n 2)) ; queriable field selected
	 (goto-line remem-selection-line) 
	 (let ((start (point)))
	   (end-of-line)
	   (let ((new-start
		  (text-property-any start (point) 
				     'field remem-selection-field)))
	     (if new-start
		 (let ((new-end
			(next-single-property-change new-start 'field)))
		   (cond ((not new-end) nil)		   
			 ((not remem-selection-field-overlay)
			  (setq remem-selection-field-overlay
				(make-overlay new-start new-end))
			  (overlay-put remem-selection-field-overlay 
				       'priority 1)
			  (overlay-put remem-selection-field-overlay
				       'face 'remem-hilite2);)
                          (remem-query-on-field (get-text-property new-start 'field)
                                                (get-text-property new-start 'orig-source) 'mouse))
			 (t ; already exists
			  (move-overlay remem-selection-field-overlay
					new-start new-end);))))))
                          (remem-query-on-field (get-text-property new-start 'field)
                                                (get-text-property new-start 'orig-source) 'mouse))))))))
        ((and remem-selection-line remem-selection-field (= n 0))  ; Selected the lineno
         (remem-retrieve-lineno (get-buffer remem-buffer-name) (+ 1 remem-selection-line)))))

					; for ease of binding
(defun remem-selection-next-field ()
  (interactive)
  (if remem-selection-field-overlay
    (let ((new-start 
           (next-single-property-change 
            (overlay-start remem-selection-field-overlay) 
            'field)))
      (cond (new-start
             (remem-selection-line-overlay-update 
              (count-lines 1 (+ new-start 1)))
             (remem-selection-field-overlay-update
              (get-text-property new-start 'field)))))))

(defun remem-selection-prev-field ()
  (interactive)
  (if remem-selection-field-overlay
      (save-excursion
        (let ((new-end
               (previous-single-property-change 
                (overlay-end remem-selection-field-overlay) 
                'field)))
          (cond (new-end
                 (remem-selection-line-overlay-update 
                  (count-lines 1 new-end))
                 (remem-selection-field-overlay-update
                  (get-text-property (- new-end 1) 'field))))))))

(defun remem-mouse-select (event &optional clickcount)
  (interactive "e")
  (let* ((moused-window (event-window event))
         (moused-buffer (cond (moused-window
                               (window-buffer moused-window))
                              (t nil))))
    (cond (moused-buffer
           (save-excursion
             (set-buffer moused-buffer)
             (remem-selection-line-overlay-update 
              (count-lines 1 (event-point event)))
             (if (get-text-property (event-point event) 'field)
                 (remem-selection-field-overlay-update
                  (get-text-property (event-point event) 'field)))
             (remem-leave-remem-window))))))

(defun remem-mouse-popup (event &optional clickcount)
  (interactive "e")
  (let* ((moused-window (event-window event))
         (moused-buffer (cond (moused-window
                               (window-buffer moused-window))
                              (t nil))))
    (cond (moused-buffer
           (set-buffer moused-buffer)
           (let ((keywords (get-text-property (event-point event) 'keywords))
                 (x-y (event-x-y event))
                 (nice-list nil))
             (cond (running-xemacs
                    (setq nice-list (list "Keywords" keywords)))
                   (t (setq nice-list (list "KEYWORDS" (list "BLAH" (cons keywords nil))))))
             (setcar x-y (+ 20 (car x-y)))
             (setcdr x-y (- (cdr x-y) 50))
             (x-popup-menu event nice-list))))))



;;; --------------
;;; INITIALIZATION
;;; --------------
					; start a scope in the current buffer
(defun remem-start-scope (directory number-lines update-time range)
  (let ((new-scope (vector
                    directory
                    number-lines
                    update-time
                    range
                    nil  ; we'll set up the process shortly
                    nil  ; no history (we aren't using history now anyway)
                    nil  ; no query (don't set to "", 'cause that matches with an empty query & we might not do the first one then)
                    ""   ; no processed
                    nil  ; no in-progress
                    nil  ; we can't set the timer until after we've made the scope
                    ""   ; no raw
                    (current-buffer)
                    0    ; we haven't done any queries yet
                    ""   ; we don't know our version number or database info yet
                    (nth 5 (file-attributes (concat (expand-file-name remem-database-dir) "/"
                                                    directory "/doclocs"))) ; Modtime for the index
		    ))
        (savant (start-process "remem" 
                               nil ;; No buffer for this process
                               (concat (expand-file-name remem-prog-dir)
                                       "/ra-retrieve")
                               (concat (expand-file-name remem-database-dir)
                                       "/" 
                                       directory))))
    (process-kill-without-query savant)
    (set-process-filter savant 'remem-process-filter)
    (remem-set-scope-proc new-scope savant)
;;;    (remem-display-database-info new-scope)    ;; Set version number
    (if (> update-time 0)
      (remem-set-scope-timer new-scope
                             (run-at-time 1 update-time 'remem-around-point new-scope)))
    ; register this scope
    (setq remem-scopes (cons new-scope remem-scopes))
    (if (member (current-buffer) remem-buffers)
      nil
      (setq remem-buffers (cons (current-buffer) remem-buffers)))
  new-scope))

(defun map-start-scopes (scopes-list)
  "maps the add-scope-to-buffer function to the list of scopes"
  (let ((current-scope (car scopes-list)))
    (cond ((null scopes-list) nil)
	  (t (remem-add-scope-to-buffer remem-buffer-name
					(car current-scope)
					(car (cdr current-scope))
					(car (cdr (cdr current-scope)))
					(car (cdr (cdr (cdr current-scope)))))
	     (map-start-scopes (cdr scopes-list))))))


(defun map-scope-percentages (scopes-list)
  (let ((current-scope (car scopes-list)))
    (cond ((null scopes-list) nil)
	  (t (setq remem-scope-percentages
		   (cons
		    (cons (remem-scope-proc current-scope)
			       (float (/ (float (remem-scope-number-lines current-scope))
					 (float remem-total-scope-lines))))
		    remem-scope-percentages))
			 
	     (map-scope-percentages (cdr scopes-list))))))

(defun remem-setup-buffer (buffer-name)
  (let ((buffer (get-buffer-create buffer-name)))
    (save-excursion
      (set-buffer buffer)
      (make-local-variable 'truncate-lines)
      (setq truncate-lines t)
      (setq mode-name "Remembrance Agent")
      (toggle-read-only t)
      (cond ((and running-xemacs (boundp 'scrollbar-width) (boundp 'scrollbar-height)
                  (set-specifier scrollbar-width  0 (get-buffer "*remem-display*"))
                  (set-specifier scrollbar-height 0 (get-buffer "*remem-display*")))))
      (setq remem-selection-line 0)
      (setq remem-selection-line-contents "")
      (setq remem-selection-field 0))))


(defun remem-add-scope-to-buffer (buffer directory number-lines update-time range)
  (cond ((buffer-live-p (get-buffer buffer))
         (save-excursion
           (set-buffer (get-buffer buffer))
           (remem-start-scope directory number-lines update-time range)
           (toggle-read-only -1)
           (setq remem-total-scope-lines (+ number-lines remem-total-scope-lines))
           (setq remem-display-buffer-height (if (< remem-total-scope-lines 4)
                                                 4
                                               (+ 1 remem-total-scope-lines)))
           (insert-string "Reading Database...\n")
           (toggle-read-only t)))))


(defun remem-display-buffer (buffer-name)
  (let ((orig-buffer (current-buffer))
        (orig-window (get-buffer-window (current-buffer))))
    (save-excursion
      (let ((w (if running-xemacs
		   (frame-lowest-window)
		 (window-at 1 (- (frame-height) 3))))
	    (buffer (get-buffer-create buffer-name)))
	(setq w (split-window w))       ;; w is now the lower of the two
	(set-window-buffer w buffer)
	(select-window w)
	(enlarge-window (- remem-display-buffer-height (window-displayed-height) 1))
	(set-window-dedicated-p w t)))
;;  (remem-leave-remem-window)
    (select-window orig-window)))

(defun remem-kill-scope (scope)
  (if (remem-scope-timer scope) (remem-cancel-timer (remem-scope-timer scope)))
  (cond (remem-global-timer 
         (remem-cancel-timer remem-global-timer)
         (setq remem-global-timer nil)))
  (if (eq (process-status (remem-scope-proc scope)) 'run)
      (process-send-string (remem-scope-proc scope) "quit\n")) ; ask it to quit
  (if (eq (process-status (remem-scope-proc scope)) 'run)
      (delete-process (remem-scope-proc scope))) ; if it doesn't... kill it
  ; deregister this scope
  (cond ((buffer-live-p (remem-scope-buffer scope))
         (save-excursion
           (set-buffer (remem-scope-buffer scope))
           (remem-set-scope-in-progress scope nil)
           (setq remem-scopes (delete scope remem-scopes))
           (setq remem-total-scope-lines (- remem-total-scope-lines (remem-scope-number-lines scope)))
           (setq remem-display-buffer-height (if (< remem-total-scope-lines 4)
                                                 4
                                               remem-total-scope-lines))
           (if (null remem-scopes)
               (setq remem-buffers (delete (remem-scope-buffer scope) remem-buffers)))))))

(defun remem-map-kill (some-list)
  (cond ((null some-list) nil)
	(t (remem-kill-scope (car some-list))
	   (remem-map-kill (cdr some-list))))) 


(defun remem-kill-all-scopes ()
  "kills all the scopes by mapping remem-kill-scope on the list of scopes" 
  (cond ((buffer-live-p (get-buffer remem-buffer-name))
         (save-excursion
           (set-buffer (get-buffer remem-buffer-name))
           (remem-log-checkpoint remem-scopes 'remem-killed)
           (remem-map-kill remem-scopes)))))

(defun remem-kill-buffer (buffer-name)
  (let ((buffer (get-buffer buffer-name)))
    (cond (buffer
           (save-excursion
             (kill-buffer buffer))))))

(defun remem-change-database (scopename scopenum)
  "Change the remembrance agent database to one of the preset indexes"
  (interactive
   (list (let* ((dirlist (directory-files remem-database-dir nil "[^\.].*"))
                (scope-name-list (mapcar '(lambda (filename)
                                            (cons filename filename))
                                         dirlist))
                (dirprompt (mapconcat 'eval dirlist " "))
                (val))
           (setq val (completing-read 
                      (concat "Change to what database (" dirprompt "): ")
                      scope-name-list
                      'consp t))
           (cdr (assoc val scope-name-list)))
         (let ((rsl-len (length remem-scopes-list))
               (val))
           (cond ((> rsl-len 1)
                  (setq val (string-to-number (read-string (concat "Enter scope number (1-" rsl-len "): ")))))
                 (t (setq val 1)))
           val)))
  (setcar (car (nthcdr (- scopenum 1) remem-scopes-list)) scopename)
  (cond ((not (assoc (buffer-name (current-buffer)) remem-buffname-db-alist))
         (let ((new-scopes-list (copy-alist remem-scopes-list)))
           (setq remem-buffname-db-alist (append remem-buffname-db-alist
                                                 (list (cons (buffer-name (current-buffer)) new-scopes-list)))))))
  (remem))


;;; ------------------------------------
;;; SET VERSION STRING (FOR LOGGING)
;;; ------------------------------------

(defun remem-database-info-filter (proc string)
  (remem-set-scope-dbinfo remem-working-scope 
                          (concat (remem-scope-dbinfo remem-working-scope)
                                  string)))

(defun remem-display-database-info (scope)
  "Display version info for a given scope"
  (let ((proc (remem-scope-proc scope))
        (linestart))
    (setq remem-versionstring-temp "")
    (cond ((remem-scope-in-progress scope)
           (message "Waiting for previous query to finish... (hit ^G to abort)")
           (while (remem-scope-in-progress scope)
             (sleep-for .2))))
                                        ;	   (setq remem-document-buffer-name
                                        ;		 (concat "*remem-document-output: " 
                                        ;			 (int-to-string lineno) "*"))

    (remem-set-scope-dbinfo scope "")
    (setq remem-working-scope scope)     ;; As a message passed to remem-database-info-filter
    (save-excursion
      (set-process-filter proc 'remem-database-info-filter)
      (process-send-string proc "info\n")

      ;; Hang out for a second to let it work it's mojo
      (while (< (length (remem-scope-dbinfo scope)) 22)
        (sleep-for .05))

      (if (string-match "info\n" (remem-scope-dbinfo scope))
          (remem-set-scope-dbinfo scope (replace-match "" t t (remem-scope-dbinfo scope))))
      (while (string-match "\n" (remem-scope-dbinfo scope))
        (remem-set-scope-dbinfo scope (replace-match "" t t (remem-scope-dbinfo scope)))))))

(defun remem-set-database-info-for-scopes (all-scopes)
  (cond (all-scopes
         (remem-display-database-info (car all-scopes))
         (remem-set-database-info-for-scopes (cdr all-scopes)))))


;;; ---------
;;; RETRIEVAL
;;; ---------


(defun remem-display-filter (proc string)
  (save-excursion
    (set-buffer (get-buffer-create remem-document-buffer-name))
    (goto-char (marker-position-nonil (process-mark proc)))
    (toggle-read-only -1)
    (insert string)
    (set-marker (process-mark proc) (point))
    (toggle-read-only t)))

;(defun remem-display-filter (proc string)
;  (save-excursion
;    (set-buffer remem-document-buffer-name)
;    (goto-char (point-max))
;    (insert string)
;    (goto-char (point-max))))

;; called by remem-retrieve
(defun remem-display-line-copy (scope docnum &optional lineno)
  "Display the output for the relevant document displayed in the given line"
  (let ((proc (remem-scope-proc scope)))
    (cond ((remem-scope-in-progress scope)
           (message "Waiting for previous query to finish... (hit ^G to abort)")
           (while (remem-scope-in-progress scope)
             (sleep-for .2))))
    (message "Retrieving docnum %d" docnum)
    (cond ((or (not docnum) (< docnum 1))
	   (message "Improper docnum."))
	  ((not scope)
	   (message "Improper scope."))
	  (t
;	   (setq remem-document-buffer-name
;		 (concat "*remem-document-output: " 
;			 (int-to-string lineno) "*"))
	   (select-window (frame-first-window))
;          (cond ((get-buffer remem-document-buffer-name)  ;; Just to clear it out
;                (kill-buffer remem-document-buffer-name)))
	   (switch-to-buffer remem-document-buffer-name)
           (toggle-read-only -1)
	   (erase-buffer)
           (toggle-read-only t)

	   (set-marker (process-mark proc) (point) 
                       (get-buffer remem-document-buffer-name))
	   (set-process-filter proc 'remem-display-filter)
	   (process-send-string proc 
				(concat "retrieve "
					(int-to-string docnum)
					"\n"))
	   ;; loop until ready
	   (goto-char (point-min))
	   (while (< (marker-position-nonil (process-mark proc)) 22)
	     (sleep-for .05))

	   (end-of-line)
	   (setq doc-pos (string-to-int (buffer-substring (point-min) (point))))
	   (forward-char)
           (toggle-read-only -1)
	   (delete-region (point-min) (point))  ;; Delete the character offset

           (end-of-line)
	   (setq remem-last-followed-doctype (buffer-substring (point-min) (point)))
	   (forward-char)
	   (delete-region (point-min) (point))  ;; Delete the character offset

	   (while (< (marker-position-nonil (process-mark proc)) doc-pos)
	     (sleep-for .001))
	   (goto-char doc-pos)
	   (beginning-of-line)
	   (recenter 0)
           (setq remem-last-followed-docnum docnum)
           (message "Type number 1-5 to rate document: 1 = [Bad suggestion], 5 = [Great suggestion]")
           (use-local-map remem-output-mode-map)
           (while (let ((old-pos (marker-position-nonil (process-mark proc))))  ;; Wait till it stops moving
                    (sleep-for .05)
                    (< old-pos (marker-position-nonil (process-mark proc)))))
           (run-hooks 'remem-gotdoc-hook)
           (toggle-read-only t)))))


;; called by remem-retrieve
(defun remem-display-line-original (scope docnum &optional lineno)
  "Load the file for the relevant document displayed in the given line"
    (let ((proc (remem-scope-proc scope)))
      (cond ((remem-scope-in-progress scope)
             (message "Waiting for previous query to finish... (hit ^G to abort)")
             (while (remem-scope-in-progress scope)
               (sleep-for .2))))
      (message "Retrieving docnum %d" docnum)
      (cond ((or (not docnum) (< docnum 1))
	     (message "Improper docnum."))
	    ((not scope)
	     (message "Improper scope."))
	    (t
;	     (setq remem-document-buffer-name
;		   (concat "*remem-document-output: " 
;			   (int-to-string lineno) "*"))
	     (select-window (frame-first-window))
;            (cond ((get-buffer remem-document-buffer-name)  ;; Just to clear it out
;                   (kill-buffer remem-document-buffer-name)))
	     (switch-to-buffer remem-document-buffer-name)
             (toggle-read-only -1)
	     (erase-buffer)
             (toggle-read-only t)

             (set-marker (process-mark proc) (point) 
                         (get-buffer remem-document-buffer-name))
	     (set-process-filter proc 'remem-display-filter)
	     (process-send-string proc 
				  (concat "loc-retrieve "
					  (int-to-string docnum)
					  "\n"))
	     (goto-char (point-min))
	     (while (< (marker-position-nonil (process-mark proc)) 22)
	       (sleep-for .05))
					; loop until ready
	     (end-of-line)
	     (setq doc-pos-start (string-to-int (buffer-substring (point-min) (point))))
	     (forward-char)
             (toggle-read-only -1)
	     (delete-region (point-min) (point))       ; Get rid of first line -- the doc start

	     (end-of-line)
	     (setq doc-pos-end (string-to-int (buffer-substring (point-min) (point))))
	     (forward-char)
	     (delete-region (point-min) (point))       ; Get rid of second line -- the doc end

	     (end-of-line)
	     (setq doc-loc (buffer-substring (point-min) (point)))
	     (forward-char)
	     (delete-region (point-min) (point))       ; Get rid of third line -- the doc loc

	     (end-of-line)
	     (setq remem-last-followed-doctype (buffer-substring (point-min) (point)))

	     (print (concat "doc-pos-start: " doc-pos-start) (get-buffer "*remem-log*"))
	     (print (concat "doc-pos-end: " doc-pos-end) (get-buffer "*remem-log*"))
	     (print (concat "doc-loc: " doc-loc) (get-buffer "*remem-log*"))

	     (bury-buffer remem-document-buffer-name)
	     (find-file doc-loc)
	     (if (string= major-mode "rmail-mode")
					; in rmail mode, use the variable pointing to start of message
		 (remem-rmail-goto-char (+ 2 doc-pos-start))     ; add two to skip the ^_^L delimiter
	       (goto-char doc-pos-start))
	     (beginning-of-line)
	     (recenter 0)
              (setq remem-last-followed-docnum docnum)
             (message (concat "Rate document: " remem-command-prefix "r <rating>, where <rating> is from 1 [Bad suggestion] to 5 [Great suggestion]"))
	     (run-hooks 'remem-gotdoc-hook)
             (toggle-read-only t)))))

;;; retrieve an original or a copy
;;; with no arguments, tries to retrieve what's at point
(defun remem-retrieve (&optional scop docn)
  (interactive)
  (save-excursion
    (let* ((scope (or scop ; pre-specified
		      (get-text-property (point) 'scope) ; we're in one
		      (and remem-buffers ; go to the first one in the list (default)
                           (buffer-live-p (car remem-buffers))
			   (set-buffer (car remem-buffers))
			   (get-text-property (point) 'scope))))
	   (docnum (or docn
		       (get-text-property (point) 'docnum))) ; note, we may be in the default buffer
           (relevance (get-text-property (point) 'relevance))
	   (lineno (count-lines 1 (point))))
      (cond (scope 
             (if remem-load-original-suggestion
                 (remem-display-line-original scope docnum lineno)
               (remem-display-line-copy scope docnum lineno))))))
  (remem-leave-remem-window))

  
;;; retrieve from a given line
(defun remem-retrieve-lineno (buffer lineno)
  (cond ((buffer-live-p buffer)
         (save-excursion
           (set-buffer buffer)
           (goto-line lineno)
           (let ((scope (get-text-property (point) 'scope))
                 (docnum (get-text-property (point) 'docnum))
                 (sims-breakdown (get-text-property (point) 'sims-breakdown))
                 (relevance (get-text-property (point) 'relevance)))
             (cond ((and scope docnum relevance lineno)
                    (remem-log-checkpoint remem-scopes 'retrieval)
                    (remem-log-suggestion-followed scope docnum relevance lineno sims-breakdown)
                    (remem-retrieve scope docnum))))))))

(defun remem-display-line-1 (&optional args)
  (interactive "P")
  (remem-retrieve-lineno (get-buffer remem-buffer-name) 1))

(defun remem-display-line-2 (&optional args)
  (interactive "P")
  (remem-retrieve-lineno (get-buffer remem-buffer-name) 2))

(defun remem-display-line-3 (&optional args)
  (interactive "P")
  (remem-retrieve-lineno (get-buffer remem-buffer-name) 3))

(defun remem-display-line-4 (&optional args)
  (interactive "P")
  (remem-retrieve-lineno (get-buffer remem-buffer-name) 4))

(defun remem-display-line-5 (&optional args)
  (interactive "P")
  (remem-retrieve-lineno (get-buffer remem-buffer-name) 5))

(defun remem-display-line-6 (&optional args)
  (interactive "P")
  (remem-retrieve-lineno (get-buffer remem-buffer-name) 6))

(defun remem-display-line-7 (&optional args)
  (interactive "P")
  (remem-retrieve-lineno (get-buffer remem-buffer-name) 7))

(defun remem-display-line-8 (&optional args)
  (interactive "P")
  (remem-retrieve-lineno (get-buffer remem-buffer-name) 8))

(defun remem-display-line-9 (&optional args)
  (interactive "P")
  (remem-retrieve-lineno (get-buffer remem-buffer-name) 9))

(defun remem-display-other (lineno)
  (interactive "nLine Number:")
  (remem-retrieve-lineno (get-buffer remem-buffer-name) lineno))



(defun remem-log-rating-1 (&optional args)
  (interactive "P")
  (remem-log-rating 1))

(defun remem-log-rating-2 (&optional args)
  (interactive "P")
  (remem-log-rating 2))

(defun remem-log-rating-3 (&optional args)
  (interactive "P")
  (remem-log-rating 3))

(defun remem-log-rating-4 (&optional args)
  (interactive "P")
  (remem-log-rating 4))

(defun remem-log-rating-5 (&optional args)
  (interactive "P")
  (remem-log-rating 5))



;;;;Starting up and killing the front end
;;;;basic initialization stuff

(defun remem-fix-window-loss ()
  "Makes sure that Remem dies when the buffer is gone, and respawns when window is no longer visible.  This is because of that stupid resize-windows bug"
  (unless (window-live-p (get-buffer-window remem-buffer-name t))
    (cond ((get-buffer remem-buffer-name)    ;; if the buffer exists, respawn the window
           (remem-display-buffer remem-buffer-name))
          (t
           (remem-cancel-timer remem-global-timer)
           (kill-remem)))))

(defun remem-restart-on-outdated-index (scope)
  "If any of the index files we're looking at are newer than they were when we started up their processes, restart ra-retrieve."
  (let ((new-modtime (nth 5 (file-attributes (concat (expand-file-name remem-database-dir) "/"
                                                     (remem-scope-directory scope)
                                                     "/doclocs"))))
        (old-modtime (remem-scope-dbmodtime scope)))
    (setq remem-debug-old-modtime old-modtime)
    (setq remem-debug-new-modtime new-modtime)
    (cond ((not (equal new-modtime old-modtime))
           (remem)
           (message "Remem index files have been updated... restarting.")
           t)
          (t nil))))

(defun start-remem ()
  "starts the processes.  essentially a bundling function"
  (save-excursion
    (cond (remem-display-running
	   (message "Remembrance Agent already running"))
	  (t
           (cond ((not (file-exists-p (concat (expand-file-name remem-prog-dir)
                                                  "/ra-retrieve")))
                  (message (concat "Cannot find program file: "
                                   (expand-file-name remem-prog-dir)
                                   "/ra-retrieve")))
                 ((not (file-exists-p (concat (expand-file-name remem-database-dir) "/")))
                  (message (concat "Cannot find index directory: "
                                   (expand-file-name remem-database-dir))))
                 ((let ((notfound nil))
                    (mapcar 
                     '(lambda (scopeinfo)
                        (cond ((not (file-exists-p (concat (expand-file-name remem-database-dir) "/"
                                                           (car scopeinfo))))
                               (message (concat "Cannot find index-file subdirectory: "
                                                (expand-file-name remem-database-dir) "/"
                                                (car scopeinfo)))
                               (setq notfound t))))
                     remem-scopes-list)
                    notfound))
                 (t
                  (setq remem-display-running t)
                  (remem-setup-buffer remem-buffer-name)
                  (map-start-scopes (reverse remem-scopes-list))
                  (save-excursion
                    (set-buffer (get-buffer-create remem-buffer-name))
                    (map-scope-percentages remem-scopes)
                    
                    ;; This probably won't log anyway, since numqueries should == 0
                    (remem-log-checkpoint remem-scopes 'remem-started)   
                    
                    (remem-set-database-info-for-scopes remem-scopes)
                    (remem-log-dbinfo remem-scopes)
                    
                    (cond (running-xemacs
                           (make-local-hook 'mouse-track-click-hook)
                           (setq mouse-track-click-hook 'remem-mouse-select)))
                    (use-local-map remem-mode-map))
                  (remem-display-buffer remem-buffer-name)
                  (setq remem-hide-display nil)
                  (setq remem-global-timer
                        (run-at-time 5 3 'remem-fix-window-loss))
                  
                  ;;to make the display stick!

;;; I don't like resetting C-xo, so I won't
;;;                  (setq remem-old-C-xo (global-key-binding "\C-xo"))
;;;                  (global-set-key "\C-xo" 'remem-other-window)

;;; But I'll make an exception for C-x1 'cause it's so useful
                  (setq remem-old-C-x1 (global-key-binding "\C-x1"))
                  (global-set-key "\C-x1" 'remem-delete-other-windows)

                  (global-set-key (concat remem-command-prefix "v") 'remem-query-now)
                  (global-set-key (concat remem-command-prefix "n") 'remem-display-other)
		  (global-set-key (concat remem-command-prefix "f") 'remem-grab-query)
		  (global-set-key (concat remem-command-prefix "d") 'remem-change-database)
                  (global-set-key (concat remem-command-prefix "q") 'remem-create-query-page)

	   ;;;set the key bindings for the retrieval
                  (global-set-key (concat remem-command-prefix "1") 'remem-display-line-1)
                  (global-set-key (concat remem-command-prefix "2") 'remem-display-line-2)
                  (global-set-key (concat remem-command-prefix "3") 'remem-display-line-3)
                  (global-set-key (concat remem-command-prefix "4") 'remem-display-line-4)
                  (global-set-key (concat remem-command-prefix "5") 'remem-display-line-5)
                  (global-set-key (concat remem-command-prefix "6") 'remem-display-line-6)
                  (global-set-key (concat remem-command-prefix "7") 'remem-display-line-7)
                  (global-set-key (concat remem-command-prefix "8") 'remem-display-line-8)
                  (global-set-key (concat remem-command-prefix "9") 'remem-display-line-9)

                  (cond (remem-non-r-number-keys
                         (global-set-key "\C-c1" 'remem-display-line-1)
                         (global-set-key "\C-c2" 'remem-display-line-2)
                         (global-set-key "\C-c3" 'remem-display-line-3)
                         (global-set-key "\C-c4" 'remem-display-line-4)
                         (global-set-key "\C-c5" 'remem-display-line-5)
                         (global-set-key "\C-c6" 'remem-display-line-6)
                         (global-set-key "\C-c7" 'remem-display-line-7)
                         (global-set-key "\C-c8" 'remem-display-line-8)
                         (global-set-key "\C-c9" 'remem-display-line-9)))
                  
                  (global-unset-key (concat remem-command-prefix "r")) ; Just to be safe
                  (global-set-key (concat remem-command-prefix "r1") 'remem-log-rating-1)
                  (global-set-key (concat remem-command-prefix "r2") 'remem-log-rating-2)
                  (global-set-key (concat remem-command-prefix "r3") 'remem-log-rating-3)
                  (global-set-key (concat remem-command-prefix "r4") 'remem-log-rating-4)
                  (global-set-key (concat remem-command-prefix "r5") 'remem-log-rating-5)
                  (run-hooks 'remem-start-hook)
                  (message  "Remembrance Agent started")))))))


(defun kill-remem ()
  "kills all the processes, closes the remem-buffer"
  (save-excursion
    (cond (remem-display-running
	   (setq remem-display-running nil)
	   (remem-kill-all-scopes)
           (setq remem-display-buffer-height 0)
           (setq remem-total-scope-lines 0)
	   (if (or remem-hide-display
		   (eq nil (get-buffer-window remem-buffer-name t)))
	       nil
	     (remem-delete-window remem-buffer-name))
	   (remem-kill-buffer remem-buffer-name)	   ;;to make the display stick!
	   (global-set-key "\C-x1" remem-old-C-x1)
;;;	   (global-set-key "\C-xo" remem-old-C-xo)

	   ;;;unset the key bindings
	   
           (global-unset-key (concat remem-command-prefix "v"))
           (global-unset-key (concat remem-command-prefix "n"))
           (global-unset-key (concat remem-command-prefix "f"))
           (global-unset-key (concat remem-command-prefix "d"))
           (global-unset-key (concat remem-command-prefix "q"))
           (global-unset-key (concat remem-command-prefix "f"))
	   (global-unset-key (concat remem-command-prefix "v"))
           (global-unset-key (concat remem-command-prefix "1"))
           (global-unset-key (concat remem-command-prefix "2"))
           (global-unset-key (concat remem-command-prefix "3"))
           (global-unset-key (concat remem-command-prefix "4"))
           (global-unset-key (concat remem-command-prefix "5"))
           (global-unset-key (concat remem-command-prefix "6"))
           (global-unset-key (concat remem-command-prefix "7"))
           (global-unset-key (concat remem-command-prefix "8"))
           (global-unset-key (concat remem-command-prefix "9"))
           (cond (remem-non-r-number-keys
                  (global-unset-key "\C-c1")
                  (global-unset-key "\C-c2")
                  (global-unset-key "\C-c3")
                  (global-unset-key "\C-c4")
                  (global-unset-key "\C-c5")
                  (global-unset-key "\C-c6")
                  (global-unset-key "\C-c7")
                  (global-unset-key "\C-c8")
                  (global-unset-key "\C-c9")))

           (run-hooks 'remem-start-hook)
	   (message "Remembrance Agent stopped")))))

(defun remem-hide-display ()
  (save-excursion
    (cond (remem-display-running
	   (cond (remem-hide-display
		  (message "Remembrance display already hidden"))
		 (t (setq remem-hide-display t)
		    (select-window (get-buffer-window (get-buffer remem-buffer-name)))
		    (delete-window))))
	  (t (message "Remembrance Agent not running")))))


(defun remem-show-display ()
  (save-excursion
    (cond (remem-display-running
	   (cond (remem-hide-display
		  (remem-display-buffer remem-buffer-name)
		  (setq remem-hide-display nil))
		 (t (message "Remembrance display not hidden"))))
	  (t (message "Remembrance Agent not running")))))
  
(defun remem-toggle ()
  (interactive)
  (let ((w (selected-window)))
    (save-excursion
      (cond (remem-display-running
             (kill-remem))
            ((not remem-display-running)
             (start-remem))))
    (select-window w)))

(defun remem ()
  "Start the remembrance agent.  If already running, kill it and restart it."
  (interactive)
  (let ((w (selected-window)))
    (save-excursion
      (cond (remem-display-running
             (kill-remem)
             (sleep-for 1)
             (start-remem))
            ((not remem-display-running)
             (start-remem))))
    (select-window w)))


  
;;; -------------------
;;; ALTERNATE FUNCTIONS
;;; -------------------

(defun remem-other-window (&rest args)
  "Replacement for the C-xo key binding that takes the cursor 
   remem-display window"
  (interactive)
  (let ((sw (selected-window))
	(rw (get-buffer-window remem-buffer-name t))
	(nw (next-window (selected-window) 1))
	w)
    (cond ((eq nw rw)
	   (other-window 2))
	  (t
	   (other-window 1)))
    ))

(defun remem-delete-window (buffer-name)
  (let ((buffer (get-buffer buffer-name)))
    (save-excursion
     (select-window (get-buffer-window buffer t))
     (delete-window))))

(defun remem-delete-other-windows (&rest args)
  "Replacement for delete-other-windows that won't delete the 
*remem-buffer* window."
  (interactive)
  (let ((sw (selected-window))
	(rw (get-buffer-window remem-buffer-name))
	(nw (next-window (selected-window) 1))
	w)
    (cond (rw
           (if (eq sw rw) 
               (progn (other-window 1)
                      (setq sw (selected-window))
                      (setq nw (next-window (selected-window) 1))))
           (while (not (eq nw sw))
             (setq w nw)
             (setq nw (next-window nw 1))
             (or (eq w rw)
                 (eq w sw)
                 (delete-window w)))
           (message (concat "Use " remem-command-prefix "t to quit the Remembrance Agent")))
          (t (delete-other-windows)))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Specialty functions for handling wierd modes
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defun remem-rmail-what-message-at-point (n)
  (let ((where n)
	(low 1)
	(high rmail-total-messages)
	(mid (/ rmail-total-messages 2)))
    (while (> (- high low) 1)
      (if (>= where (rmail-msgbeg mid))
	  (setq low mid)
	(setq high mid))
      (setq mid (+ low (/ (- high low) 2))))
    (if (>= where (rmail-msgbeg high)) high low)))

(defun remem-rmail-goto-char (n)
  "Jump to a given global char number in an rmail message"
  (interactive "nChar: ")
  (rmail-show-message (remem-rmail-what-message-at-point n))
  (goto-char n))


;;;;;;;;;;;;;;;;;;;
;; Logging
;;
;; Events that are logged (for each scope):
;;   Following a suggestion
;;     Docnum, relevance, scopename, lineno within scope, numlines shown, 
;;     whether was automatic / mouse-query / remem-query
;;   kill-remem, start-remem, following a suggestion, resize, every ~500 queries since last log
;;     Num queries done, update period for scopes, scopenames, numlines shown in scope, which event triggered
;;   Rating a suggestion
;;     Docnum, rating, rated document type, major-mode for this query
;;   Making a mouse-query
;;     Fieldno
;;   Making a remem-query
;;     just the event
;;;;;;;;;;;;;;;;;;;

(defun remem-log-string (string)
  "Append string to the logfile"
  (cond (remem-log-p
         (write-region (concat (current-time-string) "  --  " string) nil remem-logfile t 1))))

(defun remem-log-suggestion-followed (scope docnum relevance-and-line lineno sims-breakdown)
  (let ((numlines (remem-scope-number-lines scope))
        (scopename (remem-scope-directory scope))
        (relevance (substring relevance-and-line 4 8))
        (scopeline (string-to-number (substring relevance-and-line 0 3))))
    (remem-log-string (format "Followed: %d, %s, %s, %d, %d, %s, %s\n" 
                              docnum relevance scopename scopeline numlines 
                              remem-kind-of-query sims-breakdown))))
        
(defun remem-log-checkpoint-scope (scope why-check)
  (let ((num-queries (remem-scope-querycounter scope))
        (update-period (remem-scope-update-time scope))
        (scopename (remem-scope-directory scope))
        (numlines (remem-scope-number-lines scope)))

;;; No cond, so it logs events even when no checkpoint is needed.
;   (cond ((> num-queries 0)
    (remem-log-string (format "Checkpoint: %d, %s, %d, %d, %s\n" 
                              num-queries scopename update-period numlines why-check))
    (remem-set-scope-querycounter scope 0)))
;))

(defun remem-log-checkpoint (scopes-left why-check)
  (cond (scopes-left
         (remem-log-checkpoint-scope (car scopes-left) why-check)
         (remem-log-checkpoint (cdr scopes-left) why-check))))

(defun remem-log-dbinfo-scope (scope)
  (let ((scopename (remem-scope-directory scope))
        (dbinfo (remem-scope-dbinfo scope)))
    (remem-log-string (format "%s, %s\n" dbinfo scopename))))

(defun remem-log-dbinfo (scopes-left)
  (cond (scopes-left
         (remem-log-dbinfo-scope (car scopes-left))
         (remem-log-dbinfo (cdr scopes-left)))))

(defun remem-log-rating (rating)
  (cond (remem-last-followed-docnum
         (remem-log-string (format "Rating: %d, %d, %s, %s\n" remem-last-followed-docnum rating
                                   remem-last-followed-doctype remem-last-query-mode))
         (cond ((eq rating 1)
                (message (format "Document rated: 1 [Bad suggestion]")))
               ((eq rating 2)
                (message (format "Document rated: 2 [So-so suggestion]")))
               ((eq rating 3)
                (message (format "Document rated: 3 [OK suggestion]")))
               ((eq rating 4)
                (message (format "Document rated: 4 [Good suggestion]")))
               ((eq rating 5)
                (message (format "Document rated: 5 [Great suggestion]")))))
        (t
         (message (format "No document to rate or document already rated" rating))))
  (setq remem-last-followed-docnum nil)
  (setq remem-last-followed-doctype nil)
  (setq remem-last-query-mode nil))


;(defun remem-mail-hook ()
;  (save-excursion
;    (goto-char (point-min))
;    (setq remem-mail-msg-start (search-forward "\n\n"))))

;(add-hook 'rmail-show-message-hook 'remem-mail-hook)

'remem-loaded
