start /B %STAGE_DIR%\samples\monte_carlo\Debug\StageBatchGuid.exe
start /B stageSIM -stand-alone -C %STAGE_DIR%\samples\monte_carlo\config_mc.cfg -I %STAGE_DIR%\samples\monte_carlo\batch.xml

