SUBDIRS = nodekits nodes elements .

INCLUDES = -I$(top_srcdir)/src

lib_LTLIBRARIES = libsmallchange.la

libsmallchange_la_SOURCES =

libsmallchange_la_LIBADD = \
	nodekits/libnodekits.la nodes/libnodes.la elements/libelements.la

libsmallchange_la_LDFLAGS = \
	-no-undefined \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

