#ifndef MODULO_PASAJEROS_H
#define MODULO_PASAJEROS_H

#include "../dao/PasajeroDAO.h"
#include "../dao/ReservaDAO.h"
#include "../dao/NotificacionDAO.h"
#include <string>

class ModuloPasajeros
{
private:
    PasajeroDAO pasajeroDAO;
    ReservaDAO reservaDAO;
    NotificacionDAO notificacionDAO;
    std::string mensaje;
    int vista;
    int foco;

    std::string qr;
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
    std::string codigoQR;
    std::string precio;

    void dibujarNavegacion();
    void dibujarCheckIn();
    void dibujarNuevaReserva();
    void dibujarCancelaciones();
    void dibujarNotificaciones();
    void dibujarListado();

public:
    ModuloPasajeros();
    void mostrar();
};

#endif
