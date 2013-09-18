@ECHO OFF
REM --------------------------------------------------------------------------------------------
REM File    : build.cmd
REM
REM Abstract: 
REM
REM "Usage:    build.cmd"
REM
REM 
REM --------------------------------------------------------------------------------------------


:Chk_MSSdk
REM Need to set the CPU here
Set CPU=i386
IF "%~1" == "/SRV64" Set CPU=IA64
IF "%~1" == "/XP64"  Set CPU=AMD64
IF "%~1" == "/X64"   Set CPU=AMD64

REM Set the common env. variables
Goto Set_Common

:Chk_SetEnv
REM Verify that we have %MSSdk%\SetEnv.cmd
IF NOT EXIST "%MSSdk%\SetEnv.cmd" Goto ErrorBadPath

REM Check for the presence of MsVC
goto Chk_MsVC

:Chk_OS
REM Set OS/platform-specific variables
goto Set_OS

:Finish
Goto end


REM -------------------------------------------------------------------
REM Set common variables
:Set_Common
REM -------------------------------------------------------------------
Set MSSdk=C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2
Set Bkoffice=%MSSdk%\
Set Basemake=%MSSdk%\Include\BKOffice.Mak
Set INETSDK=%MSSdk%
Set MSSdk=%MSSdk%
Set Mstools=%MSSdk%

REM Default to DEBUG
goto Set_DEBUG
:Done_Debug

REM Are we building DEBUG or RETAIL
if "%~1" == "/RETAIL" goto Set_RETAIL
if "%~2" == "/RETAIL" goto Set_RETAIL
if "%~3" == "/RETAIL" goto Set_RETAIL
if "%~4" == "/RETAIL" goto Set_RETAIL
:Done_Retail

goto Chk_SetEnv


REM -------------------------------------------------------------------
REM Set OS/platform-specific variables
:Set_OS
REM -------------------------------------------------------------------

IF "x%OS%x" == "xWindows_NTx" Goto Set_WinNT

REM Check to make sure we aren't attempting to open a 64bit build window
if "%~1" == "/SRV64" goto ErrorUsage9x_IA64

Echo Setting SDK environment relative to %MSSdk%. 

REM check if the first parameter exists
if "%~1"== "" goto Set_WinXP32
if "%~1" == "/DEBUG"  goto Set_2000
if "%~1" == "/RETAIL" goto Set_2000
if "%~1" == "/2000"   goto Set_2000
if "%~1" == "/cc"     goto Set_2000
if "%~1" == "/SRV32"  goto Set_SRV32
if "%~1" == "/XP32"   goto Set_WinXP32

REM Usage not correct leave
goto ErrorUsage9x

REM -------------------------------------------------------------------
REM Set Windows 2000 specific variables
:Set_2000
REM -------------------------------------------------------------------
Echo Targeting Windows 2000 and IE 5.0 %DEBUGMSG%
Echo.
Set Lib=%MSSdk%\Lib;%Lib%
Set Include=%MSSdk%\Include;%Include%
Set Path=%MSSdk%\Bin;%path%
Set CPU=i386
Set TARGETOS=WINNT
Set APPVER=5.0
doskey > Nul
Goto Finish

REM -------------------------------------------------------------------
REM Set Windows XP32 specific variables
:Set_WinXP32
REM -------------------------------------------------------------------
Echo Targeting Windows WinXP32 %DEBUGMSG%
Echo.
Set Lib=%MSSdk%\Lib;%Lib%
Set Include=%MSSdk%\Include;%Include%
Set Path=%MSSdk%\Bin;%path%
Set CPU=i386
Set TARGETOS=WINNT
Set APPVER=5.01
doskey > Nul
Goto Finish


REM -------------------------------------------------------------------
REM Set Windows SRV32 specific variables
:Set_SRV32
REM -------------------------------------------------------------------
Echo Targeting Windows Server 2003 32bit %DEBUGMSG%
Echo.
Set Lib=%MSSdk%\Lib;%Lib%
Set Include=%MSSdk%\Include;%Include%
Set Path=%MSSdk%\Bin;%path%
Set CPU=i386
Set TARGETOS=WINNT
Set APPVER=5.02
doskey > Nul
Goto Finish


