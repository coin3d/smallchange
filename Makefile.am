SUBDIRS = bin lib

## *******************************************************************

BUILT_SOURCES = \
	smallchange-@CONFIG@.cfg

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
