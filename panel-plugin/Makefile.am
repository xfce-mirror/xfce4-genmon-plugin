plugindir = $(libexecdir)/xfce4/panel-plugins
plugin_PROGRAMS = xfce4-genmon-plugin

xfce4_genmon_plugin_CFLAGS =					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"			\
	@LIBXFCE4PANEL_CFLAGS@					\
	@LIBXFCEGUI4_CFLAGS@

xfce4_genmon_plugin_LDADD =					\
	@LIBXFCE4PANEL_LIBS@					\
	@LIBXFCEGUI4_LIBS@

xfce4_genmon_plugin_SOURCES =	\
	main.c			\
	config_gui.h		\
	config_gui.c		\
	cmdspawn.h		\
	cmdspawn.c

# .desktop file
#
# Some automake trickery here. Because we cannot use $(libexecdir) in the
# automake stage, we'll use sed to get the full path into the .desktop file.
# We also need to let intltool merge the translated fields, so we add an
# additional level of indirection: a <name>.desktop.in.in file.
# 
desktop_in_in_files = genmon.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in)
	
desktopdir = $(datadir)/xfce4/panel-plugins
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = 								\
	$(desktop_in_in_files)

DISTCLEANFILES =							\
	$(desktop_DATA) $(desktop_in_files)
	
# get full path into .desktop file
%.desktop.in: %.desktop.in.in
	sed -e "s^@PLUGIN_PATH@^$(libexecdir)/xfce4/panel-plugins^" \
		$< > $@

