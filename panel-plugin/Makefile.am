plugindir = $(libdir)/xfce4/panel/plugins
plugin_LTLIBRARIES = libgenmon.la

libgenmon_la_CFLAGS =						\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"			\
	@LIBXFCE4PANEL_CFLAGS@					\
	@LIBXFCE4UI_CFLAGS@

libgenmon_la_LDFLAGS = 						\
	-avoid-version 						\
	-module 						\
	-no-undefined 						\
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)'

libgenmon_la_LIBADD =						\
	@LIBXFCE4PANEL_LIBS@					\
	@LIBXFCE4UI_LIBS@

libgenmon_la_SOURCES =		\
	main.c			\
	config_gui.h		\
	config_gui.c		\
	cmdspawn.h		\
	cmdspawn.c

# .desktop file
#
# Some automake trickery here. Because we cannot use $(libdir) in the
# automake stage, we'll use sed to get the full path into the .desktop file.
# We also need to let intltool merge the translated fields, so we add an
# additional level of indirection: a <name>.desktop.in.in file.
# 
desktop_in_in_files = genmon.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in)
	
desktopdir = $(datadir)/xfce4/panel/plugins
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = 								\
	$(desktop_in_in_files)

DISTCLEANFILES =							\
	$(desktop_DATA) $(desktop_in_files)
	
# get full path into .desktop file
$(desktop_in_files): $(desktop_in_in_files)
	sed -e "s^@PLUGIN_PATH@^$(plugindir)^" \
		$< > $@

