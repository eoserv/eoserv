@echo off

type NUL > Makefile
echo.

if [%1] == [mingw] goto mingw
goto help

:help
	echo Usage: configure (platform)
	echo Valid platforms: mingw
	goto end

:mingw
	echo Configuring for MinGW...
	echo MAKEFILE_INC = Makefile.mgw >> Makefile
	goto okay

:okay
	echo include Makefile.all >> Makefile
	echo Done.
	type makeflag.txt
	set ERRORLEVEL=0
	goto end

:end
