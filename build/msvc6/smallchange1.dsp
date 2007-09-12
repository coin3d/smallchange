# Microsoft Developer Studio Project File - Name="smallchange1" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=smallchange1 - Win32 DLL (Release)
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "smallchange1.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "smallchange1.mak" CFG="smallchange1 - Win32 DLL (Debug)"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "smallchange1 - Win32 LIB (Release)" (based on "Win32 (x86) Static Library")
!MESSAGE "smallchange1 - Win32 LIB (Debug)" (based on "Win32 (x86) Static Library")
!MESSAGE "smallchange1 - Win32 DLL (Release)" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "smallchange1 - Win32 DLL (Debug)" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "StaticRelease"
# PROP BASE Intermediate_Dir "StaticRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "StaticRelease"
# PROP Intermediate_Dir "StaticRelease"
# PROP Target_Dir ""
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /Ob2 /Og /Oi /Oy /Gy /I "lib" /I "..\..\lib" /I "." /I "$(COINDIR)\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_LIB" /D SMALLCHANGE_DEBUG=0  /D "HAVE_CONFIG_H" /D "SMALLCHANGE_INTERNAL" /D "COIN_DLL" /YX /FD /Gs /c
# ADD CPP /nologo /MD /W3 /GX /O2 /Ob2 /Og /Oi /Oy /Gy /I "lib" /I "..\..\lib" /I "." /I "$(COINDIR)\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_LIB" /D SMALLCHANGE_DEBUG=0  /D "HAVE_CONFIG_H" /D "SMALLCHANGE_INTERNAL" /D "COIN_DLL" /YX /FD /Gs /c
RSC=rc.exe
# ADD BASE RSC /l 0x414 /d "NDEBUG"
# ADD RSC /l 0x414 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /machine:I386 /out:"smallchange1s.lib"
# ADD LIB32 /nologo /machine:I386 /out:"smallchange1s.lib"

!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "StaticDebug"
# PROP BASE Intermediate_Dir "StaticDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "StaticDebug"
# PROP Intermediate_Dir "StaticDebug"
# PROP Target_Dir ""
MTL=midl.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /Od /I "lib" /I "..\..\lib" /I "." /I "$(COINDIR)\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_LIB" /D SMALLCHANGE_DEBUG=1  /D "HAVE_CONFIG_H" /D "SMALLCHANGE_INTERNAL" /D "COIN_DLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Od /I "lib" /I "..\..\lib" /I "." /I "$(COINDIR)\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_LIB" /D SMALLCHANGE_DEBUG=1  /D "HAVE_CONFIG_H" /D "SMALLCHANGE_INTERNAL" /D "COIN_DLL" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x414 /d "_DEBUG"
# ADD RSC /l 0x414 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /machine:I386 /out:"smallchange1sd.lib"
# ADD LIB32 /nologo /machine:I386 /out:"smallchange1sd.lib"

!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /Og /Oi /Oy /Ob2 /Gy /I "lib" /I "..\..\lib" /I "." /I "$(COINDIR)\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D SMALLCHANGE_DEBUG=0 /D "HAVE_CONFIG_H" /D "SMALLCHANGE_MAKE_DLL" /D "SMALLCHANGE_INTERNAL" /D "COIN_DLL" /YX /FD /Gs /c
# ADD CPP /nologo /MD /W3 /GX /O2 /Og /Oi /Oy /Ob2 /Gy /I "lib" /I "..\..\lib" /I "." /I "$(COINDIR)\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D SMALLCHANGE_DEBUG=0 /D "HAVE_CONFIG_H" /D "SMALLCHANGE_MAKE_DLL" /D "SMALLCHANGE_INTERNAL" /D "COIN_DLL" /YX /FD /Gs /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
RCS=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 $(COINDIR)\lib\coin2.lib opengl32.lib /nologo /dll /machine:I386
# ADD LINK32 $(COINDIR)\lib\coin2.lib opengl32.lib /nologo /dll /machine:I386 /out:"smallchange1.dll" /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "lib" /I "..\..\lib" /I "." /I "$(COINDIR)\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D SMALLCHANGE_DEBUG=1 /D "HAVE_CONFIG_H" /D "SMALLCHANGE_MAKE_DLL" /D "SMALLCHANGE_INTERNAL" /D "COIN_DLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "lib" /I "..\..\lib" /I "." /I "$(COINDIR)\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D SMALLCHANGE_DEBUG=1 /D "HAVE_CONFIG_H" /D "SMALLCHANGE_MAKE_DLL" /D "SMALLCHANGE_INTERNAL" /D "COIN_DLL" /YX /FD /GZ /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
RCS=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 $(COINDIR)\lib\coin2.lib opengl32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 $(COINDIR)\lib\coin2.lib opengl32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /out:"smallchange1d.dll"

