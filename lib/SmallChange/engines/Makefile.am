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
	Rot2Heading.h \
	CubicSplineEngine.h \
	SmInverseRotation.h

engines.lst: $(engines_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(engines_lst_OBJECTS); do echo $$i >>$@; done

install-libenginesincHEADERS: $(libenginesinc_HEADERS)
	@$(NORMAL_INSTALL)
	@if test x"$(libenginesinc_HEADERS)" = x""; then :; else \
	  echo " $(mkinstalldirs) $(DESTDIR)$(libenginesincdir)"; \
	  $(mkinstalldirs) $(DESTDIR)$(libenginesincdir); \
	fi
	@list='$(libenginesinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  src="$$d$$p"; dst="$(DESTDIR)$(libenginesincdir)/$$f"; \
	  if cmp -s "$$src" "$$dst"; then :; else \
	    echo " $(INSTALL_HEADER) $$src $$dst"; \
	    $(INSTALL_HEADER) "$$src" "$$dst" || exit 1; \
	  fi \
	done
	@list='$(ObsoletedHeaders)'; for f in $$list; do \
	  file="$(DESTDIR)$(libenginesincdir)/$$f"; \
	  if test -x "$$file"; then \
	    echo " rm -f $$file"; \
	    rm -f "$$file"; \
	  fi \
	done; \
	:

