@echo off

set SOURCEFILE=followmouse.as

REM
REM to compile .as files (from .as to .swf) you need the "mxmlc" compiler
REM from the free Adobe Open Source Flex SDK!
REM
REM
REM USAGE/INSTALLATION STEPS:
REM
REM 1. download the Flex SDK from here: (use the latest production release)
REM
REM      http://opensource.adobe.com/flexsdk/
REM
REM 2. extract the zip package anywhere
REM
REM 3. edit the line below and set there the path where you
REM    have extracted the Flex SDK package:
REM

set FLEXSDKPATH=C:\TEMP\flex_sdk_4.1.0.16032_mpl


IF EXIST %FLEXSDKPATH%\bin\mxmlc GOTO HAVEFLEXSDK
REM
REM the mxmlc compiler isn't there, show this message:
REM
echo.
echo NOTE - YOU NEED TO READ AND EDIT THIS FILE BEFORE USAGE!!!
echo (open this file in any texteditor)
echo.
GOTO END


:HAVEFLEXSDK
%FLEXSDKPATH%\bin\mxmlc -use-network=false -compiler.optimize=true -static-link-runtime-shared-libraries=true %SOURCEFILE%
GOTO END

:END
pause
