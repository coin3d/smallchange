INCLUDES = -I$(top_srcdir)/lib -I$(top_builddir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = nodekits.lst
else
noinst_LTLIBRARIES = libnodekits.la
endif

SOURCES = \
	LegendKit.cpp LegendKit.h \
	SoFEMKit.cpp SoFEMKit.h \
	SmTooltipKit.cpp SmTooltipKit.h \
	SmWellLogKit.cpp SmWellLogKit.h \
	SmCameraControlKit.cpp SmCameraControlKit.h

libnodekits_la_SOURCES = $(SOURCES)

nodekits_lst_SOURCES = $(SOURCES)

libnodekitsincdir = $(includedir)/SmallChange/nodekits
libnodekitsinc_HEADERS = \
	$(top_srcdir)/lib/SmallChange/nodekits/LegendKit.h \
	$(top_srcdir)/lib/SmallChange/nodekits/SoFEMKit.h \
	$(top_srcdir)/lib/SmallChange/nodekits/SmTooltipKit.h \
	$(top_srcdir)/lib/SmallChange/nodekits/SmWellLogKit.h \
	$(top_srcdir)/lib/SmallChange/nodekits/SmCameraControlKit.h

nodekits.lst: $(nodekits_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(nodekits_lst_OBJECTS); do echo $$i >>$@; done

