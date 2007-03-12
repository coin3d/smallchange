#! /bin/sh
#
# This script generates the Visual Studio 6 build files for Windows.
#
# 20041214 larsa

projname=smallchange1

rm -f ${projname}.dsp ${projname}.dsw ${projname}.vcproj ${projname}.sln install-headers.bat

build_pwd=`pwd`
build="`cygpath -w $build_pwd | sed -e 's/\\\\/\\\\\\\\/g'`"
build_pwd="`pwd | sed -e 's/\\//\\\\\\\\/g'`\\\\"

source_pwd=`cd ../..; pwd`
source="`cygpath -w $source_pwd | sed -e 's/\\\\/\\\\\\\\/g'`"
source_pwd="`(cd ../..; pwd) | sed -e 's/\\//\\\\\\\\/g'`"

../../configure --with-msvcrt=mtd --with-suffix=d \
  --enable-debug --enable-symbols || exit 1
mv config.h lib/config-debug.h

../../configure --enable-msvcdsp --with-msvcrt=mt \
  --disable-debug --disable-symbols --enable-optimization || exit 1
mv config.h lib/config-release.h

cp ../misc/config-wrapper.h lib/config.h

# if test x"" != x"--use-msvc6"; then
make || exit 1

sed \
  -e "s/$build/./g" \
  -e "s/$build_pwd//g" \
  -e "s/$source/..\\\\../g" \
  -e "s/$source_pwd/..\\\\../g" \
  -e 's/$/\r/g' \
  <${projname}.dsp >new.dsp
mv new.dsp ${projname}.dsp

sed \
  -e "s/$build/./g" \
  -e "s/$build_pwd//g" \
  -e "s/$source/..\\\\../g" \
  -e "s/$source_pwd/..\\\\../g" \
  -e 's/$/\r/g' \
  <install-headers.bat >new.bat
mv new.bat install-headers.bat

# How can I avoid the modal upgrade prompt-dialog for MSVC7.1 here???
# devenv /command "File.OpenProject $build\\smallchange1.dsp"

