:: OpenDDS configure script: configure 
@echo off
set PATH=%PATH%;%CD%\lib
@echo on
InitialPublisher.exe -DCPSConfigFile rtps.ini
