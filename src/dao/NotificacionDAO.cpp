#include "NotificacionDAO.h"
#include <sstream>

NotificacionDAO::NotificacionDAO()
{
    db.inicializar();
}

bool NotificacionDAO::crear(int moduloDestino, const std::string &tipo, const std::string &mensaje)
{
    std::stringstream sql;
    sql << "INSERT INTO notificaciones (modulo_destino,tipo,mensaje,leida,fecha) VALUES ("
        << moduloDestino << ",'" << ConexionDB::escapar(tipo) << "','"
        << ConexionDB::escapar(mensaje) << "',0,NOW())";
    return db.ejecutar(sql.str());
}

std::vector<Notificacion> NotificacionDAO::listarPorModulo(int moduloDestino) const
{
    std::vector<Notificacion> notificaciones;
    auto filas = db.consultar("SELECT id_notificacion,modulo_destino,tipo,mensaje,leida,fecha FROM notificaciones WHERE modulo_destino=" + std::to_string(moduloDestino) + " ORDER BY id_notificacion DESC LIMIT 6");
    for (const auto &fila : filas)
    {
        if (fila.size() >= 6)
        {
            notificaciones.push_back(Notificacion(
                ConexionDB::convertirEntero(fila[0]),
                ConexionDB::convertirEntero(fila[1]),
                fila[2],
                fila[3],
                ConexionDB::convertirEntero(fila[4]) != 0,
                fila[5]));
        }
    }
    return notificaciones;
}

int NotificacionDAO::contarNoLeidas(int moduloDestino) const
{
    auto filas = db.consultar("SELECT COUNT(*) FROM notificaciones WHERE modulo_destino=" + std::to_string(moduloDestino) + " AND leida=0");
    if (!filas.empty() && !filas[0].empty())
        return ConexionDB::convertirEntero(filas[0][0]);
    return 0;
}

bool NotificacionDAO::marcarLeidas(int moduloDestino)
{
    return db.ejecutar("UPDATE notificaciones SET leida=1 WHERE modulo_destino=" + std::to_string(moduloDestino));
}
