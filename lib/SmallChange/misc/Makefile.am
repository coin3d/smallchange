
INCLUDES = -I$(top_srcdir)/lib -I$(top_builddir)/lib

EXTRA_DIST = \
	SbList.h \
	SbHash.h \
	SmHash.h \
	SbVec3.h \
	SbBox3.h \
	SbPlane.h

if BUILD_WITH_MSVC
noinst_LIBRARIES = misc.lst
else
noinst_LTLIBRARIES = libmisc.la
endif

libmisc_la_SOURCES = \
	Init.cpp Init.h \
	SbCubicSpline.cpp SbCubicSpline.h \
	SceneManager.cpp SmSceneManager.h \
	Envelope.cpp SmEnvelope.h \
        cameracontrol.cpp cameracontrol.h

misc_lst_SOURCES = \
	Init.cpp Init.h \
	SbCubicSpline.cpp SbCubicSpline.h \
	SceneManager.cpp SmSceneManager.h \
	Envelope.cpp SmEnvelope.h \
        cameracontrol.cpp cameracontrol.h

libmiscincdir = $(includedir)/SmallChange/misc
libmiscinc_HEADERS = \
	Init.h \
	SbCubicSpline.h \
	SmSceneManager.h \
	SmEnvelope.h

misc.lst: $(misc_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(misc_lst_OBJECTS); do echo $$i >>$@; done

install-libmiscincHEADERS: $(libmiscinc_HEADERS)
	@$(NORMAL_INSTALL)
	@if test x"$(libmiscinc_HEADERS)" = x""; then :; else \
	  echo " $(mkinstalldirs) $(DESTDIR)$(libmiscincdir)"; \
	  $(mkinstalldirs) $(DESTDIR)$(libmiscincdir); \
	fi
	@list='$(libmiscinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  src="$$d$$p"; dst="$(DESTDIR)$(libmiscincdir)/$$f"; \
	  if cmp -s "$$src" "$$dst"; then :; else \
	    echo " $(INSTALL_HEADER) $$src $$dst"; \
	    $(INSTALL_HEADER) "$$src" "$$dst" || exit 1; \
	  fi \
	done
	@list='$(ObsoletedHeaders)'; for f in $$list; do \
	  file="$(DESTDIR)$(libmiscincdir)/$$f"; \
	  if test -x "$$file"; then \
	    echo " rm -f $$file"; \
	    rm -f "$$file"; \
	  fi \
	done; \
	:

