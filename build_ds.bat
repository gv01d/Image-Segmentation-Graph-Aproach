@echo off
REM Batch script to build dijkstraSolution.cpp with tinyfiledialogs using g++.exe

REM Set the path to g++.exe
set "GPP=C:\msys64\mingw64\bin\g++.exe"

REM Set the source file and output executable
set "DIR=%~dp0"
set "BASE=dijkstraSolution"
set "FILE=%DIR%%BASE%.cpp"
set "EXE=%DIR%%BASE%.exe"

echo Building %FILE% with %GPP%...

REM Compile the file
"%GPP%" -fdiagnostics-color=always -g "%DIR%tinyfiledialogs.c" "%FILE%" -o "%EXE%" -lole32 -lcomdlg32 -luuid -lgdi32

REM Check for errors
if errorlevel 1 (
    echo Build failed.
    exit /b %errorlevel%
) else (
    echo Build succeeded: %EXE%
)
REM Run the executable if build succeeded
"%EXE%"
