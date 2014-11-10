dnl check if the compiler supports $1 as an extra flag
dnl if so, add it to the CXXFLAGS
AC_DEFUN([AX_ADD_CXXFLAGS],
         [pypa_save_CXXFLAGS="$CXXFLAGS"
          CXXFLAGS="$CXXFLAGS $1"
          AC_MSG_CHECKING([if $CXX supports $1])
          AC_COMPILE_IFELSE([AC_LANG_SOURCE([int pypa = 1;])],
                            [AC_MSG_RESULT([yes])],
                            [CXXFLAGS=$pypa_save_CXXFLAGS
                             AC_MSG_RESULT([no])])
         ])
