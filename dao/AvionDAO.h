#ifndef AVION_DAO_H
#define AVION_DAO_H

#include "../modelos/Avion.h"
#include "ConexionDB.h"
#include <string>
#include <vector>

class AvionDAO
{
private:
    ConexionDB db;

public:
    AvionDAO();
    std::vector<Avion> listar() const;
    std::vector<Avion> listarDisponibles() const;
    Avion buscarPorId(int id) const;
    Avion crear(const std::string &modelo, int capacidad, const std::string &estado);
    bool actualizarEstado(int id, const std::string &estado);
};

#endif
