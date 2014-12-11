lisp_LISP=remem.el 
## don't compile jimminy.el, 'cause it relies on remem.el and when making that won't be in a path yet.
## I know there's a better way to do this with Emacs, but I haven't done it yet.

EXTRA_DIST=remem.el jimminy.el ra-index.1 ra-retrieve.1

## Grr.
mostlyclean-lisp clean-lisp distclean-lisp maintainer-clean-lisp:
	rm -f $(ELCFILES)
