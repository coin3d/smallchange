
INCLUDES = -I$(top_srcdir)/lib
# INCLUDES = -I$(top_srcdir)/lib -I/cygdrive/e/Users/thammer/Code/Checkout/openal/include

if BUILD_WITH_MSVC
noinst_LIBRARIES = nodes.lst
else
noinst_LTLIBRARIES = libnodes.la
endif

libnodes_la_SOURCES = \
	DepthBuffer.cpp DepthBuffer.h \
	ViewportRegion.cpp ViewportRegion.h \
	Coinboard.cpp Coinboard.h \
	Switchboard.cpp Switchboard.h \
	SwitchboardOperator.cpp SwitchboardOperator.h \
	SoAudioClip.cpp \
	SoAudioClip.h \
	SoAudioClipP.h \
	SoAudioClipStreaming.cpp \
	SoAudioClipStreaming.h \
	SoAudioClipStreamingP.h \
	SoListener.cpp \
	SoListener.h \
	SoSound.cpp \
	SoSound.h \
	SoSoundP.h

nodes_lst_SOURCES = \
	DepthBuffer.cpp DepthBuffer.h \
	ViewportRegion.cpp ViewportRegion.h \
	Coinboard.cpp Coinboard.h \
	Switchboard.cpp Switchboard.h \
	SwitchboardOperator.cpp SwitchboardOperator.h \
	SoAudioClip.cpp \
	SoAudioClip.h \
	SoAudioClipP.h \
	SoAudioClipStreaming.cpp \
	SoAudioClipStreaming.h \
	SoAudioClipStreamingP.h \
	SoListener.cpp \
	SoListener.h \
	SoSound.cpp \
	SoSound.h \
	SoSoundP.h

libnodesincdir = $(includedir)/SmallChange/nodes
libnodesinc_HEADERS = \
	$(top_srcdir)/lib/SmallChange/nodes/ViewportRegion.h \
	$(top_srcdir)/lib/SmallChange/nodes/DepthBuffer.h \
	$(top_srcdir)/lib/SmallChange/nodes/Coinboard.h \
	$(top_srcdir)/lib/SmallChange/nodes/Switchboard.h \
	$(top_srcdir)/lib/SmallChange/nodes/SwitchboardOperator.h \
	$(top_srcdir)/lib/SmallChange/nodes/SoAudioClip.h \
	$(top_srcdir)/lib/SmallChange/nodes/SoAudioClipStreaming.h \
	$(top_srcdir)/lib/SmallChange/nodes/SoListener.h \
	$(top_srcdir)/lib/SmallChange/nodes/SoSound.h 

nodes.lst: $(nodes_lst_OBJECTS)
	@echo "Linking $@..."; \
	rm -f $@; \
	for i in $(nodes_lst_OBJECTS); do echo $$i >>$@; done

