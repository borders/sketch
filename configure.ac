#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([sketch], [0.1], [James.W.Borders@gmail.com])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.


PKG_CHECK_MODULES(GTK2, gtk+-2.0 >= 1.0)

AC_CONFIG_FILES([Makefile \
                 src/Makefile])
AC_OUTPUT
