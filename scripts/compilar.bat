@echo off
setlocal
for %%I in ("%~dp0..") do set "BASE=%%~fI\"
set "GPP=C:\ProgramData\mingw64\mingw64\bin\g++.exe"
set "SRC_DIR=%BASE%src"
set "OUT_DIR=%BASE%build"
set "RAYLIB_INCLUDE=C:\raylib\raylib-6.0_win64_mingw-w64\include"
set "RAYLIB_LIB=C:\raylib\raylib-6.0_win64_mingw-w64\lib"

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
set "OBJ_DIR=%OUT_DIR%\obj"
if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%"

setlocal enableextensions enabledelayedexpansion

echo Compilando archivos fuente...
set SRCFILES="%BASE%main.cpp" "%%SRC%%"

rem list of source files (absolute paths)
set SRCS=%BASE%main.cpp %SRC_DIR%\dao\ConexionDB.cpp %SRC_DIR%\dao\EmpleadoDAO.cpp %SRC_DIR%\dao\AvionDAO.cpp %SRC_DIR%\dao\VueloDAO.cpp %SRC_DIR%\dao\PasajeroDAO.cpp %SRC_DIR%\dao\ReservaDAO.cpp %SRC_DIR%\dao\EquipajeDAO.cpp %SRC_DIR%\dao\NotificacionDAO.cpp %SRC_DIR%\modulos\ComponentesUI.cpp %SRC_DIR%\modulos\MenuPrincipal.cpp %SRC_DIR%\modulos\ModuloVuelos.cpp %SRC_DIR%\modulos\ModuloPasajeros.cpp %SRC_DIR%\modulos\ModuloEquipaje.cpp %SRC_DIR%\modulos\ModuloMantenimiento.cpp

for %%f in (%SRCS%) do (
    echo Compilando %%~nxf
    "%GPP%" -c "%%~f" -o "%OBJ_DIR%\\%%~nxf.o" -I"%RAYLIB_INCLUDE%" -I"%BASE%" -I"%SRC_DIR%"
    if errorlevel 1 (
        echo Error al compilar %%~nxf
        exit /b 1
    )
)

echo Enlazando objetos...
set OBJLIST=
for %%o in ("%OBJ_DIR%\*.o") do set OBJLIST=!OBJLIST! "%%~fo"

"%GPP%" %OBJLIST% -o "%OUT_DIR%\aerolinea.exe" -lopengl32 -lgdi32 -lwinmm -lws2_32 -lshell32 -Wl,--subsystem,windows "%RAYLIB_LIB%\libraylibdll.a"

if %errorlevel% equ 0 (
    copy /y "%RAYLIB_LIB%\raylib.dll" "%OUT_DIR%\raylib.dll" >nul
    copy /y "%RAYLIB_LIB%\raylib.dll" "%OUT_DIR%\libraylib.dll" >nul
    echo Compilacion correcta.
) else (
    echo Hubo errores de compilacion.
    exit /b 1
)

endlocal
