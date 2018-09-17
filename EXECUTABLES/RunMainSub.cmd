:: OpenDDS configure script: configure 
@echo off
set PATH=%PATH%;%CD%\lib
@echo on
CommandSubscriber.exe -DCPSConfigFile rtps.ini
