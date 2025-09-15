@echo off
set BUILD=debug
if not "%1"=="" set BUILD=%1

if /I "%BUILD%"=="debug" (
    mkdir build\win-x64-debug 2>nul
    cl.exe /Zi /Iinclude .\source\profiler.cpp /LD /std:c++20 ^
        /Fo"build\win-x64-debug\profiler.obj" ^
        /Fe"build\win-x64-debug\profiler.dll" ^
        /Fd"build\win-x64-debug\vc140.pdb"

) else if /I "%BUILD%"=="release" (
    mkdir build\win-x64-release 2>nul
    cl.exe /Iinclude .\source\profiler.cpp /LD /std:c++20 /O2 /DNDEBUG ^
        /Fo"build\win-x64-release\profiler.obj" ^
        /Fe"build\win-x64-release\profiler.dll"

) else (
    echo Unknown build type: %BUILD%
    exit /b 1
)