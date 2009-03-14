# Microsoft Developer Studio Project File - Name="q3map" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=q3map - Win32 Damage_skin_creator
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "q3map.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "q3map.mak" CFG="q3map - Win32 Damage_skin_creator"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "q3map - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "q3map - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "q3map - Win32 Damage_skin_creator" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "q3map - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "../common" /I "c:\quake3\common" /I "c:\quake3\libs" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 glaux.lib glu32.lib wsock32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\libs\pak.lib ..\libs\jpeg6.lib /nologo /stack:0x400000 /subsystem:console /debug /machine:I386

!ELSEIF  "$(CFG)" == "q3map - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../common" /I "c:\quake3\common" /I "c:\quake3\libs" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 glaux.lib glu32.lib wsock32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\libs\pakd.lib ..\libs\jpeg6d.lib /nologo /stack:0x400000 /subsystem:console /profile /debug /machine:I386 /out:"d:/quake3/tools/q3map.exe"

!ELSEIF  "$(CFG)" == "q3map - Win32 Damage_skin_creator"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "q3map___Win32_Damage_skin_creator"
# PROP BASE Intermediate_Dir "q3map___Win32_Damage_skin_creator"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "q3map___Win32_Damage_skin_creator"
# PROP Intermediate_Dir "q3map___Win32_Damage_skin_creator"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../common" /I "c:\quake3\common" /I "c:\quake3\libs" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../common" /I "c:\quake3\common" /I "c:\quake3\libs" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_SKIN" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 glaux.lib glu32.lib wsock32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\libs\pakd.lib ..\libs\jpeg6d.lib /nologo /stack:0x400000 /subsystem:console /profile /debug /machine:I386 /out:"d:/quake3/tools/q3map.exe"
# ADD LINK32 glaux.lib glu32.lib wsock32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\libs\pakd.lib ..\libs\jpeg6d.lib /nologo /stack:0x400000 /subsystem:console /profile /debug /machine:I386 /out:"d:/skin-script.exe"

!ENDIF 

# Begin Target

# Name "q3map - Win32 Release"
# Name "q3map - Win32 Debug"
# Name "q3map - Win32 Damage_skin_creator"
# Begin Source File

SOURCE=..\common\aselib.c
# End Source File
# Begin Source File

SOURCE=.\brush.c
# End Source File
# Begin Source File

SOURCE=.\brush_primit.c
# End Source File
# Begin Source File

SOURCE=.\bsp.c
# End Source File
# Begin Source File

SOURCE=..\common\bspfile.c
# End Source File
# Begin Source File

SOURCE=..\common\cmdlib.c
# End Source File
# Begin Source File

SOURCE=.\facebsp.c
# End Source File
# Begin Source File

SOURCE=.\fog.c
# End Source File
# Begin Source File

SOURCE=.\gldraw.c
# End Source File
# Begin Source File

SOURCE=.\glfile.c
# End Source File
# Begin Source File

SOURCE=..\common\imagelib.c
# End Source File
# Begin Source File

SOURCE=.\leakfile.c
# End Source File
# Begin Source File

SOURCE=.\light.c
# End Source File
# Begin Source File

SOURCE=.\light_trace.c
# End Source File
# Begin Source File

SOURCE=.\lightmaps.c
# End Source File
# Begin Source File

SOURCE=.\lightv.c
# End Source File
# Begin Source File

SOURCE=.\map.c
# End Source File
# Begin Source File

SOURCE=..\common\mathlib.c
# End Source File
# Begin Source File

SOURCE=.\mesh.c
# End Source File
# Begin Source File

SOURCE=.\misc_model.c
# End Source File
# Begin Source File

SOURCE=..\common\mutex.c
# End Source File
# Begin Source File

SOURCE=.\patch.c
# End Source File
# Begin Source File

SOURCE=..\common\polylib.c
# End Source File
# Begin Source File

SOURCE=.\portals.c
# End Source File
# Begin Source File

SOURCE=.\prtfile.c
# End Source File
# Begin Source File

SOURCE=..\common\scriplib.c
# End Source File
# Begin Source File

SOURCE=.\shaders.c
# End Source File
# Begin Source File

SOURCE=.\surface.c
# End Source File
# Begin Source File

SOURCE=.\terrain.c
# End Source File
# Begin Source File

SOURCE=..\common\threads.c
# End Source File
# Begin Source File

SOURCE=.\tjunction.c
# End Source File
# Begin Source File

SOURCE=.\tree.c
# End Source File
# Begin Source File

SOURCE=.\vis.c
# End Source File
# Begin Source File

SOURCE=.\visflow.c
# End Source File
# Begin Source File

SOURCE=.\writebsp.c
# End Source File
# End Target
# End Project
