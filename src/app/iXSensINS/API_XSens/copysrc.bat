@echo off

setlocal

set sourcepath=%1
set publicpath=%2

call:removeDir %publicpath%\xstypes
call:removeDir %publicpath%\xcommunication
call:removeDir %publicpath%\include
call:removeDir %publicpath%\winusb

xcopy /y /s /q %sourcepath%\* %publicpath%

::get grateheader.exe from svn if not yet
::if exist ..\grateheader\bin\grateheader.exe (
:: echo Using existing grateheader
::) else (
:: echo grateheader.exe is missing, getting it from svn
:: svn co https://svn.xsens.com/softwareteam/bintools/grateheader/trunk/bin ../grateheader/bin
:: svn co https://svn.xsens.com/softwareteam/bintools/grateheader/trunk ../grateheader --depth empty
::)

::@PING 1.1.1.1 -n 1 -w 50 2>NUL | FIND "TTL=" >NUL
::..\grateheader\bin\grateheader.exe %publicpath% NOT_FOR_PUBLIC_RELEASE
endlocal
goto:eof

:removeDir
	if exist %~1 (
		move /y %~1 %~1.1
		rmdir /s /q %~1.1
	)
goto:eof


