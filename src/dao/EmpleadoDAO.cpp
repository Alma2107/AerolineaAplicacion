#include "EmpleadoDAO.h"
#include <sstream>

EmpleadoDAO::EmpleadoDAO()
{
    db.inicializar();
}

bool EmpleadoDAO::validarAcceso(int modulo, const std::string &usuario, const std::string &password) const
{
    auto total = db.consultar("SELECT COUNT(*) FROM empleados");
    if (total.empty() || total[0].empty() || ConexionDB::convertirEntero(total[0][0]) == 0)
    {
        if (modulo == 1)
            return usuario == "vuelos" && password == "vuelos123";
        if (modulo == 2)
            return usuario == "pasajeros" && password == "pasajeros123";
        if (modulo == 3)
            return usuario == "equipaje" && password == "equipaje123";
        if (modulo == 4)
            return usuario == "mantenimiento" && password == "mantenimiento123";
        return false;
    }

    std::stringstream sql;
    sql << "SELECT COUNT(*) FROM empleados WHERE modulo=" << modulo
        << " AND usuario='" << ConexionDB::escapar(usuario) << "'"
        << " AND password='" << ConexionDB::escapar(password) << "'";
    auto filas = db.consultar(sql.str());
    return !filas.empty() && !filas[0].empty() && ConexionDB::convertirEntero(filas[0][0]) > 0;
}

Empleado EmpleadoDAO::buscarPorModulo(int modulo) const
{
    auto filas = db.consultar("SELECT id_empleado,nombre,usuario,password,modulo FROM empleados WHERE modulo=" + std::to_string(modulo) + " LIMIT 1");
    if (!filas.empty() && filas[0].size() >= 5)
        return Empleado(ConexionDB::convertirEntero(filas[0][0]), filas[0][1], filas[0][2], filas[0][3], ConexionDB::convertirEntero(filas[0][4]));
    return Empleado();
}

std::vector<Empleado> EmpleadoDAO::listar() const
{
    std::vector<Empleado> empleados;
    auto filas = db.consultar("SELECT id_empleado,nombre,usuario,password,modulo FROM empleados ORDER BY modulo");
    for (const auto &fila : filas)
    {
        if (fila.size() >= 5)
            empleados.push_back(Empleado(ConexionDB::convertirEntero(fila[0]), fila[1], fila[2], fila[3], ConexionDB::convertirEntero(fila[4])));
    }
    return empleados;
}
