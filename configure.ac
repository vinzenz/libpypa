AC_INIT([A python 2 parser],
        [0.1],
        [evilissimo@gmail.com],
        [pypa],
        [http://github.com/vinzenz/pypa])
AC_PREREQ([2.59])
AC_CONFIG_MACRO_DIR([m4])

m4_pattern_allow([AM_PROG_AR])

AM_INIT_AUTOMAKE([1.10 -Wall no-define subdir-objects])

AC_LANG_CPLUSPLUS
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11

AM_PROG_AR
AC_PROG_LIBTOOL

# check for extra compiler flags
AX_ADD_CXXFLAGS([-Werror]) # must be called first to trigger an error for the other unsupported options
AX_ADD_CXXFLAGS([-W])
AX_ADD_CXXFLAGS([-Wall])
AX_ADD_CXXFLAGS([-pedantic])
AX_ADD_CXXFLAGS([-Wno-unused-parameter])
AX_ADD_CXXFLAGS([-Wno-unused-local-typedefs])


# check for cpython sources
AC_ARG_WITH([cpython-src],
            [AS_HELP_STRING([--with-cpython-src],
                            [path to cpython sources, to feed the test suite])])
AC_SUBST([CPYTHON_SRC], [$with_cpython_src])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
AC_OUTPUT
