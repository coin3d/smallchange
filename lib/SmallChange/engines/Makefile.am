
INCLUDES = -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = engines.lst
else
noinst_LTLIBRARIES = libengines.la
endif

libengines_la_SOURCES = \
	Rot2Heading.cpp Rot2Heading.h

engines_lst_SOURCES = \
	Rot2Heading.cpp Rot2Heading.h

libenginesincdir = $(includedir)/SmallChange/engines
libenginesinc_HEADERS = \
	$(top_srcdir)/lib/SmallChange/engines/Rot2Heading.h

engines.lst: $(engines_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(engines_lst_OBJECTS); do echo $$i >>$@; done

