@echo off
set CC=gcc

if not exist build mkdir build

echo Building object...
%CC% -O3 -fopenmp -Isrc -Iinclude -c src\main.c -o build\main.o

if errorlevel 1 goto end

echo Building EXE...
%CC% build\main.o -fopenmp -o build\ffjpeg.exe

if errorlevel 1 goto end

echo Building DLL...
%CC% -shared -O3 -fopenmp -Isrc -Iinclude src\main.c -o build\ffjpeg.dll

:end
echo Done.
pause
