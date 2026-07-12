#ifndef MODULO_PASAJEROS_H
#define MODULO_PASAJEROS_H

#include "../dao/PasajeroDAO.h"
#include "../dao/ReservaDAO.h"
#include "../dao/NotificacionDAO.h"
#include "../modelos/Notificacion.h"
#include <string>
#include <vector>

class ModuloPasajeros
{
private:
    PasajeroDAO pasajeroDAO;
    ReservaDAO reservaDAO;
    NotificacionDAO notificacionDAO;
    std::string mensaje;
    int vista;
    int foco;

    std::string codigoReserva;
    std::string tipoDoc;
    std::string documento;
    std::string nombre;
    std::string apellido;
    std::string fechaNacimiento;
    std::string asistencia;
    std::string detalles;
    std::string idVuelo;
    std::string idCliente;
    std::string idMetodoPago;
    std::string asiento;
    std::string idPlan;
    std::string tipoViaje;
    std::string codigoReservaNueva;
    std::string precio;
    std::string idTipoEquipaje;
    std::string cantidadEquipaje;
    std::string precioEquipaje;
    std::string idServicio;
    std::string precioServicio;
    std::string estadoAsientos;
    int vistaAnterior;
    int notificacionesNoLeidas;
    std::vector<Reserva> reservasCache;
    std::vector<Pasajero> pasajerosCache;
    std::vector<Notificacion> notificacionesCache;
    std::vector<CatalogoItem> planesCache;
    std::vector<CatalogoItem> vuelosCache;
    std::vector<CatalogoItem> tiposEquipajeCache;
    std::vector<CatalogoItem> serviciosCache;
    std::vector<std::string> asientosCache;
    std::vector<int> equipajesSeleccionados;
    std::vector<int> cantidadesEquipajeSeleccionadas;
    std::vector<int> serviciosSeleccionados;

    void dibujarNavegacion();
    void dibujarCheckIn();
    void dibujarNuevaReserva();
    void dibujarCancelaciones();
    void dibujarNotificaciones();
    void dibujarListado();
    void refrescarDatos();
    void refrescarAvisos();

public:
    ModuloPasajeros();
    void mostrar();
};

#endif
