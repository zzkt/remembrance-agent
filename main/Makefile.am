bin_PROGRAMS=ra-index ra-retrieve

# Yes, Virginia, order in the LDADD line -does- matter, because "ld"
# is documented to only look for any symbols in a library if there are
# already unresolved symbols.  In particular, if you put savutil before
# savantio, then, when compiling ra-merge, there -aren't- any unresolved
# symbols when savutil is scanned, hence it's skipped.  Then, when savantio
# is scanned, it asks for symbols that are in savutil, but by then savutil
# has already been scanned, and ld doesn't arrange to go back and look again.
# This foolishness could be solved with either a table or a second pass, but
# apparently it's considered a feature that ld doesn't do this.  --- Foner.
#
# I've rearranged the -L's and the -l's to be in the same order relative to
# each other, so it's easy to figure out who includes what from where; please
# don't gratuitiously disrupt that order (i.e., if you change the ordering of
# LDADD, then change LDFLAGS to match).  --- Foner.

# Note: I've added /sw/lib to the library directories so Mac OS-X people who use
# fink can find pcre. This should almost certainly be moved into an autoconf 
# command, once I figure out how such things are done.  -- Brad 9/21/03

ra_index_SOURCES=imain.c 
ra_index_LDADD=-ltemplate -lplugins -ldate -lm -lpcre -lsavutil
# -lz
ra_index_LDFLAGS=-L$(top_builddir)/template -L$(top_builddir)/plugins \
                 -L$(top_builddir)/savutil/parsedate \
		 -L/sw/lib \
                 -L$(top_builddir)/savutil 
#                -L$(top_builddir)/savutil/pcre 
#                -L$(top_builddir)/savutil/zlib 
ra_index_DEPENDENCIES=$(top_builddir)/savutil/libsavutil.a \
	$(top_builddir)/savutil/parsedate/libdate.a \
	$(top_builddir)/plugins/libplugins.a \
	$(top_builddir)/template/libtemplate.a
#	$(top_builddir)/savutil/pcre/libpcreposix.a 
#	$(top_builddir)/savutil/pcre/libpcre.a 
#	$(top_builddir)/savutil/zlib/libz.a 

ra_retrieve_SOURCES=rmain.c 
ra_retrieve_LDADD=-ltemplate -lplugins -ldate -lm -lpcre -lsavutil
# -lz
ra_retrieve_LDFLAGS=-L$(top_builddir)/template -L$(top_builddir)/plugins \
	-L$(top_builddir)/savutil/parsedate \
	-L/sw/lib \
	-L$(top_builddir)/savutil 
#       -L$(top_builddir)/savutil/pcre
#       -L$(top_builddir)/savutil/zlib
ra_retrieve_DEPENDENCIES=$(top_builddir)/savutil/libsavutil.a \
	$(top_builddir)/savutil/parsedate/libdate.a \
	$(top_builddir)/plugins/libplugins.a \
	$(top_builddir)/template/libtemplate.a
#	$(top_builddir)/savutil/pcre/libpcreposix.a 
#	$(top_builddir)/savutil/pcre/libpcre.a 
#	$(top_builddir)/savutil/zlib/libz.a 

EXTRA_DIST=rmain.h

$(top_builddir)/savutil/parsedate/libdate.a:
	cd $(top_builddir)/savutil/parsedate; make libdate.a

#$(top_builddir)/savutil/pcre/libpcre.a:
#	cd $(top_builddir)/savutil/pcre; make libpcre.a

#$(top_builddir)/savutil/pcre/libpcreposix.a:
#	cd $(top_builddir)/savutil/pcre; make libpcreposix.a

#$(top_builddir)/savutil/zlib/libz.a:
#	cd $(top_builddir)/savutil/zlib; make libz.a

$(top_builddir)/savutil/libsavutil.a:
	cd $(top_builddir)/savutil; make libsavutil.a

$(top_builddir)/template/libtemplate.a:
	cd $(top_builddir)/template; make libtemplate.a

$(top_builddir)/plugins/libplugins.a:
	cd $(top_builddir)/plugins; make libplugins.a


