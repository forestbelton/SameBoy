@echo off
set oldlib=%lib%
set oldinclude=%include%
set oldpath=%path%

if /I "%1" EQU "" goto fork
if /i "%1" EQU "run" goto run

echo "unknown argument: '%1'"
goto :end

:fork
(call %0 run)
goto :end

:run
cd c:\decompile\SAMEBOY
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
set lib=%lib%;C:\decompile\SDL2\lib\x86
set include=%include%;C:\decompile\SDL2\include
set path=%path%;C:\decompile\rgbds;C:\msys64\usr\bin
make
pause
goto :real_end

:end
set lib=%oldlib%
set include=%oldinclude%
set path=%oldpath%

:real_end