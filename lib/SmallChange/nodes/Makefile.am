
INCLUDES = -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = nodes.lst
else
noinst_LTLIBRARIES = libnodes.la
endif

libnodes_la_SOURCES = \
	DepthBuffer.cpp DepthBuffer.h \
	ViewportRegion.cpp ViewportRegion.h \
	Coinboard.cpp Coinboard.h

nodes_lst_SOURCES = \
	DepthBuffer.cpp DepthBuffer.h \
	ViewportRegion.cpp ViewportRegion.h \
	Coinboard.cpp Coinboard.h

libnodesincdir = $(includedir)/SmallChange/nodes
libnodesinc_HEADERS = \
	$(top_srcdir)/lib/SmallChange/nodes/ViewportRegion.h \
	$(top_srcdir)/lib/SmallChange/nodes/DepthBuffer.h \
	$(top_srcdir)/lib/SmallChange/nodes/Coinboard.h

nodes.lst: $(nodes_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(nodes_lst_OBJECTS); do echo $$i >>$@; done

