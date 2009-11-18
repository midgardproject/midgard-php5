# Microsoft Developer Studio Project File - Name="php_midgard" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=php_midgard - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "php_midgard.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "php_midgard.mak" CFG="php_midgard - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "php_midgard - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "php_midgard - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "php_midgard - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\tmp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PHP_MIDGARD_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\lib" /I "..\..\build\expat\lib" /I "..\..\build\glib" /I "..\..\build\glib\glib" /I "..\..\build\iconv\include" /I "..\..\..\..\applications\opensa\apache\src\include" /I "..\..\..\..\applications\opensa\apache\src\os\win32" /I "..\..\..\..\applications\modules\php" /I "..\..\..\..\applications\modules\php\Zend" /I "..\..\..\..\applications\modules\php\TSRM" /I "..\..\..\..\applications\modules\php\main" /D "__G_WIN32_H__" /D "HAVE_MIDGARD_SITEGROUPS" /D "APACHE_READDIR_H" /D "APACHE_OS_H" /D "PHP_EXPORTS" /D "COMPILE_DL_MIDGARD" /D ZTS=0 /D "HAVE_MIDGARD" /D ZEND_DEBUG=0 /D "ZEND_WIN32" /D "PHP_WIN32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PHP_MIDGARD_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib php4ts.lib /nologo /dll /machine:I386 /libpath:"..\..\lib\win32\Release" /libpath:"..\..\build\glib\glib" /libpath:"..\..\..\..\applications\modules\php\Release_TS"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "php_midgard - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\tmp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PHP_MIDGARD_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\lib" /I "..\..\build\expat\lib" /I "..\..\build\glib" /I "..\..\build\glib\glib" /I "..\..\build\iconv\include" /I "..\..\..\..\applications\opensa\apache\src\include" /I "..\..\..\..\applications\opensa\apache\src\os\win32" /I "..\..\..\..\applications\modules\php" /I "..\..\..\..\applications\modules\php\main" /I "..\..\..\..\applications\modules\php\Zend" /I "..\..\..\..\applications\modules\php\TSRM" /D "__G_WIN32_H__" /D "HAVE_MIDGARD_SITEGROUPS" /D "APACHE_READDIR_H" /D "APACHE_OS_H" /D "PHP_EXPORTS" /D "COMPILE_DL_MIDGARD" /D ZTS=0 /D "HAVE_MIDGARD" /D ZEND_DEBUG=1 /D "ZEND_WIN32" /D "PHP_WIN32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PHP_MIDGARD_EXPORTS" /YX /FD /I /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib php4ts_debug.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\..\lib\win32\Debug" /libpath:"..\..\build\glib\glib" /libpath:"..\..\..\..\applications\modules\php\Debug_TS"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "php_midgard - Win32 Release"
# Name "php_midgard - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\article.c
# End Source File
# Begin Source File

SOURCE=..\attachment.c
# End Source File
# Begin Source File

SOURCE=..\calendar.c
# End Source File
# Begin Source File

SOURCE=..\element.c
# End Source File
# Begin Source File

SOURCE=..\event.c
# End Source File
# Begin Source File

SOURCE=..\eventmember.c
# End Source File
# Begin Source File

SOURCE=..\file.c
# End Source File
# Begin Source File

SOURCE=..\group.c
# End Source File
# Begin Source File

SOURCE=..\host.c
# End Source File
# Begin Source File

SOURCE=..\image.c
# End Source File
# Begin Source File

SOURCE=..\mail.c
# End Source File
# Begin Source File

SOURCE=..\member.c
# End Source File
# Begin Source File

SOURCE=..\mgd_errno.c
# End Source File
# Begin Source File

SOURCE=..\midgard.c
# End Source File
# Begin Source File

SOURCE=..\oop.c
# End Source File
# Begin Source File

SOURCE=..\page.c
# End Source File
# Begin Source File

SOURCE=..\pageelement.c
# End Source File
# Begin Source File

SOURCE=..\pagelink.c
# End Source File
# Begin Source File

SOURCE=..\parameter.c
# End Source File
# Begin Source File

SOURCE=..\person.c
# End Source File
# Begin Source File

SOURCE=..\preferences.c
# End Source File
# Begin Source File

SOURCE=..\preparse.c
# End Source File
# Begin Source File

SOURCE=..\preparser.c
# End Source File
# Begin Source File

SOURCE=..\sitegroup.c
# End Source File
# Begin Source File

SOURCE=..\snippet.c
# End Source File
# Begin Source File

SOURCE=..\snippetdir.c
# End Source File
# Begin Source File

SOURCE=..\style.c
# End Source File
# Begin Source File

SOURCE=..\topic.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\mgd_access.h
# End Source File
# Begin Source File

SOURCE=..\mgd_article.h
# End Source File
# Begin Source File

SOURCE=..\mgd_attachment.h
# End Source File
# Begin Source File

SOURCE=..\mgd_calendar.h
# End Source File
# Begin Source File

SOURCE=..\mgd_element.h
# End Source File
# Begin Source File

SOURCE=..\mgd_errno.h
# End Source File
# Begin Source File

SOURCE=..\mgd_event.h
# End Source File
# Begin Source File

SOURCE=..\mgd_eventmember.h
# End Source File
# Begin Source File

SOURCE=..\mgd_file.h
# End Source File
# Begin Source File

SOURCE=..\mgd_group.h
# End Source File
# Begin Source File

SOURCE=..\mgd_host.h
# End Source File
# Begin Source File

SOURCE=..\mgd_image.h
# End Source File
# Begin Source File

SOURCE=..\mgd_internal.h
# End Source File
# Begin Source File

SOURCE=..\mgd_mail.h
# End Source File
# Begin Source File

SOURCE=..\mgd_member.h
# End Source File
# Begin Source File

SOURCE=..\mgd_oop.h
# End Source File
# Begin Source File

SOURCE=..\mgd_page.h
# End Source File
# Begin Source File

SOURCE=..\mgd_pageelement.h
# End Source File
# Begin Source File

SOURCE=..\mgd_pagelink.h
# End Source File
# Begin Source File

SOURCE=..\mgd_person.h
# End Source File
# Begin Source File

SOURCE=..\mgd_preferences.h
# End Source File
# Begin Source File

SOURCE=..\mgd_preparse.h
# End Source File
# Begin Source File

SOURCE=..\mgd_preparser.h
# End Source File
# Begin Source File

SOURCE=..\mgd_sitegroup.h
# End Source File
# Begin Source File

SOURCE=..\mgd_snippet.h
# End Source File
# Begin Source File

SOURCE=..\mgd_snippetdir.h
# End Source File
# Begin Source File

SOURCE=..\mgd_style.h
# End Source File
# Begin Source File

SOURCE=..\mgd_topic.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\php_midgard.rc
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# End Target
# End Project
