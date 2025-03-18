scriptsdir = $(datadir)/xfce4/genmon/scripts

scripts_DATA = datetime			\
	disktemp			\
	dkspuse				\
	gmail				\
	googcal				\
	migrate_to_xfconf.sh		\
	monBandwidth			\
	monBat				\
	monCPU				\
	monTime				\
	monUSB				\
	monWIFI				\
	sysstat

googcal: googcal.in Makefile
	$(AM_V_GEN) $(SED) -e "s,\@OPEN_CMD\@,exo-open,g" $< >$@

EXTRA_DIST =				\
	$(scripts_DATA)			\
	googcal.in			\
	meson.build			\
	$(NULL)

DISTCLEANFILES = \
	googcal \
	$(NULL)
