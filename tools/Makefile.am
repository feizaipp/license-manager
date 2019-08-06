AM_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	$(GIO_CFLAGS) \
	$(CRYPTO_CFLAGS) \
	$(NULL)

LIBS = \
	$(GLIB_LIBS) \
	$(GIO_LIBS) \
	$(NULL) \
	$(CRYPTO_LIBS)

dbus_built_sources = lcsmgr-generated.h lcsmgr-generated.c

$(dbus_built_sources) : $(top_srcdir)/data/org.freedesktop.LcsMgrService.xml
	gdbus-codegen --interface-prefix org.freedesktop.LcsMgrService --c-namespace LcsMgr --generate-c-code=lcsmgr-generated $<

BUILT_SOURCES =										\
	$(dbus_built_sources)								\
	$(NULL)

bin_PROGRAMS = lsmgr lsmgrsign

lsmgr_SOURCES = lsmgr.c $(BUILT_SOURCES)
lsmgrsign_SOURCES = lsmgrsign.c  $(BUILT_SOURCES)