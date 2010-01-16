@echo off
echo Initializing VCX environment variables...
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"

echo Building Debug
msbuild sg.sln /p:configuration=Debug
echo Building Release
msbuild sg.sln /p:configuration=Release
echo Building Release SSE
msbuild sg.sln /p:configuration="Release SSE"
echo Building Release SSE2
msbuild sg.sln /p:configuration="Release SSE2"

echo Reiterating build process for sanity.

echo Building Debug
msbuild sg.sln /p:configuration=Debug
echo Building Release
msbuild sg.sln /p:configuration=Release
echo Building Release SSE
msbuild sg.sln /p:configuration="Release SSE"
echo Building Release SSE2
msbuild sg.sln /p:configuration="Release SSE2"

echo Build finished.
pause

IF ERRORLEVEL 1 EXIT 1