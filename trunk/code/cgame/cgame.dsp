# Microsoft Developer Studio Project File - Name="cgame" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=cgame - Win32 Profile TA
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cgame.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cgame.mak" CFG="cgame - Win32 Profile TA"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cgame - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cgame - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cgame - Win32 Release TA" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cgame - Win32 Debug TA" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cgame - Win32 Profile TA" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cgame - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ole32.lib strmbase.lib /nologo /base:"0x30000000" /subsystem:windows /dll /machine:I386 /out:"c:\quake3\western\cgamex86.dll"
# SUBTRACT LINK32 /map /debug

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "VM_CGAME" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ole32.lib strmbase.lib /nologo /base:"0x30000000" /subsystem:windows /dll /map /debug /machine:I386 /out:"d:\quake3\westernq3\cgamex86.dll"
# SUBTRACT LINK32 /profile /nodefaultlib

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "cgame___Win32_Release_TA"
# PROP BASE Intermediate_Dir "cgame___Win32_Release_TA"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_TA"
# PROP Intermediate_Dir "Release_TA"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /Ob2 /D "VM_CGAME" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "MISSIONPACK" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /base:"0x30000000" /subsystem:windows /dll /map /machine:I386 /out:"../Release/cgamex86.dll"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 /nologo /base:"0x30000000" /subsystem:windows /dll /profile /debug /machine:I386 /out:"../Release_TA/cgamex86.dll"
# SUBTRACT LINK32 /map

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "cgame___Win32_Debug_TA"
# PROP BASE Intermediate_Dir "cgame___Win32_Debug_TA"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_TA"
# PROP Intermediate_Dir "Debug_TA"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /GX /ZI /Od /D "VM_CGAME" /D "WIN32" /D "_WINDOWS" /D "MISSIONPACK" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /base:"0x30000000" /subsystem:windows /dll /map /debug /machine:I386 /out:"..\Debug/cgamex86.dll"
# SUBTRACT BASE LINK32 /profile /nodefaultlib
# ADD LINK32 /nologo /base:"0x30000000" /subsystem:windows /dll /debug /machine:I386 /def:".\cgame.def" /out:"t:\quake3\westernq3\cgamex86.dll"
# SUBTRACT LINK32 /profile /map

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "cgame___Win32_Profile_TA"
# PROP BASE Intermediate_Dir "cgame___Win32_Profile_TA"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Profile_TA"
# PROP Intermediate_Dir "Profile_TA"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MTd /W3 /Gm /GX /ZI /Od /D "VM_CGAME" /D "WIN32" /D "_WINDOWS" /D "MISSIONPACK" /FR /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /GX /Zd /Od /D "VM_CGAME" /D "WIN32" /D "_WINDOWS" /D "MISSIONPACK" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /base:"0x30000000" /subsystem:windows /dll /map:"t:\quake3\westernq3\cgamex86.map" /debug /machine:I386 /def:".\cgame.def" /out:"t:\quake3\westernq3\cgamex86.dll"
# SUBTRACT BASE LINK32 /profile
# ADD LINK32 /nologo /base:"0x30000000" /subsystem:windows /dll /profile /map:"t:\quake3\westernq3\cgamex86.map" /debug /machine:I386 /def:".\cgame.def" /out:"t:\quake3\westernq3\cgamex86.dll"

!ENDIF 

# Begin Target

# Name "cgame - Win32 Release"
# Name "cgame - Win32 Debug"
# Name "cgame - Win32 Release TA"
# Name "cgame - Win32 Debug TA"
# Name "cgame - Win32 Profile TA"
# Begin Group "Source Files"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=..\game\bg_lib.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\bg_misc.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\bg_pmove.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\bg_slidemove.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_consolecmds.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_draw.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_drawtools.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_effects.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_ents.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_event.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_info.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_localents.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_main.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_marks.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_newDraw.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# PROP Exclude_From_Build 1
# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_players.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_playerstate.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_predict.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_scoreboard.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_servercmds.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_snapshot.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_syscalls.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_unlagged.c

!IF  "$(CFG)" == "cgame - Win32 Release"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_view.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_weapons.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cg_wq_utils.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\q_math.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\q_shared.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ui\ui_shared.c

!IF  "$(CFG)" == "cgame - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# ADD CPP /D "_CONFIGCP_Win32_20Debug" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

# ADD CPP /D "_CONFIGCP_Win32_20Release_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# ADD BASE CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"
# ADD CPP /D "_CONFIGCP_Win32_20Debug_20TA" /D "_PRJCP_cgame_2Evprj"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\game\bg_public.h
# End Source File
# Begin Source File

SOURCE=.\cg_local.h
# End Source File
# Begin Source File

SOURCE=.\cg_public.h
# End Source File
# Begin Source File

SOURCE=..\game\q_shared.h
# End Source File
# Begin Source File

SOURCE=..\game\surfaceflags.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\cgame.def

!IF  "$(CFG)" == "cgame - Win32 Release"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

!ELSEIF  "$(CFG)" == "cgame - Win32 Release TA"

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug TA"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "cgame - Win32 Profile TA"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Target
# End Project
