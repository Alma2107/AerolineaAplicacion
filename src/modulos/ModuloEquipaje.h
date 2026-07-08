#ifndef MODULO_EQUIPAJE_H
#define MODULO_EQUIPAJE_H

#include "../dao/EquipajeDAO.h"
#include "../dao/NotificacionDAO.h"
#include "../modelos/Equipaje.h"
#include "../modelos/Notificacion.h"
#include <string>
#include <vector>

class ModuloEquipaje
{
private:
    EquipajeDAO equipajeDAO;
    NotificacionDAO notificacionDAO;
    std::string mensaje;
    int vista;
    int foco;
    std::string idTicket;
    std::string idTipo;
    std::string peso;
    std::string precio;
    std::string etiqueta;
    std::string filtroListado;
    int vistaAnterior;
    int notificacionesNoLeidas;
    std::vector<Equipaje> equipajesCache;
    std::vector<Notificacion> notificacionesCache;
    std::vector<CatalogoItem> ticketsCache;
    std::vector<CatalogoItem> tiposEquipajeCache;

    void dibujarNavegacion();
    void dibujarRegistrar();
    void dibujarRastrear();
    void dibujarListado();
    void dibujarAvisos();
    void refrescarDatos();
    void refrescarAvisos();

public:
    ModuloEquipaje();
    void mostrar();
};

#endif
