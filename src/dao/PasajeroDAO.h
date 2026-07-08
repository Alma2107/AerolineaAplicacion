#ifndef PASAJERO_DAO_H
#define PASAJERO_DAO_H

#include "../modelos/Pasajero.h"
#include "ConexionDB.h"
#include <vector>

class PasajeroDAO
{
private:
    ConexionDB db;

public:
    PasajeroDAO();
    std::vector<Pasajero> listar() const;
    Pasajero buscarPorDocumento(const std::string &documento) const;
    Pasajero registrar(const std::string &tipoDocumento, const std::string &numeroDocumento,
                       const std::string &nombre, const std::string &apellido,
                       const std::string &fechaNacimiento, bool asistenciaEspecial,
                       const std::string &detallesMedicos);
};

#endif
