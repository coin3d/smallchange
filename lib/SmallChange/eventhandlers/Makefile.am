INCLUDES = -I$(top_srcdir)/lib -I$(top_builddir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = eventhandlers.lst
else
noinst_LTLIBRARIES = libeventhandlers.la
endif

RegularSources = \
	SmEventHandler.cpp SmEventHandler.h \
	SmExaminerEventHandler.cpp SmExaminerEventHandler.h \
	SmSphereEventHandler.cpp SmSphereEventHandler.h \
	SmHelicopterEventHandler.cpp SmHelicopterEventHandler.h \
        SmPanEventHandler.cpp SmPanEventHandler.h

libeventhandlers_la_SOURCES = $(RegularSources)

eventhandlers_lst_SOURCES = $(RegularSources)

libeventhandlersincdir = $(includedir)/SmallChange/eventhandlers
libeventhandlersinc_HEADERS = \
	SmEventHandler.h \
	SmExaminerEventHandler.h \
	SmHelicopterEventHandler.h \
	SmSphereEventHandler.h \
        SmPanEventHandler.h

eventhandlers.lst: $(eventhandlers_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(eventhandlers_lst_OBJECTS); do echo $$i >>$@; done

install-libeventhandlersincHEADERS: $(libeventhandlersinc_HEADERS)
	@$(NORMAL_INSTALL)
	@if test x"$(libeventhandlersinc_HEADERS)" = x""; then :; else \
	  echo " $(mkinstalldirs) $(DESTDIR)$(libeventhandlersincdir)"; \
	  $(mkinstalldirs) $(DESTDIR)$(libeventhandlersincdir); \
	fi
	@list='$(libeventhandlersinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  src="$$d$$p"; dst="$(DESTDIR)$(libeventhandlersincdir)/$$f"; \
	  if cmp -s "$$src" "$$dst"; then :; else \
	    echo " $(INSTALL_HEADER) $$src $$dst"; \
	    $(INSTALL_HEADER) "$$src" "$$dst" || exit 1; \
	  fi \
	done
	@list='$(ObsoletedHeaders)'; for f in $$list; do \
	  file="$(DESTDIR)$(libeventhandlersincdir)/$$f"; \
	  if test -x "$$file"; then \
	    echo " rm -f $$file"; \
	    rm -f "$$file"; \
	  fi \
	done; \
	:

