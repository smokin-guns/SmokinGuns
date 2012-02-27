@echo off
echo Initializing VC9 environment variables...
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
echo Building Debug
vcbuild "sg.sln" "Debug|Win32"
echo Building Release
vcbuild "sg.sln" "Release|Win32"
echo Building Release SSE
vcbuild "sg.sln" "Release SSE|Win32"
echo Building Release SSE2
vcbuild "sg.sln" "Release SSE2|Win32"
echo Build finished.
pause
IF ERRORLEVEL 1 EXIT 1

echo Traversing Backwards.
cd ..\