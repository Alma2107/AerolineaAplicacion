#include "ReservaDAO.h"
#include "PasajeroDAO.h"
#include <sstream>

ReservaDAO::ReservaDAO()
{
    db.inicializar();
}

std::vector<Reserva> ReservaDAO::listar() const
{
    std::vector<Reserva> reservas;
    auto filas = db.consultar(
        "SELECT t.id_ticket,p.id_pasajero,v.id_vuelo,CONCAT(p.nombre,' ',p.apellido),p.numero_documento,"
        "v.numero_vuelo,v.origen_iata,v.destino_iata,v.fecha_salida,COALESCE(t.numero_asiento,''),"
        "t.codigo_reserva_pnr,t.precio_tramo_pagado "
        "FROM tickets_detalle t "
        "JOIN pasajeros p ON p.id_pasajero=t.id_pasajero "
        "JOIN vuelos v ON v.id_vuelo=t.id_vuelo "
        "ORDER BY t.id_ticket DESC LIMIT 12");
    for (const auto &fila : filas)
    {
        if (fila.size() >= 12)
        {
            reservas.push_back(Reserva(
                ConexionDB::convertirEntero(fila[0]),
                ConexionDB::convertirEntero(fila[1]),
                ConexionDB::convertirEntero(fila[2]),
                fila[3], fila[4], fila[5], fila[6], fila[7], fila[8], fila[9], fila[10],
                ConexionDB::convertirDouble(fila[11])));
        }
    }
    return reservas;
}

Reserva ReservaDAO::buscarPorQR(const std::string &codigoQR) const
{
    auto filas = db.consultar(
        "SELECT t.id_ticket,p.id_pasajero,v.id_vuelo,CONCAT(p.nombre,' ',p.apellido),p.numero_documento,"
        "v.numero_vuelo,v.origen_iata,v.destino_iata,v.fecha_salida,COALESCE(t.numero_asiento,''),"
        "t.codigo_reserva_pnr,t.precio_tramo_pagado "
        "FROM tickets_detalle t "
        "JOIN pasajeros p ON p.id_pasajero=t.id_pasajero "
        "JOIN vuelos v ON v.id_vuelo=t.id_vuelo "
        "WHERE t.codigo_reserva_pnr='" + ConexionDB::escapar(codigoQR) + "' LIMIT 1");
    if (!filas.empty() && filas[0].size() >= 12)
    {
        return Reserva(
            ConexionDB::convertirEntero(filas[0][0]),
            ConexionDB::convertirEntero(filas[0][1]),
            ConexionDB::convertirEntero(filas[0][2]),
            filas[0][3], filas[0][4], filas[0][5], filas[0][6], filas[0][7], filas[0][8],
            filas[0][9], filas[0][10], ConexionDB::convertirDouble(filas[0][11]));
    }
    return Reserva();
}

std::string ReservaDAO::validarReservaPresencial(const Pasajero &pasajero, int idCliente, int idVuelo,
                                                 const std::string &asiento, int idPlan,
                                                 const std::string &codigoQR, double precio,
                                                 int idMetodoPago) const
{
    if (pasajero.getNumeroDocumento().empty() || pasajero.getNombre().empty() || pasajero.getApellido().empty())
        return "Faltan datos obligatorios del pasajero.";
    if (pasajero.getFechaNacimiento().empty())
        return "Debe indicar la fecha de nacimiento.";
    if (idCliente <= 0)
        return "El id_cliente de la compra debe ser mayor a cero.";
    if (idVuelo <= 0 || idPlan <= 0 || idMetodoPago <= 0)
        return "Vuelo, plan tarifario y metodo de pago son obligatorios.";
    if (asiento.empty())
        return "Debe asignar un asiento.";
    if (codigoQR.empty())
        return "Debe generar o ingresar un codigo QR/PNR.";
    if (precio <= 0)
        return "El precio pagado debe ser mayor a cero.";

    auto vuelo = db.consultar("SELECT COUNT(*) FROM vuelos WHERE id_vuelo=" + std::to_string(idVuelo) + " AND estado_vuelo<>'Cancelado'");
    if (vuelo.empty() || vuelo[0].empty() || ConexionDB::convertirEntero(vuelo[0][0]) == 0)
        return "El vuelo no existe o se encuentra cancelado.";

    auto cliente = db.consultar("SELECT COUNT(*) FROM clientes WHERE id_cliente=" + std::to_string(idCliente) + " AND estado_cuenta=1");
    if (cliente.empty() || cliente[0].empty() || ConexionDB::convertirEntero(cliente[0][0]) == 0)
        return "El cliente de la orden no existe o no esta activo.";

    auto plan = db.consultar("SELECT COUNT(*) FROM planes_tarifas WHERE id_plan=" + std::to_string(idPlan));
    if (plan.empty() || plan[0].empty() || ConexionDB::convertirEntero(plan[0][0]) == 0)
        return "El plan tarifario no existe.";

    auto metodo = db.consultar("SELECT COUNT(*) FROM metodos_pago WHERE id_metodo_pago=" + std::to_string(idMetodoPago));
    if (metodo.empty() || metodo[0].empty() || ConexionDB::convertirEntero(metodo[0][0]) == 0)
        return "El metodo de pago no existe.";

    auto qr = db.consultar("SELECT COUNT(*) FROM tickets_detalle WHERE codigo_reserva_pnr='" + ConexionDB::escapar(codigoQR) + "'");
    if (!qr.empty() && !qr[0].empty() && ConexionDB::convertirEntero(qr[0][0]) > 0)
        return "El codigo QR/PNR ya existe.";

    return "OK";
}

Reserva ReservaDAO::crearReservaPresencial(const Pasajero &pasajero, int idCliente, int idVuelo,
                                           const std::string &asiento, int idPlan,
                                           const std::string &codigoQR, double precio,
                                           int idMetodoPago)
{
    std::string validacion = validarReservaPresencial(pasajero, idCliente, idVuelo, asiento, idPlan, codigoQR, precio, idMetodoPago);
    if (validacion != "OK")
        return Reserva();

    PasajeroDAO pasajeroDAO;
    Pasajero guardado = pasajeroDAO.registrar(
        pasajero.getTipoDocumento(), pasajero.getNumeroDocumento(), pasajero.getNombre(),
        pasajero.getApellido(), pasajero.getFechaNacimiento(), pasajero.requiereAsistenciaEspecial(),
        pasajero.getDetallesMedicos());
    if (guardado.getId() == 0)
        return Reserva();

    std::stringstream orden;
    orden << "INSERT INTO compras_ordenes (id_cliente,fecha_compra,monto_total_pagado,id_metodo_pago) VALUES ("
          << idCliente << ",NOW()," << precio << "," << idMetodoPago << ")";
    if (!db.ejecutar(orden.str()))
        return Reserva();

    auto idOrden = db.consultar("SELECT id_orden FROM compras_ordenes ORDER BY id_orden DESC LIMIT 1");
    if (idOrden.empty() || idOrden[0].empty())
        return Reserva();

    std::stringstream ticket;
    ticket << "INSERT INTO tickets_detalle (id_orden,id_vuelo,id_pasajero,numero_asiento,id_plan,codigo_reserva_pnr,precio_tramo_pagado) VALUES ("
           << ConexionDB::convertirEntero(idOrden[0][0]) << "," << idVuelo << "," << guardado.getId() << ",'"
           << ConexionDB::escapar(asiento) << "'," << idPlan << ",'" << ConexionDB::escapar(codigoQR) << "'," << precio << ")";
    if (!db.ejecutar(ticket.str()))
        return Reserva();

    return buscarPorQR(codigoQR);
}

bool ReservaDAO::registrarCheckIn(const std::string &codigoQR)
{
    Reserva reserva = buscarPorQR(codigoQR);
    if (reserva.getIdTicket() == 0)
        return false;

    auto existe = db.consultar("SELECT COUNT(*) FROM checkins WHERE id_ticket=" + std::to_string(reserva.getIdTicket()));
    if (!existe.empty() && !existe[0].empty() && ConexionDB::convertirEntero(existe[0][0]) > 0)
        return true;

    std::stringstream sql;
    sql << "INSERT INTO checkins (id_ticket,codigo_reserva_pnr,fecha,estado) VALUES ("
        << reserva.getIdTicket() << ",'" << ConexionDB::escapar(codigoQR) << "',NOW(),'Realizado')";
    return db.ejecutar(sql.str());
}

bool ReservaDAO::cancelar(int idTicket)
{
    return db.ejecutar("DELETE FROM tickets_detalle WHERE id_ticket=" + std::to_string(idTicket));
}

bool ReservaDAO::reembolsar(int idTicket)
{
    return db.ejecutar("UPDATE tickets_detalle SET precio_tramo_pagado=0, numero_asiento='REEMB' WHERE id_ticket=" + std::to_string(idTicket));
}
