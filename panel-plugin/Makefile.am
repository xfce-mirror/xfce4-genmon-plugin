plugindir = $(libdir)/xfce4/panel/plugins
plugin_LTLIBRARIES = libgenmon.la

libgenmon_la_CFLAGS =						\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"			\
	@LIBXFCE4PANEL_CFLAGS@					\
	@LIBXFCE4UI_CFLAGS@					\
	@XFCONF_CFLAGS@

libgenmon_la_LDFLAGS = 						\
	-avoid-version 						\
	-module 						\
	-no-undefined 						\
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)'

libgenmon_la_LIBADD =						\
	@LIBXFCE4PANEL_LIBS@					\
	@LIBXFCE4UI_LIBS@					\
	@XFCONF_LIBS@


libgenmon_la_SOURCES =		\
	main.c			\
	config_gui.h		\
	config_gui.c		\
	cmdspawn.h		\
	cmdspawn.c

desktopdir = $(datadir)/xfce4/panel/plugins
desktop_in_files = genmon.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
$(desktop_DATA): $(desktop_DATA).in Makefile
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

EXTRA_DIST = 								\
	$(desktop_in_files)

DISTCLEANFILES =							\
	$(desktop_DATA)