REM -------------------------------------------------------------------
REM Set Windows NT specific variables
:Set_WinNT
REM -------------------------------------------------------------------
IF /i "%PROCESSOR_ARCHITECTURE%" == "ALPHA" (Set CPU=ALPHA) else (Set CPU=i386)
IF "%~1" == "/SRV64" Set CPU=IA64
IF "%~1" == "/XP64"  Set CPU=AMD64
IF "%~1" == "/X64"   Set CPU=AMD64

REM Verify that we aren't building SRV64 on Windows NT Version 4.0
For /F "delims=;" %%i IN ('%windir%\system32\Cmd.exe /c Ver') DO (
    IF "%%i"=="Windows NT Version 4.0  " (
        IF "%CPU%" == "IA64" goto ErrorUsage9x_IA64
        )
    )

REM check if the first parameter exists
if "%~1"== "" goto Set2000_XP32
if "%~1" == "/DEBUG"  goto Set2000_2000
if "%~1" == "/RETAIL" goto Set2000_2000
if "%~1" == "/2000"   goto Set2000_2000
if "%~1" == "/cc"     goto Set2000_2000
if "%~1" == "/XP32"   goto Set2000_XP32
if "%~1" == "/XP64"   goto Set2000_XP64
if "%~1" == "/X64"    goto Set2000_X64
if "%~1" == "/SRV32"  goto Set2000_SRV32
if "%~1" == "/SRV64"  goto Set2000_SRV64

REM Usage not correct leave
goto ErrorUsage


REM -------------------------------------------------------------------
REM Set Windows 2000 specific variables
:Set2000_2000
REM -------------------------------------------------------------------
Echo Targeting Windows 2000 and IE 5.0 %DEBUGMSG%
Echo.
Set Lib=%MSSdk%\Lib;%Lib%
Set Include=%MSSdk%\Include;%Include%
Set Path=%MSSdk%\Bin;%MSSdk%\Bin\WinNT;%path%
Set APPVER=5.0
Set TARGETOS=WINNT
Title Microsoft Platform SDK Windows 2000 IE 5.0 %DEBUGMSG% Build Environment
Goto Finish


REM -------------------------------------------------------------------
REM Set Windows XP32 specific variables
:Set2000_XP32
REM -------------------------------------------------------------------
Echo Targeting Windows XP 32 %DEBUGMSG%
Echo.
Set Lib=%MSSdk%\Lib;%Lib%
Set Include=%MSSdk%\Include;%Include%
Set Path=%MSSdk%\Bin;%MSSdk%\Bin\WinNT;%path%
Set APPVER=5.01
Set TARGETOS=WINNT
Title Microsoft Platform SDK Windows XP 32-bit IE 5.5 %DEBUGMSG% Build Environment
Goto Finish


REM -------------------------------------------------------------------
REM Set Windows XP 64 specific variables
:Set2000_XP64
REM -------------------------------------------------------------------
Echo Targeting Windows XP X64 %DEBUGMSG%
Echo.
Set Lib=%MSSdk%\Lib\AMD64;%MSSdk%\Lib\AMD64\atlmfc;
Set Include=%MSSdk%\Include;%MSSdk%\Include\crt;%MSSdk%\Include\crt\sys;%MSSdk%\Include\mfc;%MSSdk%\Include\atl
Set Path=%MSSdk%\Bin\Win64\x86\AMD64;%MSSdk%\Bin;%MSSdk%\Bin\WinNT;%path%
Set APPVER=5.02
Set TARGETOS=WINNT
Title Microsoft Platform SDK XP X64-bit IE 6.0 %DEBUGMSG% Build Environment
Goto Finish

REM -------------------------------------------------------------------
REM Set Windows Server 2003 32bit specific variables
:Set2000_SRV32
REM -------------------------------------------------------------------
Echo Targeting Windows Server 2003 32bit %DEBUGMSG%
Echo.
Set Lib=%MSSdk%\Lib;%Lib%
Set Include=%MSSdk%\Include;%Include%
Set Path=%MSSdk%\Bin;%MSSdk%\Bin\WinNT;%path%
Set APPVER=5.02
Set TARGETOS=WINNT
Title Microsoft Platform SDK Windows Server 2003 32-bit IE 6.0 %DEBUGMSG% Build Environment
Goto Finish


