AM_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	$(GIO_CFLAGS) \
	$(UDISKS2_CFLAGS) \
	$(CRYPTO_CFLAGS) \
	-DPACKAGE_LIBEXEC_DIR=\""$(libexecdir)"\"

LIBS = \
	$(GLIB_LIBS) \
	$(GIO_LIBS) \
	$(UDISKS2_LIBS) \
	$(CRYPTO_LIBS)

dbus_built_sources = lcsmgr-generated.h lcsmgr-generated.c

$(dbus_built_sources) : $(top_srcdir)/data/org.freedesktop.LcsMgrService.xml
	gdbus-codegen --interface-prefix org.freedesktop.LcsMgrService --c-namespace LcsMgr --generate-c-code=lcsmgr-generated $<

BUILT_SOURCES =										\
	$(dbus_built_sources)								\
	$(NULL)

lcsmgrserviceprivdir = $(libexecdir)/lcsmgrservice
lcsmgrservicepriv_PROGRAMS = lcsmgrserviced
lcsmgrserviced_SOURCES = main.c lcsmgrservice.c common.c hwinfo.c md5.c $(BUILT_SOURCES)

CLEANFILES = $(BUILT_SOURCES)

