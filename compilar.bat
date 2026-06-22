@echo off
g++ main.cpp dao\ConexionDB.cpp dao\EmpleadoDAO.cpp dao\AvionDAO.cpp dao\VueloDAO.cpp dao\PasajeroDAO.cpp dao\ReservaDAO.cpp dao\EquipajeDAO.cpp dao\NotificacionDAO.cpp modulos\ComponentesUI.cpp modulos\MenuPrincipal.cpp modulos\ModuloVuelos.cpp modulos\ModuloPasajeros.cpp modulos\ModuloEquipaje.cpp modulos\ModuloMantenimiento.cpp -o aerolinea_xampp_fix.exe -lraylib -lopengl32 -lgdi32 -lwinmm
if %errorlevel% equ 0 (
    echo Compilacion correcta.
) else (
    echo Hubo errores de compilacion.
)
