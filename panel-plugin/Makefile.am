plugindir = @XFCE4_PANEL_PLUGINSDIR@

plugin_LTLIBRARIES =		\
	libgenmon.la

libgenmon_la_LDFLAGS =		\
	-avoid-version		\
	-export-dynamic		\
	-module

libgenmon_la_SOURCES =	\
	main.c			\
	config_gui.h		\
	config_gui.c		\
	cmdspawn.h		\
	cmdspawn.c		\
	debug.h			\
	debug.c			

libgenmon_la_CFLAGS =		\
	@XFCE4_PANEL_CFLAGS@

libgenmon_la_LIBADD =		\
	@XFCE4_PANEL_LIBS@
