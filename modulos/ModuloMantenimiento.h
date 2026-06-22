#ifndef MODULO_MANTENIMIENTO_H
#define MODULO_MANTENIMIENTO_H

#include "../dao/AvionDAO.h"
#include "../dao/VueloDAO.h"
#include "../dao/NotificacionDAO.h"
#include <string>

class ModuloMantenimiento
{
private:
    AvionDAO avionDAO;
    VueloDAO vueloDAO;
    NotificacionDAO notificacionDAO;
    std::string mensaje;
    int vista;
    int foco;
    std::string idAvion;
    std::string estado;
    std::string modeloNuevo;
    std::string capacidadNueva;
    std::string estadoNuevo;

    void dibujarNavegacion();
    void dibujarEstadoFlota();
    void dibujarRegistrarAeronave();
    void dibujarActualizarEstado();
    void dibujarListado();
    bool notificarOperacion(int avionId, const std::string &nuevoEstado);

public:
    ModuloMantenimiento();
    void mostrar();
};

#endif
