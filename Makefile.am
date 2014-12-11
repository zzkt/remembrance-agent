SUBDIRS=savutil plugins template main other 

EXTRA_DIST= savant.h COPYING ChangeLog

all: main/ra-index main/ra-retrieve

main/ra-index:
	cd main; $(MAKE) ra-index ra-retrieve
