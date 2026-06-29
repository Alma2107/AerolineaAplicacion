#ifndef MODULO_VUELOS_H
#define MODULO_VUELOS_H

#include "../dao/VueloDAO.h"
#include "../dao/AvionDAO.h"
#include "../dao/NotificacionDAO.h"
#include "../modelos/Avion.h"
#include "../modelos/Notificacion.h"
#include <string>
#include <vector>

struct DatosClima
{
    bool consultado = false;
    bool ok = false;
    std::string iata = "AEP";
    std::string ciudad = "Buenos Aires";
    std::string actualizado = "";
    std::string condicion = "Pendiente de consulta";
    std::string fuente = "Open-Meteo";
    double temperatura = 0;
    double sensacion = 0;
    double humedad = 0;
    double precipitacion = 0;
    double lluvia = 0;
    double nubosidad = 0;
    double presion = 0;
    double viento = 0;
    double direccionViento = 0;
    double rafagas = 0;
    double tempPromedioDia = 0;
    double probLluviaMax = 0;
    double vientoPromedioDia = 0;
    double nubosidadPromedioDia = 0;
    double visibilidadMinimaKm = 0;
    std::string detalleError = "";
};

class ModuloVuelos
{
private:
    VueloDAO vueloDAO;
    AvionDAO avionDAO;
    NotificacionDAO notificacionDAO;
    std::string mensaje;
    int vista;
    int foco;

    std::string numero;
    std::string origen;
    std::string destino;
    std::string fechaSalida;
    std::string fechaLlegada;
    std::string precio;
    std::string idAvion;
    std::string estado;

    std::string accionIdVuelo;
    std::string accionIdAvion;
    std::string accionFechaSalida;
    std::string accionFechaLlegada;
    std::string accionMotivo;
    std::string climaIata;
    std::string filtroHistorial;
    DatosClima clima;
    int vistaAnterior;
    int notificacionesNoLeidas;
    std::vector<Avion> avionesCache;
    std::vector<Avion> avionesDisponiblesCache;
    std::vector<Vuelo> vuelosCache;
    std::vector<Notificacion> notificacionesCache;
    std::vector<std::string> aeropuertosCache;

    void dibujarNavegacion();
    void dibujarCrearVuelo();
    void dibujarAcciones();
    void dibujarNotificaciones();
    void dibujarListado();
    void refrescarDatos();
    void refrescarAvisos();

public:
    ModuloVuelos();
    void mostrar();
};

#endif
