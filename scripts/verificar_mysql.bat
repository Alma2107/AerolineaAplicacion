@echo off
REM Script para verificar si MySQL esta disponible y funcionando
REM Si no esta ejecutando, intenta iniciarlo desde XAMPP

cls
echo ============================================
echo   Verificador de Conexion MySQL
echo ============================================
echo.

REM Intentar conectar a MySQL
echo [1/3] Probando conexion a MySQL en 127.0.0.1:3306...
"C:\xampp\mysql\bin\mysql.exe" -h 127.0.0.1 -P 3306 -u root -e "SELECT 1;" >nul 2>&1

if errorlevel 1 (
    echo [ERROR] MySQL no responde. Intentando iniciar XAMPP MySQL...
    echo.
    
    REM Intenta usar el script de XAMPP
    if exist "C:\xampp\mysql_start.bat" (
        echo [2/3] Ejecutando C:\xampp\mysql_start.bat
        call "C:\xampp\mysql_start.bat"
        timeout /t 3 /nobreak
    ) else if exist "C:\xampp\mysql_start.cmd" (
        echo [2/3] Ejecutando C:\xampp\mysql_start.cmd
        call "C:\xampp\mysql_start.cmd"
        timeout /t 3 /nobreak
    ) else (
        echo [AVISO] No se encontro script de inicio de MySQL en XAMPP
        echo Abre manualmente el Panel de Control de XAMPP y haz click en "Start" para MySQL
        echo.
        pause
        goto :eof
    )
    
    REM Reintentar conexion
    echo.
    echo [3/3] Reintentando conexion a MySQL...
    timeout /t 2 /nobreak
    "C:\xampp\mysql\bin\mysql.exe" -h 127.0.0.1 -P 3306 -u root -e "SELECT 1;" >nul 2>&1
    
    if errorlevel 1 (
        echo [ERROR] MySQL aun no responde tras 5 segundos.
        echo.
        echo Posibles causas:
        echo  - MySQL tarda mas tiempo en iniciar
        echo  - Puerto 3306 ya esta ocupado
        echo  - MySQL tiene otra contrasena configurada
        echo.
        echo Acciones:
        echo  1. Espera 10 segundos y vuelve a ejecutar este script
        echo  2. Abre C:\xampp\xampp-control.exe y inicia MySQL manualmente
        echo  3. Verifica en logs\conexion_aerogest.log para mas detalles
        echo.
        pause
        goto :eof
    )
) else (
    echo [OK] MySQL esta disponible y respondiendo
)

echo.
echo [2/3] Verificando base de datos 'aerolinea'...
"C:\xampp\mysql\bin\mysql.exe" -h 127.0.0.1 -P 3306 -u root -e "USE aerolinea; SELECT 1;" >nul 2>&1

if errorlevel 1 (
    echo [AVISO] Base de datos 'aerolinea' no existe o no es accesible
    echo Se intentara crear/importar durante la ejecucion de la aplicacion
) else (
    echo [OK] Base de datos 'aerolinea' existe y es accesible
)

echo.
echo [3/3] Listando bases de datos disponibles:
"C:\xampp\mysql\bin\mysql.exe" -h 127.0.0.1 -P 3306 -u root -e "SHOW DATABASES;"

echo.
echo ============================================
echo   Verificacion completada
echo ============================================
echo.
