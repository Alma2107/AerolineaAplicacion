#ifndef EQUIPAJE_DAO_H
#define EQUIPAJE_DAO_H

#include "../modelos/Equipaje.h"
#include "ConexionDB.h"
#include "ReservaDAO.h"
#include <vector>

class EquipajeDAO
{
private:
    ConexionDB db;

public:
    EquipajeDAO();
    std::vector<Equipaje> listar() const;
    std::vector<Equipaje> buscar(const std::string &filtro) const;
    std::vector<CatalogoItem> listarTickets() const;
    std::vector<CatalogoItem> listarTiposEquipaje() const;
    Equipaje registrar(int idTicket, int idTipoEquipaje, double peso, double precio);
    Equipaje rastrear(const std::string &codigoEtiqueta) const;
    std::string buscarPropietarioPorEtiqueta(const std::string &codigoEtiqueta) const;
    bool actualizarEstado(const std::string &codigoEtiqueta, const std::string &estado);
};

#endif
