
NULL =

dbusservicedir       = $(datadir)/dbus-1/system-services
dbusservice_in_files = org.freedesktop.LcsMgrService.service.in
dbusservice_DATA     = $(dbusservice_in_files:.service.in=.service)

$(dbusservice_DATA): $(dbusservice_in_files) Makefile
	@sed -e "s|\@lcsmgrserviceprivdir\@|$(libexecdir)/lcsmgrservice|" $< > $@

dbusconfdir = $(sysconfdir)/dbus-1/system.d
dbusconf_in_files = org.freedesktop.LcsMgrService.conf.in
dbusconf_DATA = $(dbusconf_in_files:.conf.in=.conf)

$(dbusconf_DATA): $(dbusconf_in_files) Makefile
	cp $< $@

systemdservice_in_files = lcsmgrservice.service.in

if HAVE_SYSTEMD
systemdservicedir       = $(systemdsystemunitdir)
systemdservice_DATA     = $(systemdservice_in_files:.service.in=.service)

$(systemdservice_DATA): lcsmgrservice.service.in Makefile
	@sed -e "s|\@lcsmgrserviceprivdir\@|$(libexecdir)/lcsmgrservice|" lcsmgrservice.service.in > lcsmgrservice.service
endif


EXTRA_DIST =                                                                   \
	org.freedesktop.LcsMgrService.xml                                            \
	$(systemdservice_in_files)                                             \
	$(dbusservice_in_files)                                                \
	$(dbusconf_in_files)                                                   \
	$(NULL)

clean-local:
	rm -f *~                                                               \
	$(dbusservice_DATA)                                                    \
	$(dbusconf_DATA)                                                       \
	$(systemdservice_DATA)
