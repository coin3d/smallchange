INCLUDES = -I$(top_srcdir)/lib -I$(top_builddir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = draggers.lst
else
noinst_LTLIBRARIES = libdraggers.la
endif

libdraggers_la_SOURCES = \
	SoAngle1Dragger.cpp SoAngle1Dragger.h \
	SoAngle1Manip.cpp SoAngle1Manip.h \
	SmRangeTranslate1Dragger.cpp SmRangeTranslate1Dragger.h

draggers_lst_SOURCES = \
	SoAngle1Dragger.cpp SoAngle1Dragger.h \
	SoAngle1Manip.cpp SoAngle1Manip.h \
	SmRangeTranslate1Dragger.cpp SmRangeTranslate1Dragger.h

libdraggersincdir = $(includedir)/SmallChange/draggers
libdraggersinc_HEADERS = \
	SoAngle1Dragger.h \
	SoAngle1Manip.h \
	SmRangeTranslate1Dragger.h

draggers.lst: $(draggers_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(draggers_lst_OBJECTS); do echo $$i >>$@; done

install-libdraggersincHEADERS: $(libdraggersinc_HEADERS)
	@$(NORMAL_INSTALL)
	@if test x"$(libdraggersinc_HEADERS)" = x""; then :; else \
	  echo " $(mkinstalldirs) $(DESTDIR)$(libdraggersincdir)"; \
	  $(mkinstalldirs) $(DESTDIR)$(libdraggersincdir); \
	fi
	@list='$(libdraggersinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  src="$$d$$p"; dst="$(DESTDIR)$(libdraggersincdir)/$$f"; \
	  if cmp -s "$$src" "$$dst"; then :; else \
	    echo " $(INSTALL_HEADER) $$src $$dst"; \
	    $(INSTALL_HEADER) "$$src" "$$dst" || exit 1; \
	  fi \
	done
	@list='$(ObsoletedHeaders)'; for f in $$list; do \
	  file="$(DESTDIR)$(libdraggersincdir)/$$f"; \
	  if test -x "$$file"; then \
	    echo " rm -f $$file"; \
	    rm -f "$$file"; \
	  fi \
	done; \
	:

