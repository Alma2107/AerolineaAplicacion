#ifndef RESERVA_DAO_H
#define RESERVA_DAO_H

#include "../modelos/Pasajero.h"
#include "../modelos/Reserva.h"
#include "ConexionDB.h"
#include <string>
#include <vector>

struct CatalogoItem
{
    int id;
    std::string nombre;
    double precio;
};

class ReservaDAO
{
private:
    ConexionDB db;

public:
    ReservaDAO();
    std::vector<Reserva> listar() const;
    Reserva buscarPorCodigoReserva(const std::string &codigoReserva) const;
    std::string generarCodigoReserva() const;
    int contarAsientosDisponibles(int idVuelo) const;
    std::vector<CatalogoItem> listarPlanes() const;
    std::vector<CatalogoItem> listarTiposEquipaje() const;
    std::vector<CatalogoItem> listarServicios() const;
    std::vector<CatalogoItem> listarVuelosDisponibles() const;
    std::vector<std::string> listarAsientosDisponibles(int idVuelo) const;
    std::string validarReservaPresencial(const Pasajero &pasajero, int idCliente, int idVuelo,
                                         const std::string &asiento, int idPlan,
                                         const std::string &codigoReserva, double precio,
                                         int idMetodoPago, int idTipoEquipaje = 0,
                                         int cantidadEquipaje = 0, double precioEquipaje = 0,
                                         int idServicio = 0, double precioServicio = 0) const;
    Reserva crearReservaPresencial(const Pasajero &pasajero, int idCliente, int idVuelo,
                                   const std::string &asiento, int idPlan,
                                   const std::string &codigoReserva, double precio,
                                   int idMetodoPago, int idTipoEquipaje = 0,
                                   int cantidadEquipaje = 0, double precioEquipaje = 0,
                                   int idServicio = 0, double precioServicio = 0);
    Reserva crearReservaPresencial(const Pasajero &pasajero, int idCliente, int idVuelo,
                                   const std::string &asiento, int idPlan,
                                   int idMetodoPago, const std::vector<int> &tiposEquipaje,
                                   const std::vector<int> &cantidadesEquipaje,
                                   const std::vector<int> &servicios);
    bool registrarCheckIn(const std::string &codigoReserva);
    bool cancelar(int idTicket);
    bool reembolsar(int idTicket);
};

#endif
