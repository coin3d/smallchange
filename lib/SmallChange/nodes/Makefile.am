INCLUDES = -I$(top_srcdir)/lib -I$(top_builddir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = nodes.lst
else
noinst_LTLIBRARIES = libnodes.la
endif

SOURCES = \
	AutoFile.cpp AutoFile.h \
	SmDepthBuffer.cpp SmDepthBuffer.h \
	ViewportRegion.cpp ViewportRegion.h \
	Coinboard.cpp Coinboard.h \
	Switchboard.cpp SmSwitchboard.h \
	SwitchboardOperator.cpp SmSwitchboardOperator.h \
	CoinEnvironment.cpp CoinEnvironment.h \
	SkyDome.cpp SkyDome.h \
	ShapeScale.cpp ShapeScale.h \
	PickSwitch.cpp PickSwitch.h \
	PickCallback.cpp PickCallback.h \
	SoTCBCurve.cpp SoTCBCurve.h \
	SoText2Set.cpp SoText2Set.h \
	SoPointCloud.cpp SoPointCloud.h \
	SoLODExtrusion.cpp SoLODExtrusion.h \
	UTMCamera.cpp UTMCamera.h \
	UTMPosition.cpp UTMPosition.h \
	UTMCoordinate.cpp UTMCoordinate.h \
	SmTooltip.cpp SmTooltip.h \
	SmHQSphere.cpp SmHQSphere.h \
	SmBillboardClipPlane.cpp SmBillboardClipPlane.h \
	SmHeadlight.cpp SmHeadlight.h \
	VertexArrayShape.cpp SmVertexArrayShape.h \
	SmCoordinateSystem.cpp SmCoordinateSystem.h \
	SmMarkerSet.cpp SmMarkerSet.h \
	ViewpointWrapper.cpp SmViewpointWrapper.h \
        SmShadowText2.cpp SmShadowText2.h \
        SmTrack.cpp SmTrack.h \
        SmLazyFile.cpp SmLazyFile.h \
	SmTextureText2.cpp SmTextureText2.h \
	SmTextureText2Collector.cpp SmTextureText2Collector.h \
	SmTextureFont.cpp SmTextureFont.h


libnodes_la_SOURCES = $(SOURCES)

nodes_lst_SOURCES = $(SOURCES)

libnodesincdir = $(includedir)/SmallChange/nodes
libnodesinc_HEADERS = \
	AutoFile.h \
	ViewportRegion.h \
	SmDepthBuffer.h \
	Coinboard.h \
	SmSwitchboard.h \
	SmSwitchboardOperator.h \
	PickSwitch.h \
	PickCallback.h \
	SkyDome.h \
	CoinEnvironment.h \
	ShapeScale.h \
	SoText2Set.h \
	SoTCBCurve.h \
	SoPointCloud.h \
	SoLODExtrusion.h \
	UTMCamera.h \
	UTMPosition.h \
	UTMCoordinate.h \
	SmTooltip.h \
	SmHQSphere.h \
	SmBillboardClipPlane.h \
	SmHeadlight.h \
	SmVertexArrayShape.h \
	SmCoordinateSystem.h \
	SmMarkerSet.h \
	SmViewpointWrapper.h \
        SmShadowText2.h \
        SmTrack.h \
        SmLazyFile.h \
	SmTextureText2.h \
	SmTextureText2Collector.h \
	SmTextureFont.h

nodes.lst: $(nodes_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(nodes_lst_OBJECTS); do echo $$i >>$@; done

install-libnodesincHEADERS: $(libnodesinc_HEADERS)
	@$(NORMAL_INSTALL)
	@if test x"$(libnodesinc_HEADERS)" = x""; then :; else \
	  echo " $(mkinstalldirs) $(DESTDIR)$(libnodesincdir)"; \
	  $(mkinstalldirs) $(DESTDIR)$(libnodesincdir); \
	fi
	@list='$(libnodesinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  src="$$d$$p"; dst="$(DESTDIR)$(libnodesincdir)/$$f"; \
	  if cmp -s "$$src" "$$dst"; then :; else \
	    echo " $(INSTALL_HEADER) $$src $$dst"; \
	    $(INSTALL_HEADER) "$$src" "$$dst" || exit 1; \
	  fi \
	done
	@list='$(ObsoletedHeaders)'; for f in $$list; do \
	  file="$(DESTDIR)$(libnodesincdir)/$$f"; \
	  if test -x "$$file"; then \
	    echo " rm -f $$file"; \
	    rm -f "$$file"; \
	  fi \
	done; \
	:

