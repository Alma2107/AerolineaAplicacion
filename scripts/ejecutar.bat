@echo off
setlocal
for %%I in ("%~dp0..") do set "BASE=%%~fI\"
cd /d "%BASE%"
set "PATH=%BASE%build;C:\xampp\mysql\bin;%PATH%"

if exist "%BASE%raylib.dll" copy /y "%BASE%raylib.dll" "%BASE%build\raylib.dll" >nul
if exist "%BASE%build\raylib.dll" copy /y "%BASE%build\raylib.dll" "%BASE%build\raylib.dll" >nul
if exist "%BASE%build\raylib.dll" copy /y "%BASE%build\raylib.dll" "%BASE%build\libraylib.dll" >nul

if exist "%BASE%build\aerolinea.exe" (
    start "" "%BASE%build\aerolinea.exe"
) else (
    echo No se encontro aerolinea.exe en %BASE%build
    pause
)
