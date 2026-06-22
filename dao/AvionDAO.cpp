#include "AvionDAO.h"
#include <sstream>

AvionDAO::AvionDAO()
{
    db.inicializar();
}

std::vector<Avion> AvionDAO::listar() const
{
    std::vector<Avion> aviones;
    auto filas = db.consultar("SELECT id_avion, modelo, capacidad, estado FROM aviones ORDER BY id_avion");
    for (const auto &fila : filas)
    {
        if (fila.size() >= 4)
            aviones.push_back(Avion(ConexionDB::convertirEntero(fila[0]), fila[1], ConexionDB::convertirEntero(fila[2]), fila[3]));
    }
    return aviones;
}

std::vector<Avion> AvionDAO::listarDisponibles() const
{
    std::vector<Avion> aviones;
    auto filas = db.consultar("SELECT id_avion, modelo, capacidad, estado FROM aviones WHERE estado='Activo' ORDER BY id_avion");
    for (const auto &fila : filas)
    {
        if (fila.size() >= 4)
            aviones.push_back(Avion(ConexionDB::convertirEntero(fila[0]), fila[1], ConexionDB::convertirEntero(fila[2]), fila[3]));
    }
    return aviones;
}

Avion AvionDAO::buscarPorId(int id) const
{
    auto filas = db.consultar("SELECT id_avion, modelo, capacidad, estado FROM aviones WHERE id_avion=" + std::to_string(id) + " LIMIT 1");
    if (!filas.empty() && filas[0].size() >= 4)
        return Avion(ConexionDB::convertirEntero(filas[0][0]), filas[0][1], ConexionDB::convertirEntero(filas[0][2]), filas[0][3]);
    return Avion();
}

Avion AvionDAO::crear(const std::string &modelo, int capacidad, const std::string &estado)
{
    if (modelo.empty() || capacidad <= 0 || estado.empty())
        return Avion();

    std::stringstream sql;
    sql << "INSERT INTO aviones (modelo,capacidad,estado) VALUES ('"
        << ConexionDB::escapar(modelo) << "'," << capacidad << ",'"
        << ConexionDB::escapar(estado) << "')";

    if (!db.ejecutar(sql.str()))
        return Avion();

    auto filas = db.consultar("SELECT id_avion, modelo, capacidad, estado FROM aviones ORDER BY id_avion DESC LIMIT 1");
    if (!filas.empty() && filas[0].size() >= 4)
        return Avion(ConexionDB::convertirEntero(filas[0][0]), filas[0][1], ConexionDB::convertirEntero(filas[0][2]), filas[0][3]);
    return Avion();
}

bool AvionDAO::actualizarEstado(int id, const std::string &estado)
{
    std::stringstream sql;
    sql << "UPDATE aviones SET estado='" << ConexionDB::escapar(estado) << "' WHERE id_avion=" << id;
    return db.ejecutar(sql.str());
}
