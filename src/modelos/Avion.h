#ifndef AVION_H
#define AVION_H

#include <string>

class Avion
{
private:
    int id;
    std::string modelo;
    int capacidad;
    std::string estado;

public:
    Avion() : id(0), modelo(""), capacidad(0), estado("Activo") {}
    Avion(int id, const std::string &modelo, int capacidad, const std::string &estado)
        : id(id), modelo(modelo), capacidad(capacidad), estado(estado) {}

    int getId() const { return id; }
    void setId(int valor) { id = valor; }

    std::string getModelo() const { return modelo; }
    void setModelo(const std::string &valor) { modelo = valor; }

    int getCapacidad() const { return capacidad; }
    void setCapacidad(int valor) { capacidad = valor; }

    std::string getEstado() const { return estado; }
    void setEstado(const std::string &valor) { estado = valor; }
};

#endif
