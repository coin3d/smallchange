SUBDIRS = nodekits nodes elements .

INCLUDES = -I$(top_srcdir)/src

lib_LTLIBRARIES = libSmallChange.la

libSmallChange_la_SOURCES =

libSmallChange_la_LIBADD = \
	nodekits/libnodekits.la nodes/libnodes.la elements/libelements.la

libSmallChange_la_LDFLAGS = \
	-no-undefined \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

