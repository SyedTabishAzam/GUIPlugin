@echo off
for /f "skip=1 delims=" %%a in ('wmic logicaldisk get caption') do (
	@echo off
    cd %%a
    @echo off
    cd \
    @echo off
    for  /f "delims=" %%a in ('dir /b /s /a:d "%%a" ^|findstr /e /i "\OpenDDS"') do (
    
    @echo off
	 IF EXIST "%%~a\setenv.cmd" (call %%~a\setenv.cmd %%~a )
	  
	)
)


