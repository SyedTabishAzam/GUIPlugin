start /B %STAGE_DIR%\samples\monte_carlo\Release\StageBatchCmd.exe  %STAGE_DIR%\samples\monte_carlo\stage.commands
start /B stageSIM -stand-alone -C %STAGE_DIR%\samples\monte_carlo\config_mc.cfg -I %STAGE_DIR%\samples\monte_carlo\batch.xml
