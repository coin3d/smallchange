
m4_define([SMALLCHANGE_MAJOR], [1])
m4_define([SMALLCHANGE_MINOR], [0])
m4_define([SMALLCHANGE_MICRO], [0])

# this is bogus, but policy has not been set for this yet
m4_define([SC_ABI_CURRENT], [1])
m4_define([SC_ABI_AGE], [0])
m4_define([SC_ABI_REVISION], [0])


AC_INIT([SmallChange], [1.0.0], [coin-bugs@coin3d.org])

AC_CONFIG_SRCDIR(lib/SmallChange/nodes/DepthBuffer.cpp)
AC_CONFIG_AUX_DIR(cfg)

# If the Microsoft Visual C++ cl.exe compiler is available, set us up for
# compiling with it and to generate an MSWindows .dll file.

AC_CANONICAL_SYSTEM

SIM_AC_SETUP_MSVCPP_IFELSE

if $BUILD_WITH_MSVC; then
  SIM_AC_SETUP_MSVCRT([
    CPPFLAGS="$CPPFLAGS $sim_ac_msvcrt_CPPFLAGS"
    CFLAGS="$CFLAGS $sim_ac_msvcrt_CFLAGS"
    CXXFLAGS="$CXXFLAGS $sim_ac_msvcrt_CXXFLAGS"
    LDFLAGS="$LDFLAGS $sim_ac_msvcrt_LDFLAGS"
    LIBS="$LIBS $sim_ac_msvcrt_LIBS"
  ])
fi

AM_CONDITIONAL(BUILD_WITH_MSVC, $BUILD_WITH_MSVC)

AC_SUBST([SMALLCHANGE_MAJOR_VERSION], [SMALLCHANGE_MAJOR])
AC_SUBST([SMALLCHANGE_MINOR_VERSION], [SMALLCHANGE_MINOR])
AC_SUBST([SMALLCHANGE_MICRO_VERSION], [SMALLCHANGE_MICRO])
AC_SUBST([SMALLCHANGE_VERSION], [SMALLCHANGE_MAJOR.SMALLCHANGE_MINOR.SMALLCHANGE_MICRO])

VERSION=$SMALLCHANGE_VERSION

SC_MAJOR=$SMALLCHANGE_MAJOR_VERSION

# Libtool versioning
AC_SUBST([LT_CURRENT], [SC_ABI_CURRENT])
AC_SUBST([LT_REVISION], [SC_ABI_REVISION])
AC_SUBST([LT_AGE], [SC_ABI_AGE])

AC_DEFINE_UNQUOTED([SMALLCHANGE_MAJOR_VERSION], [$SMALLCHANGE_MAJOR_VERSION],
  [define this to the libSmallchange major version number])
AC_DEFINE_UNQUOTED([SMALLCHANGE_MINOR_VERSION], [$SMALLCHANGE_MINOR_VERSION],
  [define this to the libSmallchange minor version number])
AC_DEFINE_UNQUOTED([SMALLCHANGE_MICRO_VERSION], [$SMALLCHANGE_MICRO_VERSION],
  [define this to the libSmallchange release version number])
AC_DEFINE_UNQUOTED([SMALLCHANGE_VERSION], ["$SMALLCHANGE_VERSION"],
  [define this to the full libSmallchange major.minor.micro version number])

# *******************************************************************
# Miscellaneous options and initializations.

smallchange_build_dir=`pwd`

# **************************************************************************
#  Locate C++ compiler and set C++ as the default language to use
#  in tests. The configure script will automatically terminate if
#  it doesn't find a C++ compiler.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_LANG_CPLUSPLUS

AC_CHECK_HEADERS([windows.h unistd.h ctype.h sys/types.h dirent.h])

AM_INIT_AUTOMAKE(SmallChange, $VERSION)

#  Files to cram results from the configure tests into.
AM_CONFIG_HEADER(config.h)

#  Default to not building a shared library.
#  Can be overridden by the user with --enable-shared.
AM_DISABLE_SHARED

#  Turn off default maintainer make-rules -- use ./bootstrap instead.
AM_MAINTAINER_MODE

if $BUILD_WITH_MSVC; then
  RANLIB=true
  export RANLIB
else

#  Initialize libtool
AC_PROG_LIBTOOL

# case "$host_os" in
#   irix*)
#     if test x"${AR+set}" != x"set"; then
#       if test x"$CXX" = xCC; then
#         echo "modifying libtool"
#         mv libtool libtool.orig
#         sed -e 's/\\\$AR cru/CC -ar -o/' <libtool.orig >libtool
#         chmod 755 libtool
#         rm -f libtool.org
#       fi
#     fi
#     ;;
# esac

fi

SIM_AC_DEBUGSYMBOLS
SIM_AC_RTTI_SUPPORT
SIM_AC_EXCEPTION_HANDLING
SIM_AC_PROFILING_SUPPORT
SIM_AC_COMPILER_WARNINGS

CPPFLAGS="$CPPFLAGS -DSMALLCHANGE_INTERNAL"

case $enable_static in
yes | true)  SMALLCHANGE_STATIC=true;;
*)           SMALLCHANGE_STATIC=false;;
esac

AC_MSG_CHECKING([for strcasecmp() function])
AC_TRY_LINK([
#include <string.h>
], [
  (void)strcasecmp(NULL, NULL);
], [
  AC_MSG_RESULT([yes])
  AC_DEFINE([HAVE_STRCASECMP],, [Define to use BSD4.3 strncasecmp()])
], [
  AC_MSG_RESULT([no])
])