!ENDIF

# Begin Target

# Name "smallchange1 - Win32 DLL (Release)"
# Name "smallchange1 - Win32 DLL (Debug)"
# Name "smallchange1 - Win32 LIB (Release)"
# Name "smallchange1 - Win32 LIB (Debug)"
# Begin Group "Documents"
# PROP Default_Filter ";txt"
# Begin Source File

SOURCE=..\..\README
# End Source File
# Begin Source File

SOURCE=..\..\NEWS
# End Source File
# Begin Source File

SOURCE=..\..\LICENSE.GPL
# End Source File
# Begin Source File

SOURCE=..\..\COPYING
# End Source File
# Begin Source File

SOURCE=..\..\THANKS
# End Source File
# Begin Source File

SOURCE=..\..\ChangeLog
# End Source File
# End Group
# Begin Group "Template Files"
# PROP Default_Filter "in"
# End Group
# Begin Group "Source Files"
# PROP Default_Filter "c;cpp;ic;icc;h"

# Begin Group "SmallChange/nodekits sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\LegendKit.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodekits"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SoFEMKit.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodekits"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmTooltipKit.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodekits"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmWellLogKit.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodekits"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmCameraControlKit.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodekits"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\GeoMarkerKit.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodekits"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\NormalsKit.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodekits"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmAxisDisplayKit.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodekits"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmAxisKit.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodekits"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmPieChart.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodekits"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\PopupMenuKit.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodekits"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmDynamicObjectKit.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodekits"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmTrackPointKit.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodekits"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodekits"
!ENDIF
# End Source File
# End Group
# Begin Group "SmallChange/nodes sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\AutoFile.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\DepthBuffer.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\ViewportRegion.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\Coinboard.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\Switchboard.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SwitchboardOperator.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\CoinEnvironment.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SkyDome.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\ShapeScale.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\PickSwitch.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\PickCallback.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SoTCBCurve.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SoText2Set.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SoPointCloud.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SoLODExtrusion.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\UTMCamera.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\UTMPosition.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\UTMCoordinate.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmTooltip.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmHQSphere.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmBillboardClipPlane.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmHeadlight.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\VertexArrayShape.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmCoordinateSystem.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmMarkerSet.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\ViewpointWrapper.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmShadowText2.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmTrack.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmLazyFile.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmTextureText2.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\nodes"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\nodes"
!ENDIF
# End Source File
# End Group
# Begin Group "SmallChange/elements sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\elements\GLDepthBufferElement.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\elements"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\elements"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\elements"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\elements"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\elements\UTMElement.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\elements"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\elements"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\elements"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\elements"
!ENDIF
# End Source File
# End Group
# Begin Group "SmallChange/engines sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\engines\Rot2Heading.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\engines"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\engines"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\engines"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\engines"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\engines\CubicSplineEngine.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\engines"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\engines"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\engines"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\engines"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\engines\SmInverseRotation.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\engines"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\engines"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\engines"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\engines"
!ENDIF
# End Source File
# End Group
# Begin Group "SmallChange/actions sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\actions\SoGenerateSceneGraphAction.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\actions"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\actions"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\actions"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\actions"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\actions\SoTweakAction.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\actions"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\actions"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\actions"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\actions"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\actions\ToVertexArrayShapeAction.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\actions"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\actions"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\actions"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\actions"
!ENDIF
# End Source File
# End Group
# Begin Group "SmallChange/misc sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\misc\Init.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\misc"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\misc\SbCubicSpline.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\misc"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\misc\SceneManager.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\misc"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\misc\Envelope.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\misc"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\misc\cameracontrol.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\misc"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\misc"
!ENDIF
# End Source File
# End Group
# Begin Group "SmallChange/draggers sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\draggers\SoAngle1Dragger.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\draggers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\draggers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\draggers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\draggers"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\draggers\SoAngle1Manip.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\draggers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\draggers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\draggers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\draggers"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\draggers\SmRangeTranslate1Dragger.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\draggers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\draggers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\draggers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\draggers"
!ENDIF
# End Source File
# End Group
# Begin Group "SmallChange/eventhandlers sources"
# PROP Default_Filter "c;cpp;ic;icc;h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\eventhandlers\SmEventHandler.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\eventhandlers"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\eventhandlers\SmExaminerEventHandler.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\eventhandlers"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\eventhandlers\SmSphereEventHandler.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\eventhandlers"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\eventhandlers\SmHelicopterEventHandler.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\eventhandlers"
!ENDIF
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\eventhandlers\SmPanEventHandler.cpp
!IF  "$(CFG)" == "smallchange1 - Win32 DLL (Release)"
# PROP Intermediate_Dir "Release\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 DLL (Debug)"
# PROP Intermediate_Dir "Debug\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Release)"
# PROP Intermediate_Dir "StaticRelease\SmallChange\eventhandlers"
!ELSEIF  "$(CFG)" == "smallchange1 - Win32 LIB (Debug)"
# PROP Intermediate_Dir "StaticDebug\SmallChange\eventhandlers"
!ENDIF
# End Source File
# End Group
# End Group
# Begin Group "Public Headers"

