INCLUDES = -I$(top_srcdir)/lib -I$(top_builddir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = engines.lst
else
noinst_LTLIBRARIES = libengines.la
endif

libengines_la_SOURCES = \
	Rot2Heading.cpp Rot2Heading.h \
	CubicSplineEngine.cpp CubicSplineEngine.h \
	SmInverseRotation.cpp SmInverseRotation.h

engines_lst_SOURCES = \
	Rot2Heading.cpp Rot2Heading.h \
	CubicSplineEngine.cpp CubicSplineEngine.h \
	SmInverseRotation.cpp SmInverseRotation.h

libenginesincdir = $(includedir)/SmallChange/engines
libenginesinc_HEADERS = \
	$(top_srcdir)/lib/SmallChange/engines/Rot2Heading.h \
	$(top_srcdir)/lib/SmallChange/engines/CubicSplineEngine.h \
	$(top_srcdir)/lib/SmallChange/engines/SmInverseRotation.h

engines.lst: $(engines_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(engines_lst_OBJECTS); do echo $$i >>$@; done