AC_MSG_CHECKING([for strcmpi() function])
AC_TRY_LINK([
#include <string.h>
], [
  (void)strcmpi(NULL, NULL);
], [
  AC_MSG_RESULT([yes])
  AC_DEFINE([HAVE_STRCMPI],, [Define to use strcmpi()])
], [
  AC_MSG_RESULT([no])
])

SIM_AC_COMPILE_DEBUG([
#  CPPFLAGS="$CPPFLAGS"
], [
#  CPPFLAGS="$CPPFLAGS -DNDEBUG"
])

if test x"${with_inventor+set}" = x"set" &&
   test x"${with_inventor}" != x"no"; then
  SIM_AC_HAVE_INVENTOR_IFELSE([
    CPPFLAGS="$sim_ac_inventor_cppflags $CPPFLAGS"
    LDFLAGS="$sim_ac_inventor_ldflags $LDFLAGS"
    LIBS="$sim_ac_inventor_libs $LIBS"
  ], [AC_MSG_ERROR(couldn't compile and link against Open Inventor)])
  SIM_AC_CONFIGURATION_SETTING([Open Inventor brand], [SGI/TGS Inventor])
else
  SIM_AC_HAVE_COIN_IFELSE([
    CPPFLAGS="$CPPFLAGS $sim_ac_coin_cppflags"
    LDFLAGS="$LDFLAGS $sim_ac_coin_ldflags"
    LIBS="$sim_ac_coin_libs $LIBS"
  ], [AC_MSG_ERROR(couldn't compile and link against Coin)])
  SIM_AC_CONFIGURATION_SETTING([Open Inventor brand], [Coin])
fi

AC_SUBST([LIBFLAGS], [])

if $BUILD_WITH_MSVC; then
  rm -f vc60.pdb
  LIBFLAGS="$LIBFLAGS $sim_ac_msvcrt_LIBLDFLAGS"
  LIBS="$LIBS $sim_ac_msvcrt_LIBLIBS"
  if $SMALLCHANGE_STATIC; then :; else
    CPPFLAGS="-DSMALLCHANGE_MAKE_DLL $CPPFLAGS"
    SMALLCHANGE_EXTRA_CPPFLAGS="$SMALLCHANGE_EXTRA_CPPFLAGS -DSMALLCHANGE_DLL"
  fi
  debugfile="`pwd`/lib/SmallChange/SmallChange${SC_MAJOR}.pdb"
  debugfile=`cygpath -w "$debugfile" | sed 's,\\\\,\\\\\\\\,g'`
  if $SMALLCHANGE_STATIC; then
    SIM_AC_CONFIGURATION_SETTING([SmallChange build type], [static .lib])
    LIBFLAGS="-LIB $LIBFLAGS"
    LIBFLAGS="$LIBFLAGS /OUT:SmallChange$SC_MAJOR.lib"
    SMALLCHANGE_EXTRA_CPPFLAGS="-I$includedir"
    SMALLCHANGE_EXTRA_LDFLAGS="-L$libdir"
    SMALLCHANGE_EXTRA_LIBS=
  else
    LIBFLAGS="-DLL /INCREMENTAL:NO $LIBFLAGS"
    case $enable_symbols in
    no | false)
      SIM_AC_CONFIGURATION_SETTING([SmallChange build type],
                                   [dynamic .dll, no symbols])
      LIBFLAGS="$LIBFLAGS /RELEASE"
      ;;
    *)
      SIM_AC_CONFIGURATION_SETTING([SmallChange build type],
                                   [dynamic .dll + .pdb])
      CFLAGS="/Fd$debugfile $CFLAGS"
      CXXFLAGS="/Fd$debugfile $CXXFLAGS"
      LIBFLAGS="$LIBFLAGS /DEBUG /PDB:SmallChange$SMALLCHANGE_MAJOR_VERSION.pdb"
      ;;
    esac

    LIBFLAGS="$LIBFLAGS /OUT:SmallChange$SC_MAJOR.dll"
  fi
  SIM_AC_CONFIGURATION_SETTING([C library type], [$sim_ac_msvcrt])

  SMALLCHANGE_EXTRA_LIBS="-lSmallChange$SC_MAJOR $SMALLCHANGE_EXTRA_LIBS"
else
  SMALLCHANGE_EXTRA_LIBS="-lSmallChange $SMALLCHANGE_EXTRA_LIBS"
fi


SIM_AC_UNIQIFY_LIST(CPPFLAGS, $CPPFLAGS)
SIM_AC_UNIQIFY_LIST(LDFLAGS, $LDFLAGS)
SIM_AC_UNIQIFY_LIST(LIBS, $LIBS)

AC_OUTPUT([
        Makefile
        lib/Makefile
        lib/SmallChange/Makefile
	lib/SmallChange/actions/Makefile
	lib/SmallChange/misc/Makefile
	lib/SmallChange/nodekits/Makefile
	lib/SmallChange/nodes/Makefile
	lib/SmallChange/elements/Makefile
	lib/SmallChange/engines/Makefile
	lib/SmallChange/draggers/Makefile

	lib/SmallChange/basic.h
])

SIM_AC_CONFIGURATION_SETTING([Install prefix], [$prefix])

SIM_AC_CONFIGURATION_SUMMARY

echo ""
echo "Now, run 'make' and 'make install' to install $PACKAGE"
echo ""
