
INCLUDES = -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = nodekits.lst
else
noinst_LTLIBRARIES = libnodekits.la
endif

libnodekits_la_SOURCES = \
	LegendKit.cpp LegendKit.h

nodekits_lst_SOURCES = \
	LegendKit.cpp LegendKit.h

libnodekitsincdir = $(includedir)/SmallChange/nodekits
libnodekitsinc_HEADERS = \
	$(top_srcdir)/lib/SmallChange/nodekits/LegendKit.h

nodekits.lst: $(nodekits_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(nodekits_lst_OBJECTS); do echo $$i >>$@; done
