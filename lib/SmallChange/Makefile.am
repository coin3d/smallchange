
INCLUDES = -I$(top_srcdir)/lib

if MAC_FRAMEWORK
SUBDIRS = 
else
SUBDIRS = nodekits nodes elements engines actions misc draggers eventhandlers .
endif

if BUILD_WITH_MSVC
lib_LIBRARIES = SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@.lib
else
lib_LTLIBRARIES = libSmallChange@SUFFIX@.la
endif

if MAC_FRAMEWORK
# hack to change libdir value when installing Mac OS X compiler framework
lib@nop@dir = $(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/$(MAC_FRAMEWORK_VERSION)/Libraries
endif


if MAC_FRAMEWORK
Sources = \
  actions/SoGenerateSceneGraphAction.cpp \
  actions/SoTweakAction.cpp \
  actions/ToVertexArrayShapeAction.cpp \
  draggers/SmRangeTranslate1Dragger.cpp \
  draggers/SoAngle1Dragger.cpp \
  draggers/SoAngle1Manip.cpp \
  elements/GLDepthBufferElement.cpp \
  elements/UTMElement.cpp \
  engines/CubicSplineEngine.cpp \
  engines/Rot2Heading.cpp \
  engines/SmInverseRotation.cpp \
  eventhandlers/SmEventHandler.cpp \
  eventhandlers/SmExaminerEventHandler.cpp \
  eventhandlers/SmHelicopterEventHandler.cpp \
  eventhandlers/SmPanEventHandler.cpp \
  eventhandlers/SmSphereEventHandler.cpp \
  misc/cameracontrol.cpp \
  misc/Envelope.cpp \
  misc/Init.cpp  \
  misc/SbCubicSpline.cpp \
  misc/SceneManager.cpp \
  nodekits/bitmapfont.cpp \
  nodekits/DynamicBaseKit.cpp \
  nodekits/GeoMarkerKit.cpp \
  nodekits/LegendKit.cpp \
  nodekits/NormalsKit.cpp \
  nodekits/PopupMenuKit.cpp \
  nodekits/SmAnnotationAxis.cpp \
  nodekits/SmAnnotationWall.cpp \
  nodekits/SmAxisDisplayKit.cpp \
  nodekits/SmAxisKit.cpp \
  nodekits/SmCameraControlKit.cpp \
  nodekits/SmDynamicObjectKit.cpp \
  nodekits/SmPieChart.cpp \
  nodekits/SmTooltipKit.cpp \
  nodekits/SmTrackPointKit.cpp \
  nodekits/SmWellLogKit.cpp \
  nodekits/SoCameraPathEditKit.cpp \
  nodekits/SoFEMKit.cpp \
  nodes/AutoFile.cpp \
  nodes/Coinboard.cpp \
  nodes/CoinEnvironment.cpp \
  nodes/InterleavedArraysShape.cpp \
  nodes/PickCallback.cpp \
  nodes/PickSwitch.cpp \
  nodes/ShapeScale.cpp \
  nodes/SkyDome.cpp \
  nodes/SmBillboardClipPlane.cpp \
  nodes/SmCoordinateSystem.cpp \
  nodes/SmDepthBuffer.cpp \
  nodes/SmHeadlight.cpp \
  nodes/SmHQSphere.cpp \
  nodes/SmLazyFile.cpp \
  nodes/SmMarkerSet.cpp \
  nodes/SmShadowText2.cpp \
  nodes/SmTextureFont.cpp \
  nodes/SmTextureText2.cpp \
  nodes/SmTextureText2Collector.cpp \
  nodes/SmTooltip.cpp \
  nodes/SmTrack.cpp \
  nodes/SoLODExtrusion.cpp \
  nodes/SoPointCloud.cpp \
  nodes/SoTCBCurve.cpp \
  nodes/SoText2Set.cpp \
  nodes/Switchboard.cpp \
  nodes/SwitchboardOperator.cpp \
  nodes/UTMCamera.cpp \
  nodes/UTMCoordinate.cpp \
  nodes/UTMPosition.cpp \
  nodes/VertexArrayShape.cpp \
  nodes/ViewpointWrapper.cpp \
  nodes/ViewportRegion.cpp
else
# Adding in a dummy C++ source code file to "trick" Automake to set
# us up with C++ $CXXLD for linking (and not the default C $CCLD).
Sources = dummy.cpp
endif

libSmallChange@SUFFIX@_la_SOURCES = $(Sources)

if MAC_FRAMEWORK
libSmallChange@SUFFIX@_la_LIBADD = 
else
libSmallChange@SUFFIX@_la_LIBADD = \
	nodekits/libnodekits.la nodes/libnodes.la elements/libelements.la \
	engines/libengines.la actions/libactions.la misc/libmisc.la \
	draggers/libdraggers.la eventhandlers/libeventhandlers.la
endif

libSmallChange@SUFFIX@_la_LDFLAGS = \
	-no-undefined \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@_lib_SOURCES = $(Sources)
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

if MAC_FRAMEWORK
libSmallChangeincdir = $(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/$(MAC_FRAMEWORK_VERSION)/Headers
else
libSmallChangeincdir = $(includedir)/SmallChange
endif


if MAC_FRAMEWORK
libSmallChangeinc_HEADERS = \
  basic.h \
  SmallChange.h \
  actions/SmToVertexArrayShapeAction.h \
  actions/SoGenerateSceneGraphAction.h \
  actions/SoTweakAction.h \
  draggers/SmRangeTranslate1Dragger.h \
  draggers/SoAngle1Dragger.h \
  draggers/SoAngle1Manip.h \
  elements/GLDepthBufferElement.h \
  elements/UTMElement.h \
  engines/CubicSplineEngine.h \
  engines/Rot2Heading.h \
  engines/SmInverseRotation.h \
  eventhandlers/SmEventHandler.h \
  eventhandlers/SmExaminerEventHandler.h \
  eventhandlers/SmHelicopterEventHandler.h \
  eventhandlers/SmPanEventHandler.h \
  eventhandlers/SmSphereEventHandler.h \
  misc/cameracontrol.h \
  misc/Init.h \
  misc/SbBox3.h \
  misc/SbCubicSpline.h \
  misc/SbHash.h \
  misc/SbList.h \
  misc/SbPlane.h \
  misc/SbVec3.h \
  misc/SmEnvelope.h \
  misc/SmHash.h \
  misc/SmSceneManager.h \
  nodekits/DynamicBaseKit.h \
  nodekits/DynamicNodeKit.h \
  nodekits/LegendKit.h \
  nodekits/SbList.h \
  nodekits/SmAnnotationAxis.h \
  nodekits/SmAnnotationWall.h \
  nodekits/SmAxisDisplayKit.h \
  nodekits/SmAxisKit.h \
  nodekits/SmCameraControlKit.h \
  nodekits/SmDynamicObjectKit.h \
  nodekits/SmGeoMarkerKit.h \
  nodekits/SmNormalsKit.h \
  nodekits/SmPieChart.h \
  nodekits/SmPopupMenuKit.h \
  nodekits/SmTooltipKit.h \
  nodekits/SmTrackPointKit.h \
  nodekits/SmWellLogKit.h \
  nodekits/SoCameraPathEditKit.h \
  nodekits/SoFEMKit.h \
  nodes/AutoFile.h \
  nodes/Coinboard.h \
  nodes/CoinEnvironment.h \
  nodes/InterleavedArraysShape.h \
  nodes/PickCallback.h \
  nodes/PickSwitch.h \
  nodes/ShapeScale.h \
  nodes/SkyDome.h \
  nodes/SmBillboardClipPlane.h \
  nodes/SmCoordinateSystem.h \
  nodes/SmDepthBuffer.h \
  nodes/SmHeadlight.h \
  nodes/SmHQSphere.h \
  nodes/SmLazyFile.h \
  nodes/SmMarkerSet.h \
  nodes/SmShadowText2.h \
  nodes/SmSwitchboard.h \
  nodes/SmSwitchboardOperator.h \
  nodes/SmTextureFont.h \
  nodes/SmTextureText2.h \
  nodes/SmTextureText2Collector.h \
  nodes/SmTooltip.h \
  nodes/SmTrack.h \
  nodes/SmVertexArrayShape.h \
  nodes/SmViewpointWrapper.h \
  nodes/SoLODExtrusion.h \
  nodes/SoPointCloud.h \
  nodes/SoTCBCurve.h \
  nodes/SoText2Set.h \
  nodes/UTMCamera.h \
  nodes/UTMCoordinate.h \
  nodes/UTMPosition.h \
  nodes/ViewportRegion.h
else
libSmallChangeinc_HEADERS = basic.h SmallChange.h
endif



# **************************************************************************

if BUILD_WITH_MSVC
install-exec-local: SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@.lib
	@basename="SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@"; \
	for ext in dll pdb; do \
	  file="$$basename.$$ext"; \
	  if test -s "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(bindir)"; \
	    echo $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(bindir)/$$file"; \
	    $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(bindir)/$$file" || exit 1; \
	  fi \
	done
else
install-exec-local:
endif

if MAC_FRAMEWORK
install-data-local:
	@$(NORMAL_INSTALL)
	@echo "Setting up Mac OS X framework"
	@$(mkinstalldirs) $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/$(MAC_FRAMEWORK_VERSION)
	@ln -sf $(MAC_FRAMEWORK_VERSION) $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/Current
	@ln -sf Versions/Current/Headers $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Headers
	@ln -sf Versions/Current/Libraries $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Libraries
	@ln -sf Versions/Current/Resources $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Resources
	cd $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework/Versions/Current && ln -sf Libraries/libSmallChange.dylib $(MAC_FRAMEWORK_NAME) && cd -; \
	cd $(DESTDIR)$(MAC_FRAMEWORK_PREFIX)/$(MAC_FRAMEWORK_NAME).framework && ln -sf Versions/Current/$(MAC_FRAMEWORK_NAME) $(MAC_FRAMEWORK_NAME) && install_name_tool -id $(MAC_FRAMEWORK_NAME).framework/Versions/$(MAC_FRAMEWORK_VERSION)/$(MAC_FRAMEWORK_NAME) $(MAC_FRAMEWORK_NAME) && cd -;
else
install-data-local:
	@basename="SmallChange@SMALLCHANGE_MAJOR_VERSION@@SUFFIX@"; \
	for ext in lib; do \
	  file="$$basename.$$ext"; \
	  if test -s "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(libdir)"; \
	    echo $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(libdir)/$$file"; \
	    $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(libdir)/$$file" || exit 1; \
	  fi \
	done
endif

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

install-libSmallChangeincHEADERS: $(libSmallChangeinc_HEADERS)
	@$(NORMAL_INSTALL)
	@if test x"$(libSmallChangeinc_HEADERS)" = x""; then :; else \
	  echo " $(mkinstalldirs) $(DESTDIR)$(libSmallChangeincdir)"; \
	  $(mkinstalldirs) $(DESTDIR)$(libSmallChangeincdir); \
	  if test x"$(MAC_FRAMEWORK)" = x"true"; then \
	    $(mkinstalldirs) $(DESTDIR)$(libSmallChangeincdir)/actions; \
	    $(mkinstalldirs) $(DESTDIR)$(libSmallChangeincdir)/draggers; \
	    $(mkinstalldirs) $(DESTDIR)$(libSmallChangeincdir)/elements; \
	    $(mkinstalldirs) $(DESTDIR)$(libSmallChangeincdir)/engines; \
	    $(mkinstalldirs) $(DESTDIR)$(libSmallChangeincdir)/eventhandlers; \
	    $(mkinstalldirs) $(DESTDIR)$(libSmallChangeincdir)/misc; \
	    $(mkinstalldirs) $(DESTDIR)$(libSmallChangeincdir)/nodekits; \
	    $(mkinstalldirs) $(DESTDIR)$(libSmallChangeincdir)/nodes; \
	  fi \
	fi
	@list='$(libSmallChangeinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  if test x"$(MAC_FRAMEWORK)" = x"true"; then f=$$p; else f="`echo $$p | sed -e 's|^.*/||'`"; fi; \
	  src="$$d$$p"; dst="$(DESTDIR)$(libSmallChangeincdir)/$$f"; \
	  if cmp -s "$$src" "$$dst"; then :; else \
	    echo " $(INSTALL_HEADER) $$src $$dst"; \
	    $(INSTALL_HEADER) "$$src" "$$dst" || exit 1; \
	  fi \
	done
	@list='$(ObsoletedHeaders)'; for f in $$list; do \
	  file="$(DESTDIR)$(libSmallChangeincdir)/$$f"; \
	  if test -x "$$file"; then \
	    echo " rm -f $$file"; \
	    rm -f "$$file"; \
	  fi \
	done; \
	:
