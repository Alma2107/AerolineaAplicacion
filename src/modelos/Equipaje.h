#ifndef EQUIPAJE_H
#define EQUIPAJE_H

#include <string>

class Equipaje
{
private:
    int id;
    int idPasajero;
    std::string codigoEtiqueta;
    double peso;
    std::string estado;

public:
    Equipaje() : id(0), idPasajero(0), codigoEtiqueta(""), peso(0), estado("Registrado") {}
    Equipaje(int id, int idPasajero, const std::string &codigoEtiqueta, double peso, const std::string &estado)
        : id(id), idPasajero(idPasajero), codigoEtiqueta(codigoEtiqueta), peso(peso), estado(estado) {}

    int getId() const { return id; }
    void setId(int valor) { id = valor; }

    int getIdPasajero() const { return idPasajero; }
    void setIdPasajero(int valor) { idPasajero = valor; }

    std::string getCodigoEtiqueta() const { return codigoEtiqueta; }
    void setCodigoEtiqueta(const std::string &valor) { codigoEtiqueta = valor; }

    double getPeso() const { return peso; }
    void setPeso(double valor) { peso = valor; }

    std::string getEstado() const { return estado; }
    void setEstado(const std::string &valor) { estado = valor; }
};

#endif
