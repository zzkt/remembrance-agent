;jimminy.el -- wearable computer extensions for remem.el
;
;All code included in versions up to and including 2.09:
;   Copyright (C) 2001 Massachusetts Institute of Technology.
;
;All modifications subsequent to version 2.09 are copyright Bradley
;Rhodes or their respective authors.
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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Jimminy is the wearable computer interface to Remem.
;; It allows you to set emacs variables describing your current context:
;; location, people in the room, subject of conversation and a timestamp.
;; These can be used to annotate notes currently being taken on the wearable, 
;; and are also used as an automatic query to the RA.

;; In the full working system these emacs variables are set using Hive
;; http://www.hivecell.net

;; This should probably separate the note-taking header creation from the remem part, 
;; but it works OK. It's also a good example of how one can use the hooks in
;; remem to operate it by other than the normal means.

(provide 'jimminy)
(require 'remem)
(require 'time-stamp)


(defvar jimminy-temporary-bias-offset 3
  "Amount to temporarilly add to the hand-set bias when the field changes")
(defvar jimminy-temporary-bias-lifetime 60
  "Number of seconds to temporarilly add to a hand-set biase when the field changes")

;; Use this in remem-format-alist if you want -- it's good for smaller screens
(setq remem-format-jimminy
  '((0 2 (field 0 mouse-face remem-hilite2) nil)                   ; Number
    (1 2 (face remem-even field 1) nil)                            ; sim
    (9 15 (face remem-odd field 9 mouse-face remem-hilite) nil)    ; person
    (8 25 (face remem-even field 8 mouse-face remem-hilite) nil)   ; subject
    (5 8 (face remem-odd field 5 mouse-face remem-hilite) remem-date-print-filter)    ; date
    (28 50 (face remem-even field 28 mouse-face remem-hilite) nil))); keywords

;; For time stamps in the Jimminy header
(setq time-stamp-format "%3a, %:d %3b %:y %02H:%02M:%02S %Z")

(global-set-key "\C-ch" 'jimminy-update-header-info)
(global-set-key "\C-ci" 'jimminy-insert-header-info)
(global-set-key "\C-cl" 'jimminy-set-location-string)
(global-set-key "\C-cs" 'jimminy-set-subject-string)
(global-set-key "\C-cp" 'jimminy-set-person-string)
(global-set-key "\C-cbl" 'jimminy-set-location-bias)
(global-set-key "\C-cbs" 'jimminy-set-subject-bias)
(global-set-key "\C-cbp" 'jimminy-set-person-bias)
(global-set-key "\C-cbb" 'jimminy-set-body-bias)

(global-set-key "\C-cjil" 'jimminy-increment-location-bias)
(global-set-key "\C-cjip" 'jimminy-increment-person-bias)
(global-set-key "\C-cjis" 'jimminy-increment-subject-bias)
(global-set-key "\C-cjib" 'jimminy-increment-body-bias)

(global-set-key "\C-cjdl" 'jimminy-decrement-location-bias)
(global-set-key "\C-cjdp" 'jimminy-decrement-person-bias)
(global-set-key "\C-cjds" 'jimminy-decrement-subject-bias)
(global-set-key "\C-cjdb" 'jimminy-decrement-body-bias)
       
(global-set-key "\C-cjcl" 'jimminy-clear-location-string)
(global-set-key "\C-cjcp" 'jimminy-clear-person-string)
(global-set-key "\C-cjcs" 'jimminy-clear-subject-string)
(global-set-key "\C-cjca" 'jimminy-clear-all-string)

(defvar jimminy-location-string "")
(defvar jimminy-person-string "")
(defvar jimminy-subject-string "")

;; Biases actually used by Savant
(defvar jimminy-location-bias 1)
(defvar jimminy-subject-bias 1)
(defvar jimminy-person-bias 1)
(defvar jimminy-body-bias 1)

;; Biases that are actually what the user set
;; These might differ from the biases used because
;; we temporarily add to features that have just changed
(defvar jimminy-location-bias-setting 1)
(defvar jimminy-subject-bias-setting 1)
(defvar jimminy-person-bias-setting 1)
(defvar jimminy-body-bias-setting 1)

(defvar jimminy-location-timer nil
  "Timer for resetting location timer -- used to up priority when feature changes")
(defvar jimminy-person-timer nil
  "Timer for resetting person timer -- used to up priority when feature changes")
(defvar jimminy-subject-timer nil
  "Timer for resetting subject timer -- used to up priority when feature changes")
(defvar jimminy-body-timer nil
  "Timer for resetting body timer -- used to up priority when feature changes")

;; Don't use the major-mode in the string, we want to use JIMMINY-QUERY-MODE
(setq remem-use-major-mode-templates nil)

;; This sets the mode line for the *remem-display* mode,
;; So we don't need much info (we know the buffer name, and 
;; time is displayed in other buffer mode lines)
(defun jimminy-mode-line-update ()
  (cond ((and (boundp 'remem-buffer-name)
              (get-buffer remem-buffer-name))
         (save-excursion 
           (set-buffer (get-buffer remem-buffer-name))
           (setq mode-line-format 
                 (list 
                                        ;        "-" 
                                        ;        mode-line-modified " " 
                                        ;        mode-line-buffer-identification "["
                                        ;        global-mode-string
                                        ;        "] "
                  "-"
                  (cond ((not (eq jimminy-location-bias-setting jimminy-location-bias)) " ."))
                  (number-to-string jimminy-location-bias)
                  (cond ((not (eq jimminy-location-bias-setting jimminy-location-bias)) ". "))

                  (cond ((not (eq jimminy-subject-bias-setting jimminy-subject-bias)) " ."))
                  (number-to-string jimminy-subject-bias)
                  (cond ((not (eq jimminy-subject-bias-setting jimminy-subject-bias)) ". "))

                  (cond ((not (eq jimminy-person-bias-setting jimminy-person-bias)) " ."))
                  (number-to-string jimminy-person-bias)
                  (cond ((not (eq jimminy-person-bias-setting jimminy-person-bias)) ". "))

                  (number-to-string jimminy-body-bias)
                  "- "
                  jimminy-location-string 
                  (if (equal jimminy-location-string "") "" " ")
                  "|"
                  jimminy-subject-string 
                  (if (equal jimminy-subject-string "") "" " ")
                  "|"
                  jimminy-person-string
                  ))))))

(defun jimminy-update-changes ()
  (jimminy-mode-line-update)
  (setq remem-query-extra-text-string
        (concat "JIMMINY REMEMBRANCE QUERY MODE\n"
                "Jimminy-query-header <"
                jimminy-location-string "|"
                jimminy-person-string "|"
                jimminy-subject-string "|"
                (time-stamp-string)
                ">\n")))

;; remem-query-oneshot-preamble-string is the string through which commands 
;; are inserted into Savant.
;; Anything in this string is attached to the query command, then blanked
;; out so it only gets executed once.
;; With Jimminy we want to use our own biases rather than the template
;; defaults, so set that mode here.
(defun jimminy-set-all-biases ()
  (setq remem-query-oneshot-preamble-string 
        (concat 
         "use-handset-biases\n"
         "set-bias BODY " (number-to-string jimminy-body-bias) "\n"
         "set-bias LOCATION " (number-to-string jimminy-location-bias) "\n"
         "set-bias PERSON " (number-to-string jimminy-person-bias) "\n"
         "set-bias SUBJECT " (number-to-string jimminy-subject-bias) "\n"))
  (jimminy-update-changes))

(add-hook 'remem-start-hook 'jimminy-set-all-biases)

(jimminy-set-all-biases)  ;; In case the RA is already running

(defun jimminy-set-location-string (string)
  (interactive "sLocation: ") 
  (jimminy-set-location-bias (+ jimminy-temporary-bias-offset jimminy-location-bias-setting) t)
  (cond (jimminy-location-timer (remem-cancel-timer jimminy-location-timer)))
  (setq jimminy-location-timer 
        (run-at-time jimminy-temporary-bias-lifetime nil 'jimminy-reset-location-bias))
  (setq jimminy-location-string 
	(downcase string)) (jimminy-update-changes))
(defun jimminy-set-person-string (string)
  (interactive "sPerson: ")
  (setq jimminy-person-string (downcase string))
  (jimminy-set-person-bias (+ jimminy-temporary-bias-offset jimminy-person-bias-setting) t)
  (cond (jimminy-person-timer (remem-cancel-timer jimminy-person-timer)))
  (setq jimminy-person-timer 
        (run-at-time jimminy-temporary-bias-lifetime nil 'jimminy-reset-person-bias))
  (jimminy-update-changes))
(defun jimminy-set-subject-string (string)
  (interactive "sSubject: ")
  (setq jimminy-subject-string (downcase string))
  (jimminy-set-subject-bias (+ jimminy-temporary-bias-offset jimminy-subject-bias-setting) t)
  (cond (jimminy-subject-timer (remem-cancel-timer jimminy-subject-timer)))
  (setq jimminy-subject-timer 
        (run-at-time jimminy-temporary-bias-lifetime nil 'jimminy-reset-subject-bias))
  (jimminy-update-changes))

(defun jimminy-set-location-bias (bias &optional bias-only)
  (interactive "nLocation bias: ")
  (if (< bias 0) (setq bias 0))
  (setq jimminy-location-bias bias)
  (cond ((not bias-only) (setq jimminy-location-bias-setting bias)))
  (setq remem-query-oneshot-preamble-string 
        (concat remem-query-oneshot-preamble-string
                "set-bias LOCATION " (number-to-string bias) "\n"))
  (jimminy-update-changes))
(defun jimminy-reset-location-bias () (jimminy-set-location-bias jimminy-location-bias-setting))
(defun jimminy-set-person-bias (bias &optional bias-only)
  (interactive "nPerson bias: ")
  (if (< bias 0) (setq bias 0))
  (setq jimminy-person-bias bias)
  (cond ((not bias-only) (setq jimminy-person-bias-setting bias)))
  (setq remem-query-oneshot-preamble-string 
        (concat remem-query-oneshot-preamble-string
                "set-bias PERSON " (number-to-string bias) "\n"))
  (jimminy-update-changes))
(defun jimminy-reset-person-bias () (jimminy-set-person-bias jimminy-person-bias-setting))
(defun jimminy-set-subject-bias (bias &optional bias-only)
  (interactive "nSubject bias: ")
  (if (< bias 0) (setq bias 0))
  (setq jimminy-subject-bias bias)
  (cond ((not bias-only) (setq jimminy-subject-bias-setting bias)))
  (setq remem-query-oneshot-preamble-string 
        (concat remem-query-oneshot-preamble-string
                "set-bias SUBJECT " (number-to-string bias) "\n"))
  (jimminy-update-changes))
(defun jimminy-reset-subject-bias () (jimminy-set-subject-bias jimminy-subject-bias-setting))
(defun jimminy-set-body-bias (bias &optional bias-only)
  (interactive "nBody bias: ")
  (if (< bias 0) (setq bias 0))
  (setq jimminy-body-bias bias)
  (cond ((not bias-only) (setq jimminy-body-bias-setting bias)))
  (setq remem-query-oneshot-preamble-string 
        (concat remem-query-oneshot-preamble-string
                "set-bias BODY " (number-to-string bias) "\n"))
  (jimminy-update-changes))
(defun jimminy-reset-body-bias () (jimminy-set-body-bias jimminy-body-bias-setting))

(defun jimminy-increment-location-bias ()
  (interactive)
  (jimminy-set-location-bias (+ 1 jimminy-location-bias)))
(defun jimminy-increment-person-bias ()
  (interactive)
  (jimminy-set-person-bias (+ 1 jimminy-person-bias)))
(defun jimminy-increment-subject-bias ()
  (interactive)
  (jimminy-set-subject-bias (+ 1 jimminy-subject-bias)))
(defun jimminy-increment-body-bias ()
  (interactive)
  (jimminy-set-body-bias (+ 1 jimminy-body-bias)))

(defun jimminy-decrement-location-bias ()
  (interactive)
  (jimminy-set-location-bias (- jimminy-location-bias 1)))
(defun jimminy-decrement-person-bias ()
  (interactive)
  (jimminy-set-person-bias (- jimminy-person-bias 1)))
(defun jimminy-decrement-subject-bias ()
  (interactive)
  (jimminy-set-subject-bias (- jimminy-subject-bias 1)))
(defun jimminy-decrement-body-bias ()
  (interactive)
  (jimminy-set-body-bias (- jimminy-body-bias 1)))

(defun jimminy-clear-location-string ()
  (interactive)
  (setq jimminy-location-string "")
  (jimminy-set-location-bias jimminy-location-bias-setting))
(defun jimminy-clear-person-string ()
  (interactive)
  (setq jimminy-person-string "")
  (jimminy-set-person-bias jimminy-person-bias-setting))
(defun jimminy-clear-subject-string ()
  (interactive)
  (setq jimminy-subject-string "")
  (jimminy-set-subject-bias jimminy-subject-bias-setting))
(defun jimminy-clear-all-string ()
  (interactive)
  (setq jimminy-person-string "")
  (setq jimminy-location-string "")
  (setq jimminy-subject-string "")
  (setq jimminy-location-bias 1)
  (setq jimminy-person-bias 1)
  (setq jimminy-subject-bias 1)
  (setq jimminy-body-bias 1)
  (setq jimminy-location-bias-setting 1)
  (setq jimminy-person-bias-setting 1)
  (setq jimminy-subject-bias-setting 1)
  (setq jimminy-body-bias-setting 1)
  (setq remem-query-oneshot-preamble-string 
        (concat
         "set-bias BODY " (number-to-string jimminy-body-bias) "\n"
         "set-bias LOCATION " (number-to-string jimminy-location-bias) "\n"
         "set-bias PERSON " (number-to-string jimminy-person-bias) "\n"
         "set-bias SUBJECT " (number-to-string jimminy-subject-bias) "\n"))
  (jimminy-update-changes))

;; I'm not sure that this context-stack stuff actually works
;; It's not being used, regardless
(defvar jimminy-context-stack ())
(defun jimminy-push-context ()
  (interactive)
  (setq jimminy-context-stack
	(cons (list jimminy-person-string
		    jimminy-location-string
		    jimminy-subject-string
		    jimminy-location-bias
		    jimminy-person-bias
		    jimminy-subject-bias
		    jimminy-body-bias)
	      jimminy-context-stack)))
(defun jimminy-pop-context ()
  (interactive)
  (cond (jimminy-context-stack
	 (let ((context (car jimminy-context-stack)))
	   (setq jimminy-person-string (elt context 0))
	   (setq jimminy-location-string (elt context 1))
	   (setq jimminy-subject-string (elt context 2))
	   (setq jimminy-location-bias (elt context 3))
	   (setq jimminy-person-bias (elt context 4))
	   (setq jimminy-subject-bias (elt context 5))
	   (setq jimminy-body-bias (elt context 6)))
	 (setq jimminy-context-stack (cdr jimminy-context-stack))
         (setq remem-query-oneshot-preamble-string 
               (concat
                "set-bias BODY " (number-to-string jimminy-body-bias) "\n"
                "set-bias LOCATION " (number-to-string jimminy-location-bias) "\n"
                "set-bias PERSON " (number-to-string jimminy-person-bias) "\n"
                "set-bias SUBJECT " (number-to-string jimminy-subject-bias) "\n"))
	 (jimminy-update-changes))
	((message "No previous context."))))
(defun jimminy-exch-context ()
  "Exchange the top two on the stack."
  (interactive)
  (cond ((> (length jimminy-context-stack) 1)
	 (let ((c1 (car jimminy-context-stack))
	       (c2 (car (cdr jimminy-context-stack))))
	   (setq jimminy-context-stack
		 (cons c2 (cons c1 (cdr (cdr jimminy-context-stack)))))))
	((message "Not enough contexts."))))
(defun jimminy-swap-context ()
  "Swap the current context with the top one on the stack."
  (interactive)
  (jimminy-push-context)
  (jimminy-exch-context)
  (jimminy-pop-context))


(defun jimminy-insert-header-info-at-top ()
  "insert header info string at start of buffer"
  (interactive)
  (save-excursion
    (goto-char (point-min))
    (let ((line-start (buffer-substring (point-min) (+ (point-min) 14))))
      (message line-start)
      (cond ((string= line-start "Jimminy-header")
	     (kill-line 1)))
      (jimminy-insert-header-info))))
  
(defun jimminy-update-header-info ()
  "Update the header above point in the buffer, or insert one at point"
  (interactive)
  (let ((p (point)))
    (save-excursion
      (cond ((re-search-backward "------------------------\nJimminy-header"
				 nil t)
	     (beginning-of-line)
	     (kill-line 2)))
      (beginning-of-line)
      (jimminy-insert-header-info))
    (next-line 1)
    (goto-char p)))

(defun jimminy-insert-header-info ()
  (interactive)
  (let ((outstring
	 (concat "------------------------\n"
		 "Jimminy-header <"
		 jimminy-location-string "|"
		 jimminy-person-string "|"
		 jimminy-subject-string "|"
		 (time-stamp-string)
		 ">\n")))
    (insert outstring)))






