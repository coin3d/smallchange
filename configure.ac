m4_define([SMALLCHANGE_MAJOR], [1])
m4_define([SMALLCHANGE_MINOR], [0])
m4_define([SMALLCHANGE_MICRO], [0])
m4_define([SMALLCHANGE_BETA], [a])

# this is bogus, but policy has not been set for this yet
m4_define([SMALLCHANGE_ABI_CURRENT], [m4_eval((SMALLCHANGE_MAJOR*20)+SMALLCHANGE_MINOR)])
m4_define([SMALLCHANGE_ABI_REVISION], [SMALLCHANGE_MICRO])
m4_define([SMALLCHANGE_ABI_AGE], [SMALLCHANGE_MINOR])

AC_INIT([SmallChange], [SMALLCHANGE_MAJOR.SMALLCHANGE_MINOR.SMALLCHANGE_MICRO[]SMALLCHANGE_BETA], [coin-bugs@coin3d.org])

AC_CONFIG_SRCDIR(lib/SmallChange/nodes/DepthBuffer.cpp)
AC_CONFIG_AUX_DIR(cfg)

AC_CANONICAL_SYSTEM


# *******************************************************************
# If the Microsoft Visual C++ cl.exe compiler is available, set us up for
# compiling with it and to generate an MSWindows .dll file.

SIM_AC_SETUP_MSVCPP_IFELSE

SIM_AC_MSVC_DSP_ENABLE_OPTION


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
AC_SUBST([SMALLCHANGE_BETA_VERSION], [SMALLCHANGE_BETA])
AC_SUBST([SMALLCHANGE_VERSION], [$SMALLCHANGE_MAJOR_VERSION.$SMALLCHANGE_MINOR_VERSION.$SMALLCHANGE_MICRO_VERSION$SMALLCHANGE_BETA_VERSION])

VERSION=$SMALLCHANGE_VERSION

# Libtool versioning
AC_SUBST([LT_CURRENT], [SMALLCHANGE_ABI_CURRENT])
AC_SUBST([LT_REVISION], [SMALLCHANGE_ABI_REVISION])
AC_SUBST([LT_AGE], [SMALLCHANGE_ABI_AGE])

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
AC_PROG_CXXCPP
AC_LANG_CPLUSPLUS

# **************************************************************************
#  These are used for constructing the smallchange-config file.

SMALLCHANGE_EXTRA_CFLAGS=
SMALLCHANGE_EXTRA_CPPFLAGS=
SMALLCHANGE_EXTRA_CXXFLAGS=
SMALLCHANGE_EXTRA_LDFLAGS=
SMALLCHANGE_EXTRA_LIBS=

AC_SUBST([SMALLCHANGE_EXTRA_CPPFLAGS], [$CPPFLAGS])
AC_SUBST([SMALLCHANGE_EXTRA_CFLAGS], [])
AC_SUBST([SMALLCHANGE_EXTRA_CXXFLAGS], [])
AC_SUBST([SMALLCHANGE_EXTRA_LDFLAGS], [$LDFLAGS])
AC_SUBST([SMALLCHANGE_EXTRA_LIBS], [$LIBS])

# **************************************************************************

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

# *******************************************************************
# Configure an alternate installation?

AC_ARG_WITH(
  [alternate],
  [AC_HELP_STRING([--with-alternate=string],
                  [specify name of alternate configuration])],
  [CONFIG=$withval],
  [CONFIG=default])

AC_ARG_WITH(
  [suffix],
  [AC_HELP_STRING([--with-suffix=string],
                  [specify library name suffix (e.g. "_g")])],
  [SUFFIX=$withval],
  [SUFFIX=])

AC_SUBST(CONFIG)
AC_SUBST(SUFFIX)

# *******************************************************************
# System tests.

