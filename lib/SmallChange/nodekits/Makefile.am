INCLUDES = -I$(top_srcdir)/lib -I$(top_builddir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = nodekits.lst
else
noinst_LTLIBRARIES = libnodekits.la
endif

libnodekits_la_SOURCES = \
	LegendKit.cpp LegendKit.h \
	SoFEMKit.cpp SoFEMKit.h

nodekits_lst_SOURCES = \
	LegendKit.cpp LegendKit.h \
	SoFEMKit.cpp SoFEMKit.h

libnodekitsincdir = $(includedir)/SmallChange/nodekits
libnodekitsinc_HEADERS = \
	$(top_srcdir)/lib/SmallChange/nodekits/LegendKit.h \
	$(top_srcdir)/lib/SmallChange/nodekits/SoFEMKit.h

nodekits.lst: $(nodekits_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(nodekits_lst_OBJECTS); do echo $$i >>$@; done

