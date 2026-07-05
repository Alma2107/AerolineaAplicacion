#ifndef RESERVA_H
#define RESERVA_H

#include <string>

class Reserva
{
private:
    int idTicket;
    int idPasajero;
    int idVuelo;
    std::string pasajero;
    std::string documento;
    std::string numeroVuelo;
    std::string origen;
    std::string destino;
    std::string fechaSalida;
    std::string asiento;
    std::string codigoReserva;
    double precio;

public:
    Reserva() : idTicket(0), idPasajero(0), idVuelo(0), pasajero(""), documento(""),
                numeroVuelo(""), origen(""), destino(""), fechaSalida(""), asiento(""),
                codigoReserva(""), precio(0) {}

    Reserva(int idTicket, int idPasajero, int idVuelo, const std::string &pasajero,
            const std::string &documento, const std::string &numeroVuelo,
            const std::string &origen, const std::string &destino,
            const std::string &fechaSalida, const std::string &asiento,
            const std::string &codigoReserva, double precio)
        : idTicket(idTicket), idPasajero(idPasajero), idVuelo(idVuelo), pasajero(pasajero),
          documento(documento), numeroVuelo(numeroVuelo), origen(origen), destino(destino),
          fechaSalida(fechaSalida), asiento(asiento), codigoReserva(codigoReserva), precio(precio) {}

    int getIdTicket() const { return idTicket; }
    int getIdPasajero() const { return idPasajero; }
    int getIdVuelo() const { return idVuelo; }
    std::string getPasajero() const { return pasajero; }
    std::string getDocumento() const { return documento; }
    std::string getNumeroVuelo() const { return numeroVuelo; }
    std::string getOrigen() const { return origen; }
    std::string getDestino() const { return destino; }
    std::string getFechaSalida() const { return fechaSalida; }
    std::string getAsiento() const { return asiento; }
    std::string getCodigoReserva() const { return codigoReserva; }
    double getPrecio() const { return precio; }
};

#endif
