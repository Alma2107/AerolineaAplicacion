#ifndef MODULO_MANTENIMIENTO_H
#define MODULO_MANTENIMIENTO_H

#include "../dao/AvionDAO.h"
#include "../dao/VueloDAO.h"
#include "../dao/NotificacionDAO.h"
#include "../modelos/Avion.h"
#include <string>
#include <vector>

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
    std::string avionObjeto;
    std::string numeroAsiento;
    std::string asientoSeleccionadoMapa;
    int vistaAnterior;
    int avionMapaSeleccionado;
    std::vector<Avion> avionesCache;

    void dibujarNavegacion();
    void dibujarEstadoFlota();
    void dibujarMapaAviones();
    void dibujarPlanoAsientos(const Avion &avion);
    void dibujarRegistrarAeronave();
    void dibujarActualizarEstado();
    void dibujarListado();
    void refrescarDatos();
    bool notificarOperacion(int avionId, const std::string &nuevoEstado);

public:
    ModuloMantenimiento();
    void mostrar();
};

#endif
