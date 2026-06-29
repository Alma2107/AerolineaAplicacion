#include "EquipajeDAO.h"
#include <sstream>

EquipajeDAO::EquipajeDAO()
{
    db.inicializar();
}

std::vector<Equipaje> EquipajeDAO::listar() const
{
    std::vector<Equipaje> equipajes;
    auto filas = db.consultar("SELECT id_ticket_equipaje,id_ticket,codigo_etiqueta,precio_pagado,estado_equipaje FROM ticket_equipajes ORDER BY id_ticket_equipaje DESC LIMIT 12");
    for (const auto &fila : filas)
    {
        if (fila.size() >= 5)
            equipajes.push_back(Equipaje(ConexionDB::convertirEntero(fila[0]), ConexionDB::convertirEntero(fila[1]), fila[2], ConexionDB::convertirDouble(fila[3]), fila[4]));
    }
    return equipajes;
}

Equipaje EquipajeDAO::registrar(int idTicket, int idTipoEquipaje, double peso, double precio)
{
    auto filas = db.consultar("SELECT COALESCE(MAX(id_ticket_equipaje),0)+1 FROM ticket_equipajes");
    int siguiente = (!filas.empty() && !filas[0].empty()) ? ConexionDB::convertirEntero(filas[0][0], 1) : 1;
    std::stringstream codigo;
    codigo << "TAG-" << (1000 + siguiente);

    std::stringstream sql;
    sql << "INSERT INTO ticket_equipajes (id_ticket,id_tipo_equipaje,codigo_etiqueta,cantidad,precio_pagado,estado_equipaje) VALUES ("
        << idTicket << "," << idTipoEquipaje << ",'" << codigo.str() << "',1," << precio << ",'Registrado')";
    if (!db.ejecutar(sql.str()))
        return Equipaje();

    return rastrear(codigo.str());
}

Equipaje EquipajeDAO::rastrear(const std::string &codigoEtiqueta) const
{
    auto filas = db.consultar("SELECT id_ticket_equipaje,id_ticket,codigo_etiqueta,precio_pagado,estado_equipaje FROM ticket_equipajes WHERE codigo_etiqueta='" + ConexionDB::escapar(codigoEtiqueta) + "' LIMIT 1");
    if (!filas.empty() && filas[0].size() >= 5)
        return Equipaje(ConexionDB::convertirEntero(filas[0][0]), ConexionDB::convertirEntero(filas[0][1]), filas[0][2], ConexionDB::convertirDouble(filas[0][3]), filas[0][4]);
    return Equipaje();
}

std::string EquipajeDAO::buscarPropietarioPorEtiqueta(const std::string &codigoEtiqueta) const
{
    auto filas = db.consultar(
        "SELECT CONCAT(p.nombre,' ',p.apellido), p.tipo_documento, p.numero_documento, t.codigo_reserva_pnr "
        "FROM ticket_equipajes te "
        "JOIN tickets_detalle t ON te.id_ticket=t.id_ticket "
        "JOIN pasajeros p ON t.id_pasajero=p.id_pasajero "
        "WHERE te.codigo_etiqueta='" + ConexionDB::escapar(codigoEtiqueta) + "' LIMIT 1");
    if (!filas.empty() && filas[0].size() >= 4)
    {
        std::stringstream propietario;
        propietario << filas[0][0] << " (" << filas[0][1] << " " << filas[0][2] << ") - Reserva " << filas[0][3];
        return propietario.str();
    }
    return std::string();
}

bool EquipajeDAO::actualizarEstado(const std::string &codigoEtiqueta, const std::string &estado)
{
    std::stringstream sql;
    sql << "UPDATE ticket_equipajes SET estado_equipaje='" << ConexionDB::escapar(estado)
        << "' WHERE codigo_etiqueta='" << ConexionDB::escapar(codigoEtiqueta) << "'";
    return db.ejecutar(sql.str());
}
