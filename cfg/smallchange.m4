############################################################################
# Usage:
#   SIM_AC_HAVE_SMALLCHANGE_IFELSE( IF-FOUND, IF-NOT-FOUND )
#
# Description:
#   This macro locates the SmallChange development system.  If it is found,
#   the set of variables listed below are set up as described and made
#   available to the configure script.
#
#   The $sim_ac_smallchange_desired variable can be set to false externally to
#   make SmallChange default to be excluded.
#
# Autoconf Variables:
# > $sim_ac_smallchange_desired     true | false (defaults to false)
# < $sim_ac_smallchange_avail       true | false
# < $sim_ac_smallchange_cppflags    (extra flags the preprocessor needs)
# < $sim_ac_smallchange_cflags      (extra flags the C compiler needs)
# < $sim_ac_smallchange_cxxflags    (extra flags the C++ compiler needs)
# < $sim_ac_smallchange_ldflags     (extra flags the linker needs)
# < $sim_ac_smallchange_libs        (link library flags the linker needs)
# < $sim_ac_smallchange_datadir     (location of SmallChange data files)
# < $sim_ac_smallchange_includedir  (location of SmallChange headers)
# < $sim_ac_smallchange_version     (the libSmallChange version)
# < $sim_ac_smallchange_msvcrt      (the MSVC++ C library SmallChange was built with)
# < $sim_ac_smallchange_configcmd   (the path to smallchange-config or "false")
#
# Authors:
#   Lars J. Aas, <larsa@sim.no>
#   Morten Eriksen, <mortene@sim.no>
#
# TODO:
#

AC_DEFUN([SIM_AC_HAVE_SMALLCHANGE_IFELSE], [
AC_PREREQ([2.14a])

# official variables
sim_ac_smallchange_avail=false
sim_ac_smallchange_cppflags=
sim_ac_smallchange_cflags=
sim_ac_smallchange_cxxflags=
sim_ac_smallchange_ldflags=
sim_ac_smallchange_libs=
sim_ac_smallchange_datadir=
sim_ac_smallchange_includedir=
sim_ac_smallchange_version=

# internal variables
: ${sim_ac_smallchange_desired=true}
sim_ac_smallchange_extrapath=

AC_ARG_WITH([smallchange],
AC_HELP_STRING([--with-smallchange], [enable use of SmallChange [[default=no]]])
AC_HELP_STRING([--with-smallchange=DIR], [give prefix location of SmallChange]),
  [ case $withval in
    no)  sim_ac_smallchange_desired=false ;;
    yes) sim_ac_smallchange_desired=true ;;
    *)   sim_ac_smallchange_desired=false
         sim_ac_smallchange_extrapath=$withval ;;
    esac],
  [])

case $build in
*-mks ) sim_ac_pathsep=";" ;;
* )     sim_ac_pathsep="${PATH_SEPARATOR}" ;;
esac

if $sim_ac_smallchange_desired; then
  sim_ac_path=$PATH
  test -z "$sim_ac_smallchange_extrapath" || ## search in --with-smallchange path
    sim_ac_path="$sim_ac_smallchange_extrapath/bin${sim_ac_pathsep}$sim_ac_path"
  test x"$prefix" = xNONE ||          ## search in --prefix path
    sim_ac_path="$sim_ac_path${sim_ac_pathsep}$prefix/bin"

  AC_PATH_PROG(sim_ac_smallchange_configcmd, smallchange-config, false, $sim_ac_path)

  if test "X$sim_ac_smallchange_configcmd" = "Xfalse"; then :; else
    test -n "$CONFIG" &&
      $sim_ac_smallchange_configcmd --alternate=$CONFIG >/dev/null 2>/dev/null &&
      sim_ac_smallchange_configcmd="$sim_ac_smallchange_configcmd --alternate=$CONFIG"
  fi

  if $sim_ac_smallchange_configcmd; then
    sim_ac_smallchange_version=`$sim_ac_smallchange_configcmd --version`
    sim_ac_smallchange_cppflags=`$sim_ac_smallchange_configcmd --cppflags`
    sim_ac_smallchange_cflags=`$sim_ac_smallchange_configcmd --cflags 2>/dev/null`
    sim_ac_smallchange_cxxflags=`$sim_ac_smallchange_configcmd --cxxflags`
    sim_ac_smallchange_ldflags=`$sim_ac_smallchange_configcmd --ldflags`
    sim_ac_smallchange_libs=`$sim_ac_smallchange_configcmd --libs`
    sim_ac_smallchange_datadir=`$sim_ac_smallchange_configcmd --datadir`
    # Hide stderr on the following, as ``--includedir'', ``--msvcrt''
    # and ``--cflags'' options were added late to smallchange-config.
    sim_ac_smallchange_includedir=`$sim_ac_smallchange_configcmd --includedir 2>/dev/null`
    sim_ac_smallchange_msvcrt=`$sim_ac_smallchange_configcmd --msvcrt 2>/dev/null`
    sim_ac_smallchange_cflags=`$sim_ac_smallchange_configcmd --cflags 2>/dev/null`
    AC_CACHE_CHECK(
      [whether libSmallChange is available],
      sim_cv_smallchange_avail,
      [sim_ac_save_cppflags=$CPPFLAGS
      sim_ac_save_ldflags=$LDFLAGS
      sim_ac_save_libs=$LIBS
      CPPFLAGS="$CPPFLAGS $sim_ac_smallchange_cppflags"
      LDFLAGS="$LDFLAGS $sim_ac_smallchange_ldflags"
      LIBS="$sim_ac_smallchange_libs $LIBS"
      AC_LANG_PUSH(C++)
      AC_TRY_LINK(
        [#include <SmallChange/misc/Init.h>],
        [smallchange_init();],
        [sim_cv_smallchange_avail=true],
        [sim_cv_smallchange_avail=false])
      AC_LANG_POP
      CPPFLAGS=$sim_ac_save_cppflags
      LDFLAGS=$sim_ac_save_ldflags
      LIBS=$sim_ac_save_libs
    ])
    sim_ac_smallchange_avail=$sim_cv_smallchange_avail
  else
    locations=`IFS="${sim_ac_pathsep}"; for p in $sim_ac_path; do echo " -> $p/smallchange-config"; done`
    AC_MSG_WARN([cannot find 'smallchange-config' at any of these locations:
$locations])
  fi
fi

if $sim_ac_smallchange_avail; then
  ifelse([$1], , :, [$1])
else
  ifelse([$2], , :, [$2])
fi
]) # SIM_AC_HAVE_SMALLCHANGE_IFELSE()

