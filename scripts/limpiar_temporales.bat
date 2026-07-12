@echo off
REM Script para limpiar archivos temporales de compilacion y testing

echo Limpiando archivos temporales de la aplicacion...
echo.

set DELETED_COUNT=0

REM Limpiar archivos temporales en build/
if exist "build\temp_test.txt" (
    del /q "build\temp_test.txt" >nul 2>&1
    set /a DELETED_COUNT+=1
    echo [OK] Eliminado: build\temp_test.txt
)

if exist "build\temp_exec.txt" (
    del /q "build\temp_exec.txt" >nul 2>&1
    set /a DELETED_COUNT+=1
    echo [OK] Eliminado: build\temp_exec.txt
)

if exist "build\temp_query.txt" (
    del /q "build\temp_query.txt" >nul 2>&1
    set /a DELETED_COUNT+=1
    echo [OK] Eliminado: build\temp_query.txt
)

if exist "build\temp_exec.txt" (
    del /q "build\temp_exec.txt" >nul 2>&1
    echo [OK] Eliminado: build\temp_exec.txt
)

REM Limpiar archivos temporales en raiz
if exist "temp_exec.txt" (
    del /q "temp_exec.txt" >nul 2>&1
    set /a DELETED_COUNT+=1
    echo [OK] Eliminado: temp_exec.txt
)

if exist "temp_query.txt" (
    del /q "temp_query.txt" >nul 2>&1
    set /a DELETED_COUNT+=1
    echo [OK] Eliminado: temp_query.txt
)

if exist "temp_test.txt" (
    del /q "temp_test.txt" >nul 2>&1
    set /a DELETED_COUNT+=1
    echo [OK] Eliminado: temp_test.txt
)

REM Limpiar logs de compilacion antiguos
if exist "build\compile_errors.log" (
    del /q "build\compile_errors.log" >nul 2>&1
    set /a DELETED_COUNT+=1
    echo [OK] Eliminado: build\compile_errors.log
)

if exist "build\cc1plus.log" (
    del /q "build\cc1plus.log" >nul 2>&1
    set /a DELETED_COUNT+=1
    echo [OK] Eliminado: build\cc1plus.log
)

REM Limpiar carpeta tmp si existe
if exist "build\tmp\*" (
    del /q "build\tmp\*" >nul 2>&1
    echo [OK] Limpiada carpeta: build\tmp\
)

echo.
echo ============================================
echo Limpieza completada
echo Archivos eliminados: %DELETED_COUNT%
echo.
echo Nota: Los DLLs en build\ se mantienen para evitar
echo problemas de compatibilidad.
echo.