# The last argument consists of a space character, because the macro
# wants a non-empty argument to check for compilability instead of
# just presence.
AC_CHECK_HEADERS(
  [windows.h unistd.h ctype.h sys/stat.h sys/types.h dirent.h],
  [], [], 
  [ ])

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
    SMALLCHANGE_EXTRA_CPPFLAGS="$sim_ac_inventor_cppflags $SMALLCHANGE_EXTRA_CPPFLAGS"
    SMALLCHANGE_EXTRA_LDFLAGS="$sim_ac_inventor_ldflags $SMALLCHANGE_EXTRA_LDFLAGS"
    SMALLCHANGE_EXTRA_LIBS="$sim_ac_inventor_libs $SMALLCHANGE_EXTRA_LIBS"
  ], [AC_MSG_ERROR(couldn't compile and link against Open Inventor)])
  SIM_AC_CONFIGURATION_SETTING([Open Inventor brand], [SGI/TGS Inventor])
else
  SIM_AC_HAVE_COIN_IFELSE([
    CPPFLAGS="$CPPFLAGS $sim_ac_coin_cppflags"
    LDFLAGS="$LDFLAGS $sim_ac_coin_ldflags"
    LIBS="$sim_ac_coin_libs $LIBS"
    SMALLCHANGE_EXTRA_CPPFLAGS="$SMALLCHANGE_EXTRA_CPPFLAGS $sim_ac_coin_cppflags"
    SMALLCHANGE_EXTRA_LDFLAGS="$SMALLCHANGE_EXTRA_LDFLAGS $sim_ac_coin_ldflags"
    SMALLCHANGE_EXTRA_LIBS="$sim_ac_coin_libs $SMALLCHANGE_EXTRA_LIBS"
  ], [AC_MSG_ERROR(couldn't compile and link against Coin)])
  SIM_AC_CONFIGURATION_SETTING([Open Inventor brand], [Coin])
fi

# *******************************************************************

# SIM_AC_HAVE_LIBSCENERY_IFELSE([
#   AC_DEFINE([HAVE_LIBSCENERY], 1, [define if you have the SIM Scenery lib])
#   CPPFLAGS="$CPPFLAGS $sim_ac_libscenery_cppflags"
#   LDFLAGS="$LDFLAGS $sim_ac_libscenery_ldflags"
#   LIBS="$sim_ac_libscenery_libs $LIBS"
#   SMALLCHANGE_EXTRA_CPPFLAGS="$SMALLCHANGE_EXTRA_CPPFLAGS $sim_ac_libscenery_cppflags"
#   SMALLCHANGE_EXTRA_LDFLAGS="$SMALLCHANGE_EXTRA_LDFLAGS $sim_ac_libscenery_ldflags"
#   SMALLCHANGE_EXTRA_LIBS="$sim_ac_libscenery_libs $SMALLCHANGE_EXTRA_LIBS"
# ], [])

# *******************************************************************

AC_SUBST([LIBFLAGS], [])

if $BUILD_WITH_MSVC; then
  rm -f vc60.pdb
  LIBFLAGS="$LIBFLAGS $sim_ac_msvcrt_LIBLDFLAGS"
  LIBS="$LIBS $sim_ac_msvcrt_LIBLIBS"
  if $SMALLCHANGE_STATIC; then
    SMALLCHANGE_EXTRA_CPPFLAGS="$SMALLCHANGE_EXTRA_CPPFLAGS -DSMALLCHANGE_NOT_DLL"
  else
    CPPFLAGS="-DSMALLCHANGE_MAKE_DLL $CPPFLAGS"
    SMALLCHANGE_EXTRA_CPPFLAGS="$SMALLCHANGE_EXTRA_CPPFLAGS -DSMALLCHANGE_DLL"
  fi
  debugfile="`pwd`/lib/SmallChange/SmallChange${SMALLCHANGE_MAJOR_VERSION}.pdb"
  debugfile=`cygpath -w "$debugfile" | sed 's,\\\\,\\\\\\\\,g'`
  if $SMALLCHANGE_STATIC; then
    SIM_AC_CONFIGURATION_SETTING([SmallChange build type], [static .lib])
    LIBFLAGS="-LIB $LIBFLAGS"
    LIBFLAGS="$LIBFLAGS /OUT:SmallChange$SMALLCHANGE_MAJOR_VERSION.lib"
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

    LIBFLAGS="$LIBFLAGS /OUT:SmallChange$SMALLCHANGE_MAJOR_VERSION.dll"
  fi
  SIM_AC_CONFIGURATION_SETTING([C library type], [$sim_ac_msvcrt])

  SMALLCHANGE_EXTRA_LIBS="-lSmallChange$SMALLCHANGE_MAJOR_VERSION $SMALLCHANGE_EXTRA_LIBS"
else
  SMALLCHANGE_EXTRA_LIBS="-lSmallChange $SMALLCHANGE_EXTRA_LIBS"
fi

# **************************************************************************

SMALLCHANGE_COMPILER="$CXX"

# CXX will point to wrapmsvc in the source code hierarchy
if $BUILD_WITH_MSVC; then
  SMALLCHANGE_COMPILER="wrapmsvc"
fi

AC_SUBST([SMALLCHANGE_COMPILER])
AC_SUBST([ac_compiler_gnu])

# **************************************************************************
# Remove redundant options from certain option lists.

SIM_AC_UNIQIFY_OPTION_LIST(SMALLCHANGE_EXTRA_CPPFLAGS, -I$includedir $SMALLCHANGE_EXTRA_CPPFLAGS)
SIM_AC_UNIQIFY_OPTION_LIST(SMALLCHANGE_EXTRA_CFLAGS, $SMALLCHANGE_EXTRA_CFLAGS)
SIM_AC_UNIQIFY_OPTION_LIST(SMALLCHANGE_EXTRA_CXXFLAGS, $SMALLCHANGE_EXTRA_CXXFLAGS)
SIM_AC_UNIQIFY_OPTION_LIST(SMALLCHANGE_EXTRA_LDFLAGS, -L$libdir $SMALLCHANGE_EXTRA_LDFLAGS)
SIM_AC_UNIQIFY_OPTION_LIST(SMALLCHANGE_EXTRA_LIBS, $SMALLCHANGE_EXTRA_LIBS)

SIM_AC_UNIQIFY_OPTION_LIST(CPPFLAGS, $CPPFLAGS)
SIM_AC_UNIQIFY_OPTION_LIST(LDFLAGS, $LDFLAGS)
SIM_AC_UNIQIFY_OPTION_LIST(LIBS, $LIBS)

# **************************************************************************

AC_OUTPUT([
        Makefile

        bin/Makefile

        lib/Makefile
        lib/SmallChange/Makefile
	lib/SmallChange/actions/Makefile
	lib/SmallChange/misc/Makefile
	lib/SmallChange/nodekits/Makefile
	lib/SmallChange/nodes/Makefile
	lib/SmallChange/elements/Makefile
	lib/SmallChange/engines/Makefile
	lib/SmallChange/draggers/Makefile
	lib/SmallChange/eventhandlers/Makefile

	lib/SmallChange/basic.h
])

SIM_AC_CONFIGURATION_SETTING([Install prefix], [$prefix])

SIM_AC_CONFIGURATION_SUMMARY

echo ""
echo "Now, run 'make' and 'make install' to install $PACKAGE"
echo ""
