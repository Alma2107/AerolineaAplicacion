#ifndef NOTIFICACION_DAO_H
#define NOTIFICACION_DAO_H

#include "../modelos/Notificacion.h"
#include "ConexionDB.h"
#include <string>
#include <vector>

class NotificacionDAO
{
private:
    ConexionDB db;

public:
    NotificacionDAO();
    bool crear(int moduloDestino, const std::string &tipo, const std::string &mensaje);
    std::vector<Notificacion> listarPorModulo(int moduloDestino) const;
    int contarNoLeidas(int moduloDestino) const;
    bool marcarLeidas(int moduloDestino);
};

#endif
