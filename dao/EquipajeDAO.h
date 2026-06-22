#ifndef EQUIPAJE_DAO_H
#define EQUIPAJE_DAO_H

#include "../modelos/Equipaje.h"
#include "ConexionDB.h"
#include <vector>

class EquipajeDAO
{
private:
    ConexionDB db;

public:
    EquipajeDAO();
    std::vector<Equipaje> listar() const;
    Equipaje registrar(int idTicket, int idTipoEquipaje, double peso, double precio);
    Equipaje rastrear(const std::string &codigoEtiqueta) const;
    bool actualizarEstado(const std::string &codigoEtiqueta, const std::string &estado);
};

#endif
