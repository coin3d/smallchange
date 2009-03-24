m4_define([SMALLCHANGE_MAJOR], [1])
m4_define([SMALLCHANGE_MINOR], [0])
m4_define([SMALLCHANGE_MICRO], [0])
m4_define([SMALLCHANGE_BETA], [a])

# this is bogus, but policy has not been set for this yet
m4_define([SMALLCHANGE_ABI_CURRENT], [m4_eval((SMALLCHANGE_MAJOR*20)+SMALLCHANGE_MINOR)])
m4_define([SMALLCHANGE_ABI_REVISION], [SMALLCHANGE_MICRO])
m4_define([SMALLCHANGE_ABI_AGE], [SMALLCHANGE_MINOR])

AC_INIT([SmallChange], [SMALLCHANGE_MAJOR.SMALLCHANGE_MINOR.SMALLCHANGE_MICRO[]SMALLCHANGE_BETA], [coin-bugs@coin3d.org])

AC_CONFIG_SRCDIR(lib/SmallChange/nodes/SmDepthBuffer.cpp)
AC_CONFIG_AUX_DIR(cfg)

AC_CANONICAL_SYSTEM

SIM_AC_SETUP_MKS

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
smallchange_src_dir=`cd $srcdir; pwd`

AC_SUBST(smallchange_build_dir)
AC_SUBST(smallchange_src_dir)

SIM_AC_RELATIVE_SRC_DIR

# **************************************************************************
#  Locate C++ compiler and set C++ as the default language to use
#  in tests. The configure script will automatically terminate if
#  it doesn't find a C++ compiler.

# cross_compiling: On MKS, configure will open a window when trying to run
# the executable a.out in the cross-compilation test because it is not
# named *.exe, halting the configure script until it has been closed, even
# resulting in a faulty conclusion for the test (cross_compiling=yes).
# By setting the $cross_compiling variable to yes in advance, this test is
# skipped.  We are of course not cross-compiling, so we correct the variable
# afterwards.  2003-06-25 larsa

case $build in
*-mks ) cross_compiling=yes ;;
esac

AC_PROG_CC

case $build in
*-mks ) cross_compiling=no ;;
esac

AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_LANG_CPLUSPLUS

SIM_AC_STRIP_EXIT_DECLARATION

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
# Universal Binary support (Mac OS X)

SIM_AC_UNIVERSAL_BINARIES

# **************************************************************************

AM_INIT_AUTOMAKE(SmallChange, $VERSION)

#  Files to cram results from the configure tests into.
AM_CONFIG_HEADER(config.h)

#  Default to not building a shared library.
#  Can be overridden by the user with --enable-shared.
# AM_DISABLE_SHARED

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

SIM_AC_COMPILE_DEBUG([
  CPPFLAGS="$CPPFLAGS -DSMALLCHANGE_DEBUG=1"
], [
  CPPFLAGS="$CPPFLAGS -DSMALLCHANGE_DEBUG=0"
])

SIM_AC_DEBUGSYMBOLS
SIM_AC_RTTI_SUPPORT
SIM_AC_EXCEPTION_HANDLING
SIM_AC_PROFILING_SUPPORT
SIM_AC_COMPILER_WARNINGS
SIM_AC_DETECT_COMMON_COMPILER_FLAGS

CPPFLAGS="$CPPFLAGS -DSMALLCHANGE_INTERNAL"

# **************************************************************************
# Figure out if library should be built static or dynamic.

case ${enable_static-no} in
yes | true) SMALLCHANGE_STATIC=true
            enable_shared=no ;; # fix --enable-static bug? in autoconf 2.58
*)          SMALLCHANGE_STATIC=false ;;
esac

case ${enable_shared-default} in
default)    ;;
yes | true) SMALLCHANGE_STATIC=false ;;
*)          SMALLCHANGE_STATIC=true ;;
esac

# **************************************************************************
# Configure an alternate installation?
# **************************************************************************

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

# **************************************************************************
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

sim_ac_configure_save_ldflags=$LDFLAGS
sim_ac_configure_save_libs=$LIBS

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

# **************************************************************************

CFLAGS="$CFLAGS $sim_ac_compiler_CFLAGS"
CXXFLAGS="$CXXFLAGS $sim_ac_compiler_CXXFLAGS"

# **************************************************************************

AC_SUBST([LIBFLAGS], [])

SIM_AC_CHECK_OPENGL(
  [
  AC_DEFINE([HAVE_OPENGL],1, [define if you have OpenGL])
  COIN_EXTRA_CPPFLAGS="$COIN_EXTRA_CPPFLAGS $sim_ac_ogl_cppflags"
  COIN_EXTRA_LDFLAGS="$COIN_EXTRA_LDFLAGS $sim_ac_ogl_ldflags"
  COIN_EXTRA_LIBS="$sim_ac_ogl_libs $COIN_EXTRA_LIBS"
  ],
  [ # Couldn't link with OpenGL:
    SIM_AC_ERROR([no-opengl])
  ]
)

