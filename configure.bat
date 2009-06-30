@echo off

type NUL > Makefile.local
echo.

if [%1] == [mingw] goto mingw
goto help

:help
	echo Usage: configure (platform)
	echo Valid platforms: mingw
	goto end

:mingw
	echo Configuring for MinGW...
	echo MAKEFILE_INC = Makefile.mgw >> Makefile.local
	goto okay

:okay
	echo include Makefile.all >> Makefile.local
	echo Done.
	type makeflag.txt
	set ERRORLEVEL=0
	goto end

:end
