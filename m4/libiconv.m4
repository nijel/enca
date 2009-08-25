## Several iconv tests.  This file is in public domain.
## N.B.: Following test relies on files iconvcap.c and has to compile and
##       run it.
## Defines:
## HAVE_ICONV when have iconv_open(), iconv.h and it's usable
## ICONV_IS_TRANSITIVE when it seems no triangulation is needed.
## LIBS (adds library when needed)
## Creates file iconvenc.h (to be included only if defined(HAVE_ICONV))---it
##   would be cleaner to put all the definitions to config.h, but then
##   (1) we would have to check for every charset separately
##   (2) config.h would get polluted by dozens of charset aliases
## Note we need tools/iconvenc.null
AC_DEFUN([ye_CHECK_FUNC_ICONV_USABLE],
[AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_HEADER_STDC])dnl
AC_REQUIRE([AC_C_CONST])dnl
dnl

dnl Test for iconv.
dnl We have to test several things.  First, whether is possible to compile/link
dnl with iconv. Then, whether it makes sense---many iconv implementations don't
dnl provide anything useful, specifically we should check for GNU iconv, since
dnl it doesn't need triangulation, thus allowing simplier interface.
AC_ARG_WITH(libiconv,
  [  --with-libiconv@<:@=DIR@:>@   look for libiconv in DIR/lib and DIR/include @<:@auto@:>@],
  [case "$withval" in
    yes|auto) WANT_LIBICONV=1 ;;
    no)  WANT_LIBICONV=0 ;;
    *)   WANT_LIBICONV=1 ; yeti_libiconv_CPPFLAGS="-I$withval/include" ; yeti_libiconv_LDFLAGS="-L$withval/lib" ;;
    esac],
  [WANT_LIBICONV=1])

if test "$WANT_LIBICONV" = 1; then
  yeti_save_LIBS="$LIBS"
  yeti_save_CPPFLAGS="$CPPFLAGS"
  yeti_save_LDFLAGS="$LDFLAGS"
  CPPFLAGS="$CPPFLAGS $yeti_libiconv_CPPFLAGS"
  LDFLAGS="$LDFLAGS $yeti_libiconv_LDFLAGS"
  AC_SEARCH_LIBS(iconv_open,iconv,libiconv_ok=yes,libiconv_ok=no)
  if test "$libiconv_ok" = yes; then
    AC_CHECK_HEADER(iconv.h,
      [libiconv_ok=yes
      if test "$ac_cv_search_iconv_open" != "none required"; then
        CONVERTOR_LIBS="$CONVERTOR_LIBS $ac_cv_search_iconv_open"
      fi],
      libiconv_ok=no)
  fi
  LIBS="$yeti_save_LIBS"
fi
dnl Check whether iconv() second argument should be declared const.  All
dnl systems I met except RedHat Linux seem to have it const.
if test "$libiconv_ok" = yes; then
  AC_CACHE_CHECK([whether iconv second argument needs const],
    yeti_cv_proto_iconv_arg2_const,
    AC_TRY_COMPILE([#include <stdlib.h>
     #include <iconv.h>
     extern
     #ifdef __cplusplus
     "C"
     #endif
     size_t iconv(iconv_t, const char**, size_t*, char**, size_t*);],
     [],
     yeti_cv_proto_iconv_arg2_const=yes,
     yeti_cv_proto_iconv_arg2_const=no,))
  if test "$yeti_cv_proto_iconv_arg2_const" = yes; then
    yeti_ac_tmp=const
  else
    yeti_ac_tmp="/* */"
  fi
  AC_DEFINE_UNQUOTED(ICONV_ARG2_CONST,
    $yeti_ac_tmp,
    [Define as const when iconv() second argument needs const.])
fi
dnl Compile iconvcap.c and run it to determine what encodings iconv actually
dnl knows an under what names. This is not needed with GNU iconv. HAVE_ICONV
dnl is finally defined _only_ if iconv prove at least some minimal reasonable
dnl capabilities.
if test "$libiconv_ok" = yes; then
  AC_MSG_CHECKING([whether iconv implementation is usable])
  if $CC -o iconvcap$ac_exeext $CFLAGS $CPPFLAGS $LDFLAGS $srcdir/iconvcap.c $LIBS $CONVERTOR_LIBS 1>&5 2>&5 && test -s ./iconvcap$ac_exeext 2>&5; then
    if ./iconvcap 2>&5 >iconvenc.h; then
      libiconv_ok=yes
    else
      libiconv_ok=no
    fi
  else
    libiconv_ok=no
  fi
  AC_MSG_RESULT($libiconv_ok)
  if test "$libiconv_ok" = yes; then
    AC_DEFINE(HAVE_ICONV,1,[Define if you have the UNIX98 iconv functions.])
    AC_CACHE_CHECK([whether iconv is transitive],
      yeti_cv_lib_c_iconv_transitive,
      if ./iconvcap iconvenc.h 2>&5; then
        yeti_cv_lib_c_iconv_transitive=yes
      else
        yeti_cv_lib_c_iconv_transitive=no
      fi)
    if test "$yeti_cv_lib_c_iconv_transitive" = yes; then
      AC_DEFINE(ICONV_IS_TRANSITIVE,1,[Define if iconv is transitive.])
    else
      AC_MSG_WARN([A non-transitive but otherwise usable iconv implementation
                    was found.  This beast was believed to be mythical.
                    Please send your system specs to the maintainer.])
    fi
  else
    echo >iconvenc.h
  fi
else
  libiconv_ok=no
fi

if test "$libiconv_ok" != "yes"; then
  cat $srcdir/tools/iconvenc.null >iconvenc.h
  if test "$WANT_LIBICONV" = 1; then
    CPPFLAGS="$yeti_save_CPPFLAGS"
    LDFLAGS="$yeti_save_LDFLAGS"
  fi
fi])