# PROP Default_Filter "h;ic;icc"
# Begin Group "SmallChange\nodekits headers"
# Set Default_Filter "h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\LegendKit.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SoFEMKit.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmTooltipKit.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmWellLogKit.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmCameraControlKit.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmGeoMarkerKit.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmNormalsKit.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmAxisDisplayKit.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmAxisKit.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmPieChart.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmPopupMenuKit.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmDynamicObjectKit.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodekits\SmTrackPointKit.h
# End Source File
# End Group
# Begin Group "SmallChange\nodes headers"
# Set Default_Filter "h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\AutoFile.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\ViewportRegion.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\DepthBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\Coinboard.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmSwitchboard.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmSwitchboardOperator.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\PickSwitch.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\PickCallback.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SkyDome.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\CoinEnvironment.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\ShapeScale.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SoText2Set.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SoTCBCurve.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SoPointCloud.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SoLODExtrusion.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\UTMCamera.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\UTMPosition.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\UTMCoordinate.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmTooltip.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmHQSphere.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmBillboardClipPlane.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmHeadlight.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmVertexArrayShape.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmCoordinateSystem.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmMarkerSet.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmViewpointWrapper.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmShadowText2.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmTrack.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmLazyFile.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SmTextureText2.h
# End Source File
# End Group
# Begin Group "SmallChange\elements headers"
# Set Default_Filter "h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\elements\UTMElement.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\elements\GLDepthBufferElement.h
# End Source File
# End Group
# Begin Group "SmallChange\engines headers"
# Set Default_Filter "h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\engines\Rot2Heading.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\engines\CubicSplineEngine.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\engines\SmInverseRotation.h
# End Source File
# End Group
# Begin Group "SmallChange\actions headers"
# Set Default_Filter "h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\actions\SoGenerateSceneGraphAction.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\actions\SoTweakAction.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\actions\SmToVertexArrayShapeAction.h
# End Source File
# End Group
# Begin Group "SmallChange\misc headers"
# Set Default_Filter "h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\misc\Init.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\misc\SbCubicSpline.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\misc\SmSceneManager.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\misc\SmEnvelope.h
# End Source File
# End Group
# Begin Group "SmallChange\draggers headers"
# Set Default_Filter "h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\draggers\SmRangeTranslate1Dragger.h
# End Source File
# End Group
# Begin Group "SmallChange\eventhandlers headers"
# Set Default_Filter "h"
# Begin Source File

SOURCE=..\..\lib\SmallChange\eventhandlers\SmEventHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\eventhandlers\SmExaminerEventHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\eventhandlers\SmHelicopterEventHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\eventhandlers\SmSphereEventHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\SmallChange\eventhandlers\SmPanEventHandler.h
# End Source File
# End Group
# Begin Group "SmallChange headers"
# Set Default_Filter "h"
# Begin Source File

SOURCE=lib\SmallChange\basic.h
# End Source File
# End Group
# End Group
# Begin Group "Private Headers"

# PROP Default_Filter "h;ic;icc"
# Begin Group "nodes local includes"

# PROP Default_Filter "h;ic;icc"
# Begin Source File

SOURCE=..\..\lib\SmallChange\nodes\SceneryRender.icc

# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Group
# End Target
# End Project
