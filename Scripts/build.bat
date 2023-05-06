cd c:\decompile\SAMEBOY
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
set lib=%lib%;C:\decompile\SDL2\lib\x86
set include=%include%;C:\decompile\SDL2\include
set path=%path%;C:\decompile\rgbds;C:\msys64\usr\bin
make
pause