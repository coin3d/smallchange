# Microsoft Developer Studio Project File - Name="Simple" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Simple - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Simple.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Simple.mak" CFG="Simple - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Simple - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Simple - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Simple - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x414 /d "NDEBUG"
# ADD RSC /l 0x414 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "Simple - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GR /GX /ZI /Od /I "$(CODE_INSTALL)\include" /I "$(SDK_PTHREADS)\include" /I "$(CODE_CHECKOUT)\openal\win\AL" /I "$(CODE_CHECKOUT)\openal\win\ALc" /I "$(CODE_CHECKOUT)\openal\win\ALu" /I "$(CODE_CHECKOUT)\openal\win\ALut" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "COIN_DLL" /D "SIMAGE_DLL" /D "SOWIN_DLL" /D "HAVE_PTHREAD" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x414 /d "_DEBUG"
# ADD RSC /l 0x414 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(CODE_INSTALL)\lib\Coin0.lib $(CODE_INSTALL)\lib\SoWin0.lib $(CODE_CHECKOUT)\openal\win\Alut\Debug\ALut.lib $(CODE_CHECKOUT)\openal\win\OpenAL32\Debug\OpenAL32.lib winmm.lib $(SDK_PTHREADS)\lib\pthreadVC.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Simple - Win32 Release"
# Name "Simple - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ALTools.cpp
# End Source File
# Begin Source File

SOURCE=.\SbAudioWorkerThread.cpp
# End Source File
# Begin Source File

SOURCE=.\simple.cpp

!IF  "$(CFG)" == "Simple - Win32 Release"

!ELSEIF  "$(CFG)" == "Simple - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\simple2.cpp
# End Source File
# Begin Source File

SOURCE=.\simple3.cpp

!IF  "$(CFG)" == "Simple - Win32 Release"

!ELSEIF  "$(CFG)" == "Simple - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SoAudioClip.cpp
# End Source File
# Begin Source File

SOURCE=.\SoAudioClipStreaming.cpp
# End Source File
# Begin Source File

SOURCE=.\SoAudioDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\SoAudioRenderAction.cpp
# End Source File
# Begin Source File

SOURCE=.\SoListener.cpp
# End Source File
# Begin Source File

SOURCE=.\SoSound.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\SbAudioWorkerThread.h
# End Source File
# Begin Source File

SOURCE=.\SoAudioClip.h
# End Source File
# Begin Source File

SOURCE=.\SoAudioClipStreaming.h
# End Source File
# Begin Source File

SOURCE=.\SoAudioDevice.h
# End Source File
# Begin Source File

SOURCE=.\SoAudioRenderAction.h
# End Source File
# Begin Source File

SOURCE=.\SoListener.h
# End Source File
# Begin Source File

SOURCE=.\SoSound.h
# End Source File
# Begin Source File

SOURCE=.\SoSoundBuffer.h
# End Source File
# Begin Source File

SOURCE=.\SoSoundscape.h
# End Source File
# Begin Source File

SOURCE=.\SoSoundSource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ToDo.txt
# End Source File
# End Target
# End Project