REM -------------------------------------------------------------------
REM Set Windows Server 2003 64-bit specific variables
:Set2000_SRV64
REM -------------------------------------------------------------------
Echo Targeting Windows Server 2003 IA64-bit %DEBUGMSG%
Echo.
Set Lib=%MSSdk%\Lib\IA64;%MSSdk%\Lib\IA64\mfc;
Set Include=%MSSdk%\Include\crt;%MSSdk%\Include\crt\sys;%MSSdk%\Include\mfc;%MSSdk%\Include\atl;%MSSdk%\Include;
Set Path=%MSSdk%\Bin\Win64\IA64;%MSSdk%\Bin\Win64;%MSSdk%\Bin;%MSSdk%\Bin\WinNT;%path%
Set APPVER=5.02
Set TARGETOS=WINNT
Title Microsoft Platform SDK Windows Server 2003 IA64-bit IE 6.0 %DEBUGMSG% Build Environment
Goto Finish

REM -------------------------------------------------------------------
REM Set Windows X64 specific variables
:Set2000_X64
REM -------------------------------------------------------------------
Echo Targeting Windows Server 2003 X64 %DEBUGMSG%
Echo.
Set Lib=%MSSdk%\Lib\AMD64;%MSSdk%\Lib\AMD64\atlmfc;
Set Include=%MSSdk%\Include;%MSSdk%\Include\crt;%MSSdk%\Include\crt\sys;%MSSdk%\Include\mfc;%MSSdk%\Include\atl
Set Path=%MSSdk%\Bin\Win64\x86\AMD64;%MSSdk%\Bin;%MSSdk%\Bin\WinNT;%path%
Set APPVER=5.02
Set TARGETOS=WINNT
Title Microsoft Platform SDK Server 2003 X64-bit IE 6.0 %DEBUGMSG% Build Environment
Goto Finish

REM -------------------------------------------------------------------
:Chk_MsVC
REM We want to make sure MSVCDir and DevEnvDir are set to folders that exist
REM We will try to detect VC using its common environment variables.
REM Else, we will try to detect a version of VC through more robust means
REM -------------------------------------------------------------------
REM: Note - We will not be searching for VC on 64-bit platforms, for now
if "%CPU%" == "IA64" (
  Set MSVCVer=Win64
  goto Chk_OS
)
if "%CPU%" == "AMD64" (
  Set MSVCVer=Win64
  goto Chk_OS
)

REM: Begin search for VS
goto Check_MsVcDir_Exists


REM -------------------------------------------------------------------
:Check_MsVcDir_Exists
REM -------------------------------------------------------------------
REM Does the MsVcDir environment variable exist?
REM If not, go try DevEnvDir
if "x%MSVCDir%x" == "xx" Goto Check_DevEnvDir_Exists

REM Yes, check if the MsVcDir path exists
REM If the path MSVCDir does NOT Exist, the environmental integrity is compromised, so
REM try to detect a version of VC through more robust means
IF NOT EXIST "%MSVCDir%\." goto DetectVC

REM Does the DevEnvDir environment variable exist?
IF "x%DevEnvDir%x" == "xx" (
  REM No. If the effective DevEnvDir does NOT Exist, the environmental integrity is compromised, so
  REM try to detect a version of VC through more robust means
  IF NOT EXIST "%MSVCDir%\..\Common7\IDE\." goto DetectVC
  REM Derive DevEnvDir environment variable from MsVcDir environement variable
  Set DevEnvDir=%MSVCDir%\..\Common7\IDE
)

REM We assume VC7 is present
goto VC7Detected


REM -------------------------------------------------------------------
:Check_DevEnvDir_Exists
REM -------------------------------------------------------------------
REM Does the DevEnvDir environment variable exist?
REM If not, try to detect a version of VC through more robust means
if "x%DevEnvDir%x" == "xx" Goto DetectVC

REM Yes, check if the DevEnvDir path exists
REM If the path DevEnvDir does NOT Exist, the environmental integrity is compromised, so
REM try to detect a version of VC through more robust means
IF NOT EXIST "%DevEnvDir%\." goto DetectVC

REM Does the MsVcDir environment variable exist?
IF "x%MsVcDir%x" == "xx" (
  REM No. If the effective MsVcDir does NOT Exist, the environmental integrity is compromised, so
  REM try to detect a version of VC through more robust means
  IF NOT EXIST "%DevEnvDir%\..\..\VC7\." goto DetectVC
  REM Derive MsVcDir environment variable from DevEnvDir environement variable
  Set MsVcDir=%DevEnvDir%\..\..\VC7
)

