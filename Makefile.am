SUBDIRS = bin lib

## *******************************************************************

BUILT_SOURCES = \
	smallchange-@CONFIG@.cfg

CLEANFILES = \
	smallchange1.dsp \
	smallchange1.dsw \
	smallchange1.ncb \
	smallchange1.opt \
	smallchange1.plg \
	smallchange1.dll \
	smallchange1.ilk \
	smallchange1d.dll \
	smallchange1d.ilk

DISTCLEANFILES = \
	smallchange-@CONFIG@.cfg

m4datadir = $(datadir)/aclocal
dist_m4data_DATA = cfg/smallchange.m4

cfgdatadir = $(datadir)/Coin/conf
cfgdata_DATA = smallchange-@CONFIG@.cfg

EXTRA_DIST = \
	smallchange.cfg.in \
	cfg/gendsp.sh.in \
	cfg/gendsp.pl.in \
	build/misc/generate.sh \
	build/misc/install-sdk.bat \
	build/misc/create-directories.bat \
	build/misc/config-wrapper.h \
	build/msvc6/install-headers.bat
	build/msvc6/smallchange1.dsp \
	build/msvc6/smallchange1.dsw \
	build/msvc6/lib/config-debug.h \
	build/msvc6/lib/config-release.h \
	build/msvc6/lib/config.h \
	build/msvc7/install-headers.bat
	build/msvc7/smallchange1.vcproj \
	build/msvc7/smallchange1.sln \
	build/msvc7/lib/config-debug.h \
	build/msvc7/lib/config-release.h \
	build/msvc7/lib/config.h \
	build/msvc8/install-headers.bat
	build/msvc8/smallchange1.vcproj \
	build/msvc8/smallchange1.sln \
	build/msvc8/lib/config-debug.h \
	build/msvc8/lib/config-release.h \
	build/msvc8/lib/config.h

smallchange-@CONFIG@.cfg: config.status $(srcdir)/smallchange.cfg.in
	@$(SHELL) config.status --file=$@:smallchange.cfg.in
