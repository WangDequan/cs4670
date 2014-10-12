# Microsoft Developer Studio Project File - Name="svm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=svm - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "svm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "svm.mak" CFG="svm - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "svm - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "svm - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/SVM", CAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "svm - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "D:\Courses\fltk-1.0.11" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib fltk.lib wsock32.lib glut32.lib /nologo /subsystem:console /machine:I386 /nodefaultlib:"libc" /libpath:"D:\Courses\fltk-1.0.11\lib"

!ELSEIF  "$(CFG)" == "svm - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "D:\Courses\fltk-1.0.11" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib fltkd.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libcd" /pdbtype:sept /libpath:"D:\Courses\fltk-1.0.11\lib"

!ENDIF 

# Begin Target

# Name "svm - Win32 Release"
# Name "svm - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\imagelib\Transform.cpp
# End Source File
# Begin Source File

SOURCE=.\src\imagelib\WarpImage.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\imagelib\Transform.h
# End Source File
# Begin Source File

SOURCE=.\src\imagelib\WarpImage.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "ImageLib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\ImageLib\FileIO.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImageLib\FileIO.h
# End Source File
# Begin Source File

SOURCE=.\src\ImageLib\Image.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImageLib\Image.h
# End Source File
# Begin Source File

SOURCE=.\src\ImageLib\Image.inl
# End Source File
# Begin Source File

SOURCE=.\src\ImageLib\RefCntMem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImageLib\RefCntMem.h
# End Source File
# End Group
# Begin Group "Demo Data"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ext_mdl.txt
# End Source File
# Begin Source File

SOURCE=.\exterior.tga
# End Source File
# Begin Source File

SOURCE=.\maya.tga
# End Source File
# Begin Source File

SOURCE=.\maya.txt
# End Source File
# Begin Source File

SOURCE=.\roof.gif
# End Source File
# Begin Source File

SOURCE=.\windows.gif
# End Source File
# End Group
# Begin Group "Project"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\HelpPageUI.cpp
# End Source File
# Begin Source File

SOURCE=.\src\HelpPageUI.h
# End Source File
# Begin Source File

SOURCE=.\src\ImgView.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImgView.h
# End Source File
# Begin Source File

SOURCE=.\src\jacob.cpp
# End Source File
# Begin Source File

SOURCE=.\src\jacob.h
# End Source File
# Begin Source File

SOURCE=.\src\mat.h
# End Source File
# Begin Source File

SOURCE=.\src\PriorityQueue.h
# End Source File
# Begin Source File

SOURCE=.\src\svm.h
# End Source File
# Begin Source File

SOURCE=.\src\svmAux.h
# End Source File
# Begin Source File

SOURCE=.\src\svmMain.cpp
# End Source File
# Begin Source File

SOURCE=.\src\svmmath.cpp
# End Source File
# Begin Source File

SOURCE=.\src\svmmath.h
# End Source File
# Begin Source File

SOURCE=.\src\svmUI.cpp
# End Source File
# Begin Source File

SOURCE=.\src\svmUI.h
# End Source File
# Begin Source File

SOURCE=.\src\vec.h
# End Source File
# End Group
# End Target
# End Project
