
INCLUDES = -I$(top_srcdir)/lib

if BUILD_WITH_MSVC
noinst_LIBRARIES = nodes.lst
else
noinst_LTLIBRARIES = libnodes.la
endif

libnodes_la_SOURCES = \
	AutoFile.cpp AutoFile.h \
	DepthBuffer.cpp DepthBuffer.h \
	ViewportRegion.cpp ViewportRegion.h \
	Coinboard.cpp Coinboard.h \
	Switchboard.cpp Switchboard.h \
	SwitchboardOperator.cpp SwitchboardOperator.h \
	CoinEnvironment.cpp CoinEnvironment.h \
	SkyDome.cpp SkyDome.h \
	ShapeScale.cpp ShapeScale.h \
	PickSwitch.cpp PickSwitch.h \
	PickCallback.cpp PickCallback.h \
	SoTCBCurve.cpp \
	SoTCBCurve.h \
	SoText2Set.cpp \
	SoText2Set.h \
	SoPointCloud.cpp \
	SoPointCloud.h \
	SoLODExtrusion.cpp \
	SoLODExtrusion.h

nodes_lst_SOURCES = \
	AutoFile.cpp AutoFile.h \
	DepthBuffer.cpp DepthBuffer.h \
	ViewportRegion.cpp ViewportRegion.h \
	Coinboard.cpp Coinboard.h \
	Switchboard.cpp Switchboard.h \
	SwitchboardOperator.cpp SwitchboardOperator.h \
	CoinEnvironment.cpp CoinEnvironment.h \
	SkyDome.cpp SkyDome.h \
	ShapeScale.cpp ShapeScale.h \
	PickSwitch.cpp PickSwitch.h \
	PickCallback.cpp PickCallback.h \
	SoTCBCurve.cpp \
	SoTCBCurve.h \
	SoText2Set.cpp \
	SoText2Set.h \
	SoPointCloud.cpp \
	SoPointCloud.h \
	SoLODExtrusion.cpp \
	SoLODExtrusion.h

libnodesincdir = $(includedir)/SmallChange/nodes
libnodesinc_HEADERS = \
	$(top_srcdir)/lib/SmallChange/nodes/AutoFile.h \
	$(top_srcdir)/lib/SmallChange/nodes/ViewportRegion.h \
	$(top_srcdir)/lib/SmallChange/nodes/DepthBuffer.h \
	$(top_srcdir)/lib/SmallChange/nodes/Coinboard.h \
	$(top_srcdir)/lib/SmallChange/nodes/Switchboard.h \
	$(top_srcdir)/lib/SmallChange/nodes/SwitchboardOperator.h \
	$(top_srcdir)/lib/SmallChange/nodes/PickSwitch.h \
	$(top_srcdir)/lib/SmallChange/nodes/PickCallback.h \
	$(top_srcdir)/lib/SmallChange/nodes/SkyDome.h \
	$(top_srcdir)/lib/SmallChange/nodes/CoinEnvironment.h \
	$(top_srcdir)/lib/SmallChange/nodes/ShapeScale.h \
	$(top_srcdir)/lib/SmallChange/nodes/SoText2Set.h \
	$(top_srcdir)/lib/SmallChange/nodes/SoTCBCurve.h \
	$(top_srcdir)/lib/SmallChange/nodes/SoPointCloud.h \
	$(top_srcdir)/lib/SmallChange/nodes/SoLODExtrusion.h

nodes.lst: $(nodes_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(nodes_lst_OBJECTS); do echo $$i >>$@; done

