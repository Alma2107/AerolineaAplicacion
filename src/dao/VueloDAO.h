#ifndef VUELO_DAO_H
#define VUELO_DAO_H

#include "../modelos/Vuelo.h"
#include "ConexionDB.h"
#include <vector>

class VueloDAO
{
private:
    ConexionDB db;

public:
    VueloDAO();
    std::vector<Vuelo> listar() const;
    std::vector<std::string> listarAeropuertos() const;
    Vuelo buscarPorId(int id) const;
    std::string validar(const Vuelo &vuelo) const;
    Vuelo crear(const Vuelo &vuelo);
    bool asignarAvion(int idVuelo, int idAvion);
    bool cancelar(int idVuelo, const std::string &motivo);
    bool reprogramar(int idVuelo, const std::string &nuevaSalida, const std::string &nuevaLlegada);
    std::vector<Vuelo> listarPorAvion(int idAvion) const;
};

#endif
