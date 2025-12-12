AC_INIT([frobby], [0.9.9])
AM_INIT_AUTOMAKE([foreign subdir-objects])
LT_INIT

AC_SUBST([LIBFROBBY_VERSION_INFO], [1:4:0])

AC_PROG_CC
AC_PROG_CXX

# check for GMP
AC_DEFUN([FROBBY_NO_GMP], [AC_MSG_ERROR([could not find GMP])])
AC_LANG([C])
AC_CHECK_HEADERS([gmp.h],, [FROBBY_NO_GMP])
AC_SEARCH_LIBS([__gmpz_init], [gmp],, [FROBBY_NO_GMP])
AC_LANG([C++])
AC_CHECK_HEADERS([gmpxx.h],, [FROBBY_NO_GMP])
LIBS="$LIBS -lgmpxx"
AC_MSG_CHECKING([for libgmpxx])
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([
	#include <gmpxx.h>
	], [mpz_class x(5); x++])],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     FROBBY_NO_GMP])

AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile])
AC_OUTPUT
