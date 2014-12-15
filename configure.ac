dnl -*- mode: m4 -*-
AC_PREREQ([2.68])
AC_INIT([gnome-reader],[0.0.1],[],[gnome-reader])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/reader-application.c])
AC_CONFIG_MACRO_DIR([m4])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# require automake 1.11
AM_INIT_AUTOMAKE([1.11 dist-xz no-dist-gzip tar-ustar])
AM_MAINTAINER_MODE

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
IT_PROG_INTLTOOL([0.35.0])
PKG_PROG_PKG_CONFIG([0.22])

GETTEXT_PACKAGE=gnome-reader
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

AM_GLIB_GNU_GETTEXT

## don't rerun to this point if we abort
# AC_CACHE_SAVE

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_SEARCH_LIBS([strerror],[cposix])
AC_HEADER_STDC

# no stupid static libraries
AC_DISABLE_STATIC([])
# enable libtool
LT_INIT

LIBGD_INIT([view_common main_view main_icon_view main_list_view])

## don't rerun to this point if we abort
AC_CACHE_SAVE

GLIB_GSETTINGS

dnl ================================================================
dnl Misc
dnl ================================================================
AC_PATH_PROG(GLIB_COMPILE_RESOURCES, glib-compile-resources)

GNOME_COMMON_INIT
GNOME_COMPILE_WARNINGS(maximum)
GNOME_MAINTAINER_MODE_DEFINES

MAINTAINER_COMPILER_FLAGS="$MAINTAINER_COMPILER_FLAGS                        \
                           -Wall -Wcast-align -Wuninitialized                \
                           -Wno-strict-aliasing -Wempty-body -Wformat        \
                           -Wformat-security -Wformat-nonliteral -Winit-self \
                           -Wdeclaration-after-statement -Wvla               \
                           -Wpointer-arith -Wmissing-declarations            \
                           -Wcast-align                                      \
                           -Wredundant-decls"

# strip leading spaces
MAINTAINER_COMPILER_FLAGS=${MAINTAINER_COMPILER_FLAGS#*  }
AC_SUBST(MAINTAINER_COMPILER_FLAGS)

PKG_CHECK_MODULES(READER,
        gtk+-3.0 >= 3.14.5
        tracker-sparql-1.0 >= 1.2.4
        webkit2gtk-3.0 >= 2.2.6
        libgrss-0.5 >= 0.5.0
)

AC_CONFIG_FILES([
      Makefile
      libgd/Makefile
      src/Makefile
      po/Makefile.in
])

AC_OUTPUT

dnl ==========================================================================
echo "

        gnome-reader $VERSION
        ==================

        prefix:    ${prefix}
        compiler:  ${CC}

        Now type 'make' to build $PACKAGE
"
