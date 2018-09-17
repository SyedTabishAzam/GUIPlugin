start /B %STAGE_DIR%\GUIPlugin\x64\Release\StageBatchGui.exe
start /B stageSIM -stand-alone -C %STAGE_DIR%\GUIPlugin\config_mc.cfg -I %STAGE_DIR%\GUIPlugin\batch.xml
cd %STAGE_DIR%\GUIPlugin