INCLUDES = -I$(top_srcdir)/lib -I$(top_builddir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = elements.lst
else
noinst_LTLIBRARIES = libelements.la
endif

libelements_la_SOURCES = \
	GLDepthBufferElement.cpp GLDepthBufferElement.h \
	SmSceneryViewElement.cpp SmSceneryViewElement.h \
	UTMElement.cpp UTMElement.h

elements_lst_SOURCES = \
	GLDepthBufferElement.cpp GLDepthBufferElement.h \
	SmSceneryViewElement.cpp SmSceneryViewElement.h \
	UTMElement.cpp UTMElement.h

libelementsincdir = $(includedir)/SmallChange/elements
libelementsinc_HEADERS = \
	$(top_srcdir)/lib/SmallChange/elements/UTMElement.h \
	$(top_srcdir)/lib/SmallChange/elements/SmSceneryViewElement.h \
	$(top_srcdir)/lib/SmallChange/elements/GLDepthBufferElement.h

elements.lst: $(elements_lst_OBJECTS)
	@echo "Linking $@..." \
	rm -f $@; \
	for i in $(elements_lst_OBJECTS); do echo $$i >>$@; done

