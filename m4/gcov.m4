# AM_GCOV

AC_DEFUN([AM_GCOV],
[

AC_ARG_WITH( gcov,
    [  --with-gcov           compile for coverage tests using gcov (default=no)],
    [ 
if test "$GCC" = "yes" 
then
        using_gcov=$with_gcov
fi 
]
)

AC_PATH_PROG([GCOV], [gcov])
if test "$GCOV" -a "$using_gcov" = "yes"
then
    GCOV_FLAGS="-fprofile-arcs -ftest-coverage"
    GCOV_CPPFLAGS=${GCOV_FLAGS}
    GCOV_LDFLAGS=${GCOV_FLAGS}
    AC_SUBST(GCOV_FLAGS)
    AC_SUBST(GCOV_CPPFLAGS)
    AC_SUBST(GCOV_LDFLAGS)
fi
])
