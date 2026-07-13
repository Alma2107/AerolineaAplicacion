@echo off
setlocal
for %%I in ("%~dp0..") do set "BASE=%%~fI"
set "GPP="
set "SRC_DIR=%BASE%\src"
set "OUT_DIR=%BASE%\build"
set "RAYLIB_INCLUDE="
set "RAYLIB_LIB="

if exist "C:\msys64\ucrt64\bin\g++.exe" set "GPP=C:\msys64\ucrt64\bin\g++.exe"
if not defined GPP if exist "C:\msys64\mingw64\bin\g++.exe" set "GPP=C:\msys64\mingw64\bin\g++.exe"
if not defined GPP if exist "C:\msys64\mingw32\bin\g++.exe" set "GPP=C:\msys64\mingw32\bin\g++.exe"
if not defined GPP if exist "C:\ProgramData\mingw64\mingw64\bin\g++.exe" set "GPP=C:\ProgramData\mingw64\mingw64\bin\g++.exe"
if not defined GPP if exist "C:\MinGW\bin\g++.exe" set "GPP=C:\MinGW\bin\g++.exe"
if not defined GPP if exist "C:\Program Files\mingw-w64\bin\g++.exe" set "GPP=C:\Program Files\mingw-w64\bin\g++.exe"
if not defined GPP (
    for /f "delims=" %%G in ('where g++ 2^>nul') do (
        set "GPP=%%G"
        goto :gpp_found
    )
)
:gpp_found

if not exist "%GPP%" (
    echo No se encontro g++. Instale MinGW/Raylib o ajuste la ruta GPP en este script.
    exit /b 1
)
for %%G in ("%GPP%") do set "GPP_BIN=%%~dpG"
set "PATH=%GPP_BIN%;%PATH%"

if exist "C:\msys64\ucrt64\include\raylib.h" (
    set "RAYLIB_INCLUDE=C:\msys64\ucrt64\include"
    set "RAYLIB_LIB=C:\msys64\ucrt64\lib"
) else if exist "C:\msys64\mingw64\include\raylib.h" (
    set "RAYLIB_INCLUDE=C:\msys64\mingw64\include"
    set "RAYLIB_LIB=C:\msys64\mingw64\lib"
) else if exist "C:\msys64\ucrt64\bin\raylib.dll" (
    set "RAYLIB_INCLUDE=C:\msys64\ucrt64\include"
    set "RAYLIB_LIB=C:\msys64\ucrt64\lib"
) else if exist "C:\raylib\raylib-6.0_win64_mingw-w64\include\raylib.h" (
    set "RAYLIB_INCLUDE=C:\raylib\raylib-6.0_win64_mingw-w64\include"
    set "RAYLIB_LIB=C:\raylib\raylib-6.0_win64_mingw-w64\lib"
) else if exist "%BASE%\raylib\include\raylib.h" (
    set "RAYLIB_INCLUDE=%BASE%\raylib\include"
    set "RAYLIB_LIB=%BASE%\raylib\lib"
)

if not defined RAYLIB_INCLUDE (
    echo No se encontro Raylib include. Ajuste la ruta RAYLIB_INCLUDE en este script.
    exit /b 1
)
if not defined RAYLIB_LIB (
    echo No se encontro Raylib lib. Ajuste la ruta RAYLIB_LIB en este script.
    exit /b 1
)

if not exist "%RAYLIB_INCLUDE%" (
    echo No se encontro Raylib include: %RAYLIB_INCLUDE%
    exit /b 1
)

if not exist "%RAYLIB_LIB%" (
    echo No se encontro Raylib lib: %RAYLIB_LIB%
    exit /b 1
)

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
set "OBJ_DIR=%OUT_DIR%\obj"
if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%"
set "TMP=%OUT_DIR%\tmp"
set "TEMP=%OUT_DIR%\tmp"
if not exist "%TMP%" mkdir "%TMP%"
if exist "%OBJ_DIR%\*.o" del /q "%OBJ_DIR%\*.o"
set "EXE_OUT=%BASE%\aerolinea_actualizado.exe"
if exist "%EXE_OUT%" del /q "%EXE_OUT%"
set "COMPILE_LOG=%OUT_DIR%\compile_errors.log"
if exist "%COMPILE_LOG%" del /q "%COMPILE_LOG%"

setlocal enableextensions enabledelayedexpansion

echo Compilando archivos fuente...

rem list of source files (absolute paths). Keep every item quoted for folders with spaces.
set SRCS=%BASE%\main.cpp %SRC_DIR%\dao\ConexionDB.cpp %SRC_DIR%\dao\EmpleadoDAO.cpp %SRC_DIR%\dao\AvionDAO.cpp %SRC_DIR%\dao\VueloDAO.cpp %SRC_DIR%\dao\PasajeroDAO.cpp %SRC_DIR%\dao\ReservaDAO.cpp %SRC_DIR%\dao\EquipajeDAO.cpp %SRC_DIR%\dao\NotificacionDAO.cpp %SRC_DIR%\modulos\ComponentesUI.cpp %SRC_DIR%\modulos\MenuPrincipal.cpp %SRC_DIR%\modulos\ModuloVuelos.cpp %SRC_DIR%\modulos\ModuloPasajeros.cpp %SRC_DIR%\modulos\ModuloEquipaje.cpp %SRC_DIR%\modulos\ModuloMantenimiento.cpp

for %%S in (%SRCS%) do (
    echo Compilando %%~nxS
    echo "%GPP%" -c "%%S" -o "%OBJ_DIR%\%%~nxS.o" -I"%RAYLIB_INCLUDE%" -I"%BASE%" -I"%SRC_DIR%" > "%COMPILE_LOG%"
    "%GPP%" -c "%%S" -o "%OBJ_DIR%\%%~nxS.o" -I"%RAYLIB_INCLUDE%" -I"%BASE%" -I"%SRC_DIR%" >> "%COMPILE_LOG%" 2>&1
    if not exist "%OBJ_DIR%\%%~nxS.o" (
        echo Error al compilar %%~nxS
        type "%COMPILE_LOG%"
        echo.
        echo El compilador no genero el archivo objeto esperado. Si no aparece ningun error arriba, revise la instalacion de g++/MSYS2.
        exit /b 1
    )
)

echo Enlazando objetos...
set OBJLIST=
for %%o in ("%OBJ_DIR%\*.o") do set OBJLIST=!OBJLIST! "%%~fo"

"%GPP%" %OBJLIST% -o "%EXE_OUT%" -L"%RAYLIB_LIB%" -lraylib -lopengl32 -lgdi32 -lwinmm -lws2_32 -lshell32 -Wl,--subsystem,windows > "%COMPILE_LOG%" 2>&1

if exist "%EXE_OUT%" (
    if exist "%RAYLIB_LIB%\raylib.dll" copy /y "%RAYLIB_LIB%\raylib.dll" "%BASE%\raylib.dll" >nul
    if exist "%RAYLIB_LIB%\..\bin\libraylib.dll" copy /y "%RAYLIB_LIB%\..\bin\libraylib.dll" "%BASE%\libraylib.dll" >nul
    if exist "%BASE%\libraylib.dll" copy /y "%BASE%\libraylib.dll" "%BASE%\raylib.dll" >nul
    echo Compilacion correcta: %EXE_OUT%
) else (
    echo Hubo errores de compilacion.
    type "%COMPILE_LOG%"
    echo.
    echo No se genero %EXE_OUT%. Revise el log anterior antes de abrir el ejecutable.
    exit /b 1
)

endlocal
