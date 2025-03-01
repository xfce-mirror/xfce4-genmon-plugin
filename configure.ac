dnl configure.ac
dnl
dnl xfce4-genmon-plugin - Generic Monitor plugin for xfce4-panel
dnl
dnl 2004 Roger Seguin <roger_seguin@msn.com>
dnl 2006 Julien Devemy <jujucece@gmail.com>
dnl 2016 Tony Paulic <tony.paulic@gmail.com>
dnl

m4_define([copyright_year], [2025])
XDT_VERSION_INIT([4.2.1], [git])

AC_COPYRIGHT([Copyright (c) 2005-copyright_year() The Xfce development team. All rights reserved.])
AC_INIT([xfce4-genmon-plugin], [xdt_version], [https://gitlab.xfce.org/panel-plugins/xfce4-genmon-plugin])
AC_REVISION([xdt_version_build])
AC_PREREQ([2.69])

AM_INIT_AUTOMAKE([1.8 no-dist-gzip dist-bzip2 tar-ustar foreign])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE
AC_CONFIG_MACRO_DIRS([m4])
AC_DEFINE([VERSION_FULL], [PACKAGE_VERSION], [Alias for VERSION and PACKAGE_VERSION for meson compatibility])
COPYRIGHT_YEAR=copyright_year()
AC_DEFINE_UNQUOTED([COPYRIGHT_YEAR], ["$COPYRIGHT_YEAR"], [Copyright year])
AC_SUBST([COPYRIGHT_YEAR])

dnl Check for UNIX variants
AC_USE_SYSTEM_EXTENSIONS

dnl Check for basic programs
AC_PROG_CC
AC_PROG_INSTALL

dnl Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

dnl configure the panel plugin
XDT_CHECK_PACKAGE([GLIB], [glib-2.0], [2.50.0])
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.22.0])
XDT_CHECK_PACKAGE([LIBXFCE4PANEL], [libxfce4panel-2.0], [4.16.0])
XDT_CHECK_PACKAGE([LIBXFCE4UI], [libxfce4ui-2], [4.16.0])
XDT_CHECK_PACKAGE([LIBXFCE4UTIL], [libxfce4util-1.0], [4.16.0])
XDT_CHECK_PACKAGE([XFCONF], [libxfconf-0], [4.16.0])

dnl Translations
GETTEXT_PACKAGE="$PACKAGE"
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Name of default gettext domain])
AC_SUBST([GETTEXT_PACKAGE])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])

dnl Check for debugging support
XDT_FEATURE_DEBUG([xdt_debug_default])

AC_CONFIG_FILES([
Makefile
panel-plugin/Makefile
data/Makefile
data/icons/Makefile
data/icons/16x16/Makefile
data/icons/48x48/Makefile
data/icons/128x128/Makefile
data/icons/scalable/Makefile
scripts/Makefile
po/Makefile.in
])
AC_OUTPUT
