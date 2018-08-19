@echo OFF

REM Not to call this setup command file more than one time
if DEFINED STAGE_DEV_SETUP_WINDOWS (
call start_batch_gui.cmd
goto _END
)

REM *** STAGE where the Developer's user can READ and WRITE. ***
set STAGE_DEV_HOME=%~dp0

REM ***** STAGE Default Setup *****
PUSHD .
cd ..
call setup.cmd
POPD

if not exist "%STAGE_DIR%" (
  echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  echo ERROR STAGE installation folder [%STAGE_DIR%]  not found. 
  echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  pause
  exit 1
)

if not exist "%STAGE_DIR%\lib" (
  echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  echo ERROR STAGE installation folder [%STAGE_DIR%]  does not have the Dev Kit. 
  echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  pause
  exit 1
)


REM *** VISUAL STUDIO settings ***

if "%STAGE_VISUALSTUDIO_VERSION%" == "vc10" (
   REM ***** The following is for Visual Studio 2010 *****
   call "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" amd64
)
if "%STAGE_VISUALSTUDIO_VERSION%" == "vc12" (
   REM ***** The following is for Visual Studio 2012 *****
   call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" amd64
)

REM **** STAGE Optional or overwritten Env. vars *****************

REM **** STAGE Gen Folder ****
rem echo setting STAGE_GEN_DIR to %STAGE_DEV_HOME%\gen
rem set STAGE_GEN_DIR=%STAGE_DEV_HOME%\gen

if not exist "%STAGE_GEN_DIR%" (
  echo ERROR STAGE_GEN_DIR=%STAGE_GEN_DIR% does not exist
  pause 
  exit 1
)

REM *** User Plug-ins ***
set STAGE_SM_USER_PLUGINS_DIR=%STAGE_DIR%\userplugins\sm
set STAGE_SIM_USER_PLUGINS_DIR=%STAGE_DIR%\userplugins\sim

REM *** Config ****
rem set STAGE_CONFIG_FILE=%STAGE_DEV_HOME%\config\config.cfg
rem echo setting default config through STAGE_CONFIG_FILE to %STAGE_CONFIG_FILE%

set STAGE_DEV_SETUP_WINDOWS=1

echo STAGE Developing Setup Successful
call start_batch_gui.cmd


if "%1" == "" (
	
   if "%SETUP_NOSHELL%" == "1" goto _END
   
   cd /D %STAGE_DEV_HOME%   
   REM Set title and keep shell cmd open
   title "STAGE %STAGE_VERSION% Devel Shell %STAGE_PROCESSOR_ARCHITECTURE% %STAGE_VISUALSTUDIO_VERSION%"
   %comspec% /k

   goto _END
)

start /b %*
exit /B 0



:_END

