INCLUDES = -I$(top_srcdir)/lib -I$(top_builddir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = actions.lst
else
noinst_LTLIBRARIES = libactions.la
endif

libactions_la_SOURCES = \
	SoGenerateSceneGraphAction.cpp \
	SoGenerateSceneGraphAction.h \
	SoTweakAction.cpp \
	SoTweakAction.h \
	SmToVertexArrayShapeAction.h \
	ToVertexArrayShapeAction.cpp

actions_lst_SOURCES = \
	SoGenerateSceneGraphAction.cpp \
	SoGenerateSceneGraphAction.h \
	SoTweakAction.cpp \
	SoTweakAction.h \
	SmToVertexArrayShapeAction.h \
	ToVertexArrayShapeAction.cpp

libactionsincdir = $(includedir)/SmallChange/actions
libactionsinc_HEADERS = \
	SoGenerateSceneGraphAction.h \
	SoTweakAction.h \
	SmToVertexArrayShapeAction.h

actions.lst: $(actions_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(actions_lst_OBJECTS); do echo $$i >>$@; done


install-libactionsincHEADERS: $(libactionsinc_HEADERS)
	@$(NORMAL_INSTALL)
	@if test x"$(libactionsinc_HEADERS)" = x""; then :; else \
	  echo " $(mkinstalldirs) $(DESTDIR)$(libactionsincdir)"; \
	  $(mkinstalldirs) $(DESTDIR)$(libactionsincdir); \
	fi
	@list='$(libactionsinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  src="$$d$$p"; dst="$(DESTDIR)$(libactionsincdir)/$$f"; \
	  if cmp -s "$$src" "$$dst"; then :; else \
	    echo " $(INSTALL_HEADER) $$src $$dst"; \
	    $(INSTALL_HEADER) "$$src" "$$dst" || exit 1; \
	  fi \
	done
	@list='$(ObsoletedHeaders)'; for f in $$list; do \
	  file="$(DESTDIR)$(libactionsincdir)/$$f"; \
	  if test -x "$$file"; then \
	    echo " rm -f $$file"; \
	    rm -f "$$file"; \
	  fi \
	done; \
	:

