#ifndef EMPLEADO_DAO_H
#define EMPLEADO_DAO_H

#include "../modelos/Empleado.h"
#include "ConexionDB.h"
#include <vector>

class EmpleadoDAO
{
private:
    ConexionDB db;

public:
    EmpleadoDAO();
    bool validarAcceso(int modulo, const std::string &usuario, const std::string &password) const;
    Empleado buscarPorModulo(int modulo) const;
    std::vector<Empleado> listar() const;
};

#endif
