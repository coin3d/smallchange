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
	smallchange.cfg.in

smallchange-@CONFIG@.cfg: config.status $(srcdir)/smallchange.cfg.in
	@$(SHELL) config.status --file=$@:smallchange.cfg.in
