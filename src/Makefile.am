bin_PROGRAMS = enca
noinst_SCRIPTS = text2c.sh

enca_SOURCES = \
	COPYING.c \
	HELP.c \
	common.h \
	convert.c \
	convert_extern.c \
	convert_recode.c \
	convert_iconv.c \
	enca.c \
	filebuf.c \
	locale_detect.c \
	options.c

EXTRA_enca_SOURCES = \
	getopt.h

BUILT_SOURCES = \
	COPYING.c \
	HELP.c

EXTRA_DIST = \
	HELP.in \
	text2c.sh

enca_LDADD = @LIBOBJS@ @CONVERTER_LIBS@ ../lib/libenca.la
enca_LDFLAGS = @LDFLAGS@ @LIBS@
enca_DEPENDENCIES = @LIBOBJS@ ../lib/libenca.la
enca_CPPFLAGS = -DEXTCONV_DIR=\"@libexecdir@/enca/extconv\"
AM_CPPFLAGS = -I$(top_srcdir)/lib $(GCOV_CPPFLAGS)
AM_LDFLAGS =  $(GCOV_LDFLAGS)
AM_CFLAGS =  $(GCOV_FLAGS)

# BUILT_SOURCES.  How we build them:

# For inclusion of GNU General Public License directly into Enca
COPYING.c: $(top_srcdir)/COPYING $(srcdir)/text2c.sh
	$(srcdir)/text2c.sh $(top_srcdir)/COPYING >COPYING.c

HELP.c: HELP $(srcdir)/text2c.sh
	$(srcdir)/text2c.sh HELP >HELP.c

install-exec-hook:
	cd $(DESTDIR)$(bindir); \
	inst1=`echo enca | sed '$(transform)'`; \
	inst2=`echo enconv | sed '$(transform)'`; \
	$(LN_S) -f $$inst1$(EXEEXT) $$inst2$(EXEEXT)

uninstall-hook:
	inst=`echo enconv | sed '$(transform)'`; \
	rm -f $(DESTDIR)$(bindir)/$$inst$(EXEEXT)

clean-local:
	rm -f core.* *~ HELP.c COPYING.c
	rm -f *.gc*

dist-hook:
	rm -f $(distdir)/HELP.c $(distdir)/COPYING.c

