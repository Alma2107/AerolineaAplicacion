#ifndef RESERVA_DAO_H
#define RESERVA_DAO_H

#include "../modelos/Pasajero.h"
#include "../modelos/Reserva.h"
#include "ConexionDB.h"
#include <string>
#include <vector>

class ReservaDAO
{
private:
    ConexionDB db;

public:
    ReservaDAO();
    std::vector<Reserva> listar() const;
    Reserva buscarPorQR(const std::string &codigoQR) const;
    std::string validarReservaPresencial(const Pasajero &pasajero, int idCliente, int idVuelo,
                                         const std::string &asiento, int idPlan,
                                         const std::string &codigoQR, double precio,
                                         int idMetodoPago) const;
    Reserva crearReservaPresencial(const Pasajero &pasajero, int idCliente, int idVuelo,
                                   const std::string &asiento, int idPlan,
                                   const std::string &codigoQR, double precio,
                                   int idMetodoPago);
    bool registrarCheckIn(const std::string &codigoQR);
    bool cancelar(int idTicket);
    bool reembolsar(int idTicket);
};

#endif
