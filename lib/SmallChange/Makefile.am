SUBDIRS = nodekits nodes elements engines .

INCLUDES = -I$(top_srcdir)/lib

lib_LTLIBRARIES = libSmallChange.la

libSmallChange_la_SOURCES =

libSmallChange_la_LIBADD = \
	nodekits/libnodekits.la nodes/libnodes.la elements/libelements.la \
	engines/libengines.la

libSmallChange_la_LDFLAGS = \
	-no-undefined \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)


SmallChange@SMALLCHANGE_MAJOR_VERSION@.lib: Makefile $(libSmallChange_la_OBJECTS) $(libSmallChange_la_DEPENDENCIES)
	@basename="SmallChange@SMALLCHANGE_MAJOR_VERSION@"; \
	rm -f "$$basename.dll" "$$basename.lib"; \
	if @BUILD_WITH_MSVC@; then \
	  echo "Linking $$basename.lib..."; \
	  echo $(CC) $(LDFLAGS) $(LIBFLAGS) \
	    `echo $(libSmallChange_la_OBJECTS) | sed 's%\.lo%.obj%g'` \
	    $(libSmallChange_la_LIBADD) $(LIBS); \
	  $(CC) $(LDFLAGS) $(LIBFLAGS) \
	    `echo $(libSmallChange_la_OBJECTS) | sed 's%\.lo%.obj%g'` \
	    $(libSmallChange_la_LIBADD) $(LIBS); \
	else \
	  touch "$$basename.dll" "$$basename.lib"; \
	fi

# **************************************************************************

all-local: SmallChange@SMALLCHANGE_MAJOR_VERSION@.lib

install-exec-local:
	@basename="SmallChange@SMALLCHANGE_MAJOR_VERSION@"; \
	for ext in dll pdb; do \
	  file="$$basename.$$ext"; \
	  if test -s "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(bindir)"; \
	    echo $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(bindir)/$$file"; \
	    $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(bindir)/$$file"; \
	  fi \
	done

install-data-local:
	@basename="SmallChange@SMALLCHANGE_MAJOR_VERSION@"; \
	for ext in lib; do \
	  file="$$basename.$$ext"; \
	  if test -s "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(libdir)"; \
	    echo $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(libdir)/$$file"; \
	    $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(libdir)/$$file"; \
	  fi \
	done

uninstall-local:
	@basename="SmallChange@SMALLCHANGE_MAJOR_VERSION@"; \
	for file in \
	  "$(DESTDIR)$(bindir)/$$basename.dll" \
	  "$(DESTDIR)$(bindir)/$$basename.pdb" \
	  "$(DESTDIR)$(libdir)/$$basename.lib"; do \
	  if test -f "$$file"; then \
	    echo " rm -f" "$$file"; \
	    rm -f "$$file"; \
	  fi \
	done; \
	incbasedir="$(DESTDIR)$(includedir)/SmallChange"; \
	for dir in \
	  "$$incbasedir/elements" \
	  "$$incbasedir/nodekits" \
	  "$$incbasedir/nodes" \
	  "$$incbasedir"; do \
	  if test -d "$$dir"; then \
	    echo " rmdir" "$$dir"; \
	    rmdir "$$dir"; \
	  fi \
	done

