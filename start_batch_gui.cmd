start /B %STAGE_DIR%\GUIPlugin\Release\StageBatchGui.exe
start /B stageSIM -stand-alone -C %STAGE_DIR%\GUIPlugin\config_mc.cfg -I %STAGE_DIR%\GUIPlugin\batch.xml
