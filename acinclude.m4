# AC_GNU_SOURCE
# --------------
AC_DEFUN([AC_GNU_SOURCE],
[AH_VERBATIM([_GNU_SOURCE],
[/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# undef _GNU_SOURCE
#endif])dnl
AC_BEFORE([$0], [AC_COMPILE_IFELSE])dnl
AC_BEFORE([$0], [AC_RUN_IFELSE])dnl
AC_DEFINE([_GNU_SOURCE])
])

# AC_PROG_EGREP
# -------------
AC_DEFUN([AC_PROG_EGREP],
[AC_CACHE_CHECK([for egrep], [ac_cv_prog_egrep],
   [if echo a | (grep -E '(a|b)') >/dev/null 2>&1
    then ac_cv_prog_egrep='grep -E'
    else ac_cv_prog_egrep='egrep'
    fi])
 EGREP=$ac_cv_prog_egrep
 AC_SUBST([EGREP])
])# AC_PROG_EGREP


# AC_TYPE_MBSTATE_T
# -----------------
AC_DEFUN([AC_TYPE_MBSTATE_T],
  [AC_CACHE_CHECK([for mbstate_t], ac_cv_type_mbstate_t,
     [AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM(
           [AC_INCLUDES_DEFAULT
#           include <wchar.h>],
           [mbstate_t x; return sizeof x;])],
        [ac_cv_type_mbstate_t=yes],
        [ac_cv_type_mbstate_t=no])])
   if test $ac_cv_type_mbstate_t = yes; then
     AC_DEFINE([HAVE_MBSTATE_T], 1,
               [Define to 1 if <wchar.h> declares mbstate_t.])
   else
     AC_DEFINE([mbstate_t], int,
               [Define to a type if <wchar.h> does not define.])
   fi])
