# Microsoft Developer Studio Project File - Name="ui" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ui - Win32 Debug TA
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ui.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ui.mak" CFG="ui - Win32 Debug TA"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ui - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ui - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ui - Win32 Release TA" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ui - Win32 Debug TA" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ui - Win32 vector" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/MissionPack/code/ui", AOBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ui - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "UI_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "UI_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 /nologo /base:"0x40000000" /dll /map /machine:I386 /out:"../Release/uix86_new.dll"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ui___Win32_Debug"
# PROP BASE Intermediate_Dir "ui___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "UI_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "UI_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /base:"0x40000000" /dll /pdb:"../Debug/ui.pdb" /map /debug /machine:I386 /out:"../Debug/uix86_new.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ui___Win32_Release_TA"
# PROP BASE Intermediate_Dir "ui___Win32_Release_TA"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_TA"
# PROP Intermediate_Dir "Release_TA"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "UI_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "UI_EXPORTS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /base:"0x40000000" /dll /map /machine:I386 /out:"../Release/uix86.dll"
# ADD LINK32 /nologo /base:"0x40000000" /dll /machine:I386 /out:"../Release_TA/uix86.dll"
# SUBTRACT LINK32 /profile /map

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ui___Win32_Debug_TA"
# PROP BASE Intermediate_Dir "ui___Win32_Debug_TA"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_TA"
# PROP Intermediate_Dir "Debug_TA"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "UI_EXPORTS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "UI_EXPORTS" /D "MISSIONPACK" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /base:"0x40000000" /dll /pdb:"../Debug/ui.pdb" /map /debug /machine:I386 /out:"../Debug/uix86_new.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 /nologo /base:"0x40000000" /dll /debug /machine:I386 /out:"t:\quake3\westernq3\uix86.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /map

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ui___Win32_vector"
# PROP BASE Intermediate_Dir "ui___Win32_vector"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ui___Win32_vector"
# PROP Intermediate_Dir "ui___Win32_vector"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "UI_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "UI_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /base:"0x40000000" /dll /map /machine:I386 /out:"../Release_TA/uix86.dll"
# ADD LINK32 /nologo /base:"0x40000000" /dll /map /machine:I386 /out:"../Release_TA/uix86.dll"

!ENDIF 

# Begin Target

# Name "ui - Win32 Release"
# Name "ui - Win32 Debug"
# Name "ui - Win32 Release TA"
# Name "ui - Win32 Debug TA"
# Name "ui - Win32 vector"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\game\bg_lib.c

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\bg_misc.c

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\q_math.c

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\q_shared.c

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui.def

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_atoms.c

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_gameinfo.c
# End Source File
# Begin Source File

SOURCE=.\ui_main.c

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_players.c

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_shared.c

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_syscalls.c

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_util.c

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\game\bg_public.h

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\keycodes.h

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ui\menudef.h

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\q_shared.h

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\surfaceflags.h

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\cgame\tr_types.h

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_local.h

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_public.h

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ui_shared.h

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

!ELSEIF  "$(CFG)" == "ui - Win32 Release TA"

!ELSEIF  "$(CFG)" == "ui - Win32 Debug TA"

!ELSEIF  "$(CFG)" == "ui - Win32 vector"

!ENDIF 

# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