REM We assume VC7 is present
goto VC7Detected


REM -------------------------------------------------------------------
:VC7Detected
REM -------------------------------------------------------------------
Set MSVCVer=7.0
goto Chk_OS


REM -------------------------------------------------------------------
REM Set RETAIL
REM -------------------------------------------------------------------
:Set_RETAIL
Set NODEBUG=1
Set DEBUGMSG=RETAIL
goto Done_Retail


REM -------------------------------------------------------------------
REM Set DEBUG
REM -------------------------------------------------------------------
:Set_DEBUG
Set NODEBUG=
Set DEBUGMSG=DEBUG
goto Done_Debug


REM -------------------------------------------------------------------
:DetectVC
REM -------------------------------------------------------------------
Echo.
Echo Attempting to detect a Microsoft Visual Studio installation
Echo.
Call "%MSSDK%\Setup\VCDetect.exe" "%TEMP%\VCInit.bat"
IF NOT EXIST "%TEMP%\VCInit.bat" goto ErrorVCEnv
Call "%TEMP%\VCInit.bat"
Echo.
goto Chk_OS


REM -------------------------------------------------------------------
:ErrorVCEnv
REM -------------------------------------------------------------------
Echo.
Echo Warning: The environmental variables MSDevDir and MSVCDir
Echo  were not found to exist. Check your Lib, Include and PATH to
Echo  verify that the SDK Lib, Include, and Bin directories precede
Echo  the compiler directories in the environment.
Echo.
Echo Note: Microsoft Visual Studio provides VCVARS32.BAT to Set them.
Echo  You must run VCVARS32.BAT first and then run SetEnv.cmd.
Echo.
Echo Current Settings:
Echo Lib=%Lib%
Echo Include=%Include%
Echo PATH=%PATH%
Echo.
Goto Finish

REM -------------------------------------------------------------------
:ErrorBadPath
REM -------------------------------------------------------------------
Echo.
Echo Error: The file "%MSSdk%\SetEnv.cmd" does not appear to exist, or
Echo        an existing MSSdk env. variable does not match the expected
Echo        value encoded in this batch file by Platform SDK Setup.
Echo        Please check the path and an existing MSSdk variable
Echo        for correctness.
Echo.
Goto Finish

REM -------------------------------------------------------------------
:ErrorUsage9x_IA64
REM -------------------------------------------------------------------
echo.
echo Building Samples for the IA64 Platform is not currently supported on 
echo Windows 95, 98, Millennium or NT 4.0 Platforms.
echo.

REM -------------------------------------------------------------------
:ErrorUsage9x
REM -------------------------------------------------------------------
echo  "Usage Setenv [/2000 | /XP32 | /SRV32] [/DEBUG | /RETAIL]"
echo.
echo                 /2000   - target Windows 2000 and IE 5.0
echo                 /XP32   - target Windows XP 32 (default)
echo                 /SRV32  - target Windows Server 2003
echo                 /DEBUG  - set the environment to DEBUG
echo                 /RETAIL - set the environment to RETAIL
Goto Finish

REM -------------------------------------------------------------------
:ErrorUsage
REM -------------------------------------------------------------------
echo  "Usage Setenv [/2000 | /XP32 | /XP64 | /SRV32 | /SRV64 | /X64] [/DEBUG | /RETAIL]"
echo.
echo                 /2000   - target Windows 2000 and IE 5.0
echo                 /XP32   - target Windows XP 32 (default)
echo                 /XP64   - target Windows XP 64
echo                 /SRV32  - target Windows Server 2003 32 bit
echo                 /SRV64  - target Windows Server 2003 64 bit
echo                 /X64    - target Windows for the X64 bit prerelease content
echo                 /DEBUG  - set the environment to DEBUG
echo                 /RETAIL - set the environment to RETAIL
Goto Finish


REM -------------------------------------------------------------------
:end
IF "x%OS%x" == "xWindows_NTx" Goto end_NT
echo *** WARNING ***
echo You are currently building on a Windows 9x based platform.  Most samples have 
echo NMAKE create a destination directory for created objects and executables.  
echo There is a known issue with the OS where NMAKE fails to create this destination
echo directory when the current directory is several directories deep.  To fix this 
echo problem, you must create the destination directory by hand from the command 
echo line before calling NMAKE. 
echo.

:end_NT
Set DEBUGMSG=
REM -------------------------------------------------------------------
