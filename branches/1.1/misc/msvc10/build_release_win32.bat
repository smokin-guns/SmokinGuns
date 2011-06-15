@echo off
echo Initializing VC10 environment variables...
call "C:\Program Files\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
rem call "C:\Program Files\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"
rem echo Building Debug Win32
rem msbuild "sg.sln" /p:Configuration=Debug /p:Platform=Win32
rem echo Building Debug x64
rem msbuild "sg.sln" /p:Configuration=Debug /p:Platform=x64
echo Building Release Win32
msbuild "sg.sln" /p:Configuration=Release /p:Platform=Win32
rem echo Building Release x64
rem msbuild "sg.sln" /p:Configuration=Release /p:Platform=x64
echo Build finished.
pause
IF ERRORLEVEL 1 EXIT 1

echo Traversing Backwards.
cd ..\
