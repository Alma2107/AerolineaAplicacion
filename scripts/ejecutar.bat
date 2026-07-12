@echo off
setlocal
for %%I in ("%~dp0..") do set "BASE=%%~fI\"
cd /d "%BASE%"
set "PATH=%BASE%build;C:\xampp\mysql\bin;%PATH%"

echo.
echo Verificando MySQL antes de compilar...
echo.
call "%BASE%scripts\verificar_mysql.bat"
echo.

call "%BASE%scripts\compilar.bat"
if errorlevel 1 (
    echo Error durante la compilacion. No se iniciara el ejecutable.
    pause
    exit /b 1
)

if exist "%BASE%raylib.dll" copy /y "%BASE%raylib.dll" "%BASE%build\raylib.dll" >nul
if exist "%BASE%build\raylib.dll" copy /y "%BASE%build\raylib.dll" "%BASE%build\raylib.dll" >nul
if exist "%BASE%build\raylib.dll" copy /y "%BASE%build\raylib.dll" "%BASE%build\libraylib.dll" >nul

if exist "%BASE%build\aerolinea_actualizado.exe" (
    start "" "%BASE%build\aerolinea_actualizado.exe"
) else if exist "%BASE%build\aerolinea.exe" (
    start "" "%BASE%build\aerolinea.exe"
) else (
    echo No se encontro aerolinea_actualizado.exe ni aerolinea.exe en %BASE%build
    pause
)
