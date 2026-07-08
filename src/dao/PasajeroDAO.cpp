#include "PasajeroDAO.h"
#include <cctype>
#include <sstream>

PasajeroDAO::PasajeroDAO()
{
    db.inicializar();
}

std::vector<Pasajero> PasajeroDAO::listar() const
{
    std::vector<Pasajero> pasajeros;
    auto filas = db.consultar("SELECT id_pasajero,tipo_documento,numero_documento,nombre,apellido,fecha_nacimiento,asistencia_especial,detalles_medicos FROM pasajeros ORDER BY id_pasajero DESC LIMIT 12");
    for (const auto &fila : filas)
    {
        if (fila.size() >= 8)
        {
            pasajeros.push_back(Pasajero(
                ConexionDB::convertirEntero(fila[0]), fila[1], fila[2], fila[3], fila[4],
                fila[5], ConexionDB::convertirEntero(fila[6]) != 0, fila[7], false));
        }
    }
    return pasajeros;
}

Pasajero PasajeroDAO::buscarPorDocumento(const std::string &documento) const
{
    auto filas = db.consultar("SELECT id_pasajero,tipo_documento,numero_documento,nombre,apellido,fecha_nacimiento,asistencia_especial,detalles_medicos FROM pasajeros WHERE numero_documento='" + ConexionDB::escapar(documento) + "' LIMIT 1");
    if (!filas.empty() && filas[0].size() >= 8)
    {
        return Pasajero(
            ConexionDB::convertirEntero(filas[0][0]), filas[0][1], filas[0][2], filas[0][3], filas[0][4],
            filas[0][5], ConexionDB::convertirEntero(filas[0][6]) != 0, filas[0][7], false);
    }
    return Pasajero();
}

Pasajero PasajeroDAO::registrar(const std::string &tipoDocumento, const std::string &numeroDocumento,
                                const std::string &nombre, const std::string &apellido,
                                const std::string &fechaNacimiento, bool asistenciaEspecial,
                                const std::string &detallesMedicos)
{
    if (tipoDocumento == "DNI")
    {
        if (numeroDocumento.empty() || numeroDocumento.size() > 8)
            return Pasajero();
        for (char c : numeroDocumento)
            if (!std::isdigit((unsigned char)c))
                return Pasajero();
    }

    Pasajero existente = buscarPorDocumento(numeroDocumento);
    if (existente.getId() != 0)
        return existente;

    std::stringstream sql;
    sql << "INSERT INTO pasajeros (tipo_documento,numero_documento,nombre,apellido,fecha_nacimiento,asistencia_especial,detalles_medicos) VALUES ('"
        << ConexionDB::escapar(tipoDocumento) << "','"
        << ConexionDB::escapar(numeroDocumento) << "','"
        << ConexionDB::escapar(nombre) << "','"
        << ConexionDB::escapar(apellido) << "','"
        << ConexionDB::escapar(fechaNacimiento) << "',"
        << (asistenciaEspecial ? 1 : 0) << ",'"
        << ConexionDB::escapar(detallesMedicos) << "')";
    if (!db.ejecutar(sql.str()))
        return Pasajero();

    return buscarPorDocumento(numeroDocumento);
}
