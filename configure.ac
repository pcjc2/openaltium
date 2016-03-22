#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.


AC_PREREQ([2.67])
AC_INIT([openaltium], [0.0.1], [pcjc2@cam.ac.uk])
AC_CONFIG_SRCDIR([libopenaltium/read_data.c])
AC_CONFIG_AUX_DIR([build-tools])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC

AC_LIBTOOL_WIN32_DLL
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AM_PROG_CC_C_O

# Checks for libraries using pkg-config

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.12.0], ,
  AC_MSG_ERROR([GLib 2.12.0 or later is required.]))

# Checks for libraries

AC_SEARCH_LIBS([getopt_long], [gnugetopt],
               AC_DEFINE([HAVE_GETOPT_LONG], 1,
                         [Define to 1 if you have the `getopt_long' function.]))

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
#AC_CHECK_FUNCS([strdup])

AC_CONFIG_FILES([Makefile
                libopenaltium/Makefile])
AC_OUTPUT
