#ifndef AEROPUERTO_H
#define AEROPUERTO_H

#include <string>

class Aeropuerto
{
private:
    std::string codigoIata;
    std::string nombre;
    std::string ciudad;
    std::string pais;

public:
    Aeropuerto() : codigoIata(""), nombre(""), ciudad(""), pais("") {}
    Aeropuerto(const std::string &codigoIata, const std::string &nombre,
               const std::string &ciudad, const std::string &pais)
        : codigoIata(codigoIata), nombre(nombre), ciudad(ciudad), pais(pais) {}

    std::string getCodigoIata() const { return codigoIata; }
    void setCodigoIata(const std::string &valor) { codigoIata = valor; }

    std::string getNombre() const { return nombre; }
    void setNombre(const std::string &valor) { nombre = valor; }

    std::string getCiudad() const { return ciudad; }
    void setCiudad(const std::string &valor) { ciudad = valor; }

    std::string getPais() const { return pais; }
    void setPais(const std::string &valor) { pais = valor; }
};

#endif
