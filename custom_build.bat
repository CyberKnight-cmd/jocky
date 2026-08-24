@echo off
setlocal

:: Initialize MSVC build environment if cl.exe is not available
where cl.exe >nul 2>nul
if %errorlevel% neq 0 (
    echo Initializing MSVC environment...
    call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
)

echo Building JOCKY Compiler (v0.1)...


if not exist bin mkdir bin

:: Compile all C files in src directory
:: /nologo: suppress startup banner
:: /W3: warning level 3
:: /Zi: generate debugging information
:: /Fe: output executable name
cl.exe /nologo /W3 /Zi src\*.c /Febin\jky.exe

if %errorlevel% neq 0 (
    echo Build failed.
    exit /b %errorlevel%
)

echo Build succeeded! Executable is in bin\jky.exe
endlocal
