
INCLUDES = -I$(top_srcdir)/lib

SUBDIRS = nodekits nodes elements engines actions misc draggers .

if BUILD_WITH_MSVC
lib_LIBRARIES = SmallChange@SMALLCHANGE_MAJOR_VERSION@.lib
else
lib_LTLIBRARIES = libSmallChange.la
endif

libSmallChange_la_SOURCES =

libSmallChange_la_LIBADD = \
	nodekits/libnodekits.la nodes/libnodes.la elements/libelements.la \
	engines/libengines.la actions/libactions.la misc/libmisc.la \
	draggers/libdraggers.la

libSmallChange_la_LDFLAGS = \
	-no-undefined \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

SmallChange@SMALLCHANGE_MAJOR_VERSION@_lib_SOURCES =
SmallChange@SMALLCHANGE_MAJOR_VERSION@_lib_LIBADD = \
	nodekits/nodekits.lst nodes/nodes.lst elements/elements.lst \
	engines/engines.lst actions/actions.lst misc/misc.lst \
	draggers/draggers.lst

SmallChange@SMALLCHANGE_MAJOR_VERSION@.lib: Makefile \
  $(libSmallChange_lib_OBJECTS) \
  $(libSmallChange_lib_DEPENDENCIES)
	@basename="SmallChange@SMALLCHANGE_MAJOR_VERSION@"; \
	rm -f "$$basename.dll" "$$basename.lib"; \
	echo "Linking $$basename.lib..."; \
	echo $(CC) $(LDFLAGS) $(LIBFLAGS) \
	  $(SmallChange@SMALLCHANGE_MAJOR_VERSION@_lib_OBJECTS) \
	  $(SmallChange@SMALLCHANGE_MAJOR_VERSION@_lib_LIBADD) $(LIBS); \
	$(CC) $(LDFLAGS) $(LIBFLAGS) \
	  $(SmallChange@SMALLCHANGE_MAJOR_VERSION@_lib_OBJECTS) \
	  $(SmallChange@SMALLCHANGE_MAJOR_VERSION@_lib_LIBADD) $(LIBS);

# **************************************************************************

if BUILD_WITH_MSVC
install-exec-local: SmallChange@SMALLCHANGE_MAJOR_VERSION@.lib
	@basename="SmallChange@SMALLCHANGE_MAJOR_VERSION@"; \
	for ext in dll pdb lib; do \
	  file="$$basename.$$ext"; \
	  if test -s "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(bindir)"; \
	    echo $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(bindir)/$$file"; \
	    $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(bindir)/$$file"; \
	  fi \
	done
else
install-exec-local:
endif

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