# **************************************************************************

if $SMALLCHANGE_STATIC && $BUILD_WITH_MSVC; then
  LDFLAGS=$sim_ac_configure_save_ldflags
  LIBS=$sim_ac_configure_save_libs
fi

# **************************************************************************

# check if compiler/linker handles __declspec()
sim_ac_win32_declspec_available=false
AC_MSG_CHECKING([for __declspec() support])
AC_TRY_LINK(
 [],
 [__declspec(dllimport) int foo;],
 [sim_ac_win32_declspec_available=true
  AC_MSG_RESULT([available])],
 [AC_MSG_RESULT([not available])])

if $sim_ac_win32_declspec_available; then
  if $SMALLCHANGE_STATIC; then
    SMALLCHANGE_EXTRA_CPPFLAGS="$SMALLCHANGE_EXTRA_CPPFLAGS -DSMALLCHANGE_NOT_DLL"
  else
    CPPFLAGS="-DSMALLCHANGE_MAKE_DLL $CPPFLAGS"
    SMALLCHANGE_EXTRA_CPPFLAGS="$SMALLCHANGE_EXTRA_CPPFLAGS -DSMALLCHANGE_DLL"
  fi
fi

if $BUILD_WITH_MSVC; then
  rm -f vc60.pdb
  LIBFLAGS="$LIBFLAGS $sim_ac_msvcrt_LIBLDFLAGS"
  LIBS="$LIBS $sim_ac_msvcrt_LIBLIBS"
  debugfile="`pwd`/lib/SmallChange/SmallChange${SMALLCHANGE_MAJOR_VERSION}${SUFFIX}.pdb"
  debugfile=`cygpath -w "$debugfile" | sed 's,\\\\,\\\\\\\\,g'`
  if $SMALLCHANGE_STATIC; then
    SIM_AC_CONFIGURATION_SETTING([SmallChange build type], [static .lib])
    LIBFLAGS="-LIB $LIBFLAGS"
    LIBFLAGS="$LIBFLAGS /OUT:SmallChange${SMALLCHANGE_MAJOR_VERSION}${SUFFIX}.lib"
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
      CFLAGS="/Fd$debugfile -g $CFLAGS"
      CXXFLAGS="/Fd$debugfile -g $CXXFLAGS"
      LIBFLAGS="$LIBFLAGS /DEBUG /PDB:SmallChange${SMALLCHANGE_MAJOR_VERSION}${SUFFIX}.pdb"
      ;;
    esac

    LIBFLAGS="$LIBFLAGS /OUT:SmallChange${SMALLCHANGE_MAJOR_VERSION}${SUFFIX}.dll"
  fi
  SIM_AC_CONFIGURATION_SETTING([C library type], [$sim_ac_msvcrt])

  SMALLCHANGE_EXTRA_LIBS="-lSmallChange${SMALLCHANGE_MAJOR_VERSION}${SUFFIX} $SMALLCHANGE_EXTRA_LIBS"
else
  SMALLCHANGE_EXTRA_LIBS="-lSmallChange${SUFFIX} $SMALLCHANGE_EXTRA_LIBS"
fi

# **************************************************************************

SMALLCHANGE_COMPILER="$CXX"

# CXX will point to wrapmsvc in the source code hierarchy
if $BUILD_WITH_MSVC; then
  SMALLCHANGE_COMPILER="wrapmsvc"
fi

# Must call gcc as g++ when compiling C++ on Mac OS X to ensure
# linking against libstdc++ if necessary.
case "$host_os" in
  darwin*)
    if test x"$CC" = xgcc; then
      CC=g++
    fi
    ;;
esac
        
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

SIM_AC_MSVC_DSP_SETUP([SMALLCHANGE], [SmallChange], [smallchange], [-I$smallchange_build_dir/lib -I$smallchange_src_dir/lib -I$smallchange_build_dir])

# **************************************************************************

AC_CONFIG_FILES([
        SmallChange.pc
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
        cfg/gendsp.sh
        cfg/gendsp.pl
])

# MKS will have "\c" as $ECHO_C, but that string in the config.status
# substitution command will cause config.status to hang because it is
# somehow translated to "&c".  We therefore blank it here, and restore
# it further below, after AC_OUTPUT.

sim_ac_ECHO_C=$ECHO_C
case $build in
*-mks ) ECHO_C="" ;;
esac

AC_OUTPUT

ECHO_C=$sim_ac_ECHO_C

if test -f cfg/gendsp.sh; then
  chmod +x cfg/gendsp.sh
fi
if test -f cfg/gendsp.pl; then
  chmod +x cfg/gendsp.pl
fi

# **************************************************************************

case "$sim_ac_framework:$sim_ac_make_dsp" in
true:* | *:true )
  ;;
* )
  SIM_AC_CONFIGURATION_SETTING([Installation prefix], [$prefix])
  ;;
esac

SIM_AC_CONFIGURATION_SUMMARY

echo ""
echo "Now, run 'make' and 'make install' to install $PACKAGE"
echo ""

