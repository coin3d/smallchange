
INCLUDES = -I$(top_srcdir)/lib

SUBDIRS = nodekits nodes elements engines actions misc draggers eventhandlers .

if BUILD_WITH_MSVC
lib_LIBRARIES = SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@.lib
else
lib_LTLIBRARIES = libSmallChange@SUFFIX@.la
endif

libSmallChange@SUFFIX@_la_SOURCES =

libSmallChange@SUFFIX@_la_LIBADD = \
	nodekits/libnodekits.la nodes/libnodes.la elements/libelements.la \
	engines/libengines.la actions/libactions.la misc/libmisc.la \
	draggers/libdraggers.la eventhandlers/libeventhandlers.la

libSmallChange@SUFFIX@_la_LDFLAGS = \
	-no-undefined \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@_lib_SOURCES =
SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@_lib_LIBADD = \
	nodekits/nodekits.lst nodes/nodes.lst elements/elements.lst \
	engines/engines.lst actions/actions.lst misc/misc.lst \
	draggers/draggers.lst eventhandlers/eventhandlers.lst

SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@.lib: Makefile \
  $(SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@_lib_OBJECTS) \
  $(SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@_lib_DEPENDENCIES)
	@basename="SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@"; \
	rm -f "$$basename.dll" "$$basename.lib"; \
	echo "Linking $$basename.lib..."; \
	echo $(CC) $(LDFLAGS) $(LIBFLAGS) \
	  $(SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@_lib_OBJECTS) \
	  $(SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@_lib_LIBADD) $(LIBS); \
	$(CC) $(LDFLAGS) $(LIBFLAGS) \
	  $(SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@_lib_OBJECTS) \
	  $(SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@_lib_LIBADD) $(LIBS);

# **************************************************************************

libSmallChange@SUFFIX@incdir = $(includedir)/SmallChange
libSmallChange@SUFFIX@inc_HEADERS = basic.h

# **************************************************************************

if BUILD_WITH_MSVC
install-exec-local: SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@.lib
	@basename="SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@"; \
	for ext in dll pdb; do \
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
	@basename="SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@"; \
	for ext in lib; do \
	  file="$$basename.$$ext"; \
	  if test -s "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(libdir)"; \
	    echo $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(libdir)/$$file"; \
	    $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(libdir)/$$file"; \
	  fi \
	done

uninstall-local:
	@basename="SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@"; \
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
	  "$$incbasedir/eventhandlers" \
	  "$$incbasedir"; do \
	  if test -d "$$dir"; then \
	    echo " rmdir" "$$dir"; \
	    rmdir "$$dir"; \
	  fi \
	done

