#ifndef OBJETO_PERDIDO_DAO_H
#define OBJETO_PERDIDO_DAO_H

#include "ConexionDB.h"
#include <string>

class ObjetoPerdidoDAO
{
private:
    ConexionDB db;

public:
    ObjetoPerdidoDAO();
    int registrar(const std::string &tipo, const std::string &descripcion, int idAvion,
                  const std::string &numeroAsiento, const std::string &codigoEquipaje,
                  const std::string &fechaHallazgo, const std::string &ubicacionExacta,
                  const std::string &empleadoHallazgo);
};

#endif
