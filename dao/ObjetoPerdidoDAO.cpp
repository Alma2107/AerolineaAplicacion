#include "ObjetoPerdidoDAO.h"
#include <sstream>

ObjetoPerdidoDAO::ObjetoPerdidoDAO()
{
    db.inicializar();
}

int ObjetoPerdidoDAO::registrar(const std::string &tipo, const std::string &descripcion, int idAvion,
                                const std::string &numeroAsiento, const std::string &codigoEquipaje,
                                const std::string &fechaHallazgo, const std::string &ubicacionExacta,
                                const std::string &empleadoHallazgo)
{
    if (tipo.empty() || descripcion.empty() || idAvion <= 0 || fechaHallazgo.empty() || ubicacionExacta.empty() || empleadoHallazgo.empty())
        return 0;

    std::stringstream sql;
    sql << "INSERT INTO objetos_perdidos (tipo,descripcion,id_avion,numero_asiento,codigo_equipaje,fecha_hallazgo,ubicacion_exacta,empleado_hallazgo,estado) VALUES ('"
        << ConexionDB::escapar(tipo) << "','"
        << ConexionDB::escapar(descripcion) << "',"
        << idAvion << ",'"
        << ConexionDB::escapar(numeroAsiento) << "','"
        << ConexionDB::escapar(codigoEquipaje) << "','"
        << ConexionDB::escapar(fechaHallazgo) << "','"
        << ConexionDB::escapar(ubicacionExacta) << "','"
        << ConexionDB::escapar(empleadoHallazgo) << "','Pendiente verificacion')";
    if (!db.ejecutar(sql.str()))
        return 0;

    auto filas = db.consultar("SELECT id_objeto FROM objetos_perdidos ORDER BY id_objeto DESC LIMIT 1");
    if (!filas.empty() && !filas[0].empty())
        return ConexionDB::convertirEntero(filas[0][0]);
    return 0;
}
