#include "ReservaDAO.h"
#include "PasajeroDAO.h"
#include <ctime>
#include <random>
#include <set>
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

Reserva ReservaDAO::buscarPorCodigoReserva(const std::string &codigoReserva) const
{
    auto filas = db.consultar(
        "SELECT t.id_ticket,p.id_pasajero,v.id_vuelo,CONCAT(p.nombre,' ',p.apellido),p.numero_documento,"
        "v.numero_vuelo,v.origen_iata,v.destino_iata,v.fecha_salida,COALESCE(t.numero_asiento,''),"
        "t.codigo_reserva_pnr,t.precio_tramo_pagado "
        "FROM tickets_detalle t "
        "JOIN pasajeros p ON p.id_pasajero=t.id_pasajero "
        "JOIN vuelos v ON v.id_vuelo=t.id_vuelo "
        "WHERE t.codigo_reserva_pnr='" + ConexionDB::escapar(codigoReserva) + "' LIMIT 1");
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

std::string ReservaDAO::generarCodigoReserva() const
{
    static const std::string caracteres = "ABCDEFGHJKLMNPSTUVWXYZ23456789";
    static std::mt19937 generador((unsigned int)std::time(nullptr));

    for (int intento = 0; intento < 40; ++intento)
    {
        std::stringstream codigo;
        for (int i = 0; i < 6; ++i)
            codigo << caracteres[generador() % caracteres.size()];

        auto existente = db.consultar("SELECT COUNT(*) FROM tickets_detalle WHERE codigo_reserva_pnr='" + ConexionDB::escapar(codigo.str()) + "'");
        if (existente.empty() || existente[0].empty() || ConexionDB::convertirEntero(existente[0][0]) == 0)
            return codigo.str();
    }

    auto siguiente = db.consultar("SELECT COALESCE(MAX(id_ticket),0)+1 FROM tickets_detalle");
    int numero = siguiente.empty() || siguiente[0].empty() ? 1 : ConexionDB::convertirEntero(siguiente[0][0], 1);
    std::stringstream fallback;
    fallback << "P" << numero;
    return fallback.str();
}

int ReservaDAO::contarAsientosDisponibles(int idVuelo) const
{
    auto filas = db.consultar(
        "SELECT a.capacidad,"
        "SUM(CASE WHEN t.id_ticket IS NOT NULL AND t.precio_tramo_pagado>0 AND COALESCE(t.numero_asiento,'')<>'REEMB' THEN 1 ELSE 0 END) "
        "FROM vuelos v "
        "JOIN aviones a ON a.id_avion=v.id_avion "
        "LEFT JOIN tickets_detalle t ON t.id_vuelo=v.id_vuelo "
        "WHERE v.id_vuelo=" +
        std::to_string(idVuelo) + " AND v.estado_vuelo<>'Cancelado' "
                                "GROUP BY a.capacidad");
    if (filas.empty() || filas[0].size() < 2)
        return -1;

    int capacidad = ConexionDB::convertirEntero(filas[0][0]);
    int vendidos = ConexionDB::convertirEntero(filas[0][1]);
    return capacidad - vendidos;
}

std::vector<CatalogoItem> ReservaDAO::listarPlanes() const
{
    std::vector<CatalogoItem> items;
    auto filas = db.consultar("SELECT id_plan,nombre_plan,cargo_extra_plan FROM planes_tarifas ORDER BY id_plan");
    for (const auto &fila : filas)
        if (fila.size() >= 3)
            items.push_back({ConexionDB::convertirEntero(fila[0]), fila[1], ConexionDB::convertirDouble(fila[2])});
    return items;
}

std::vector<CatalogoItem> ReservaDAO::listarTiposEquipaje() const
{
    std::vector<CatalogoItem> items;
    auto filas = db.consultar("SELECT id_tipo_equipaje,nombre_tipo,precio_unitario FROM tipos_equipaje ORDER BY id_tipo_equipaje");
    for (const auto &fila : filas)
        if (fila.size() >= 3)
            items.push_back({ConexionDB::convertirEntero(fila[0]), fila[1], ConexionDB::convertirDouble(fila[2])});
    return items;
}

std::vector<CatalogoItem> ReservaDAO::listarServicios() const
{
    std::vector<CatalogoItem> items;
    auto filas = db.consultar("SELECT id_servicio,nombre_servicio,precio_servicio FROM servicios_adicionales ORDER BY id_servicio");
    for (const auto &fila : filas)
        if (fila.size() >= 3)
            items.push_back({ConexionDB::convertirEntero(fila[0]), fila[1], ConexionDB::convertirDouble(fila[2])});
    return items;
}

std::vector<CatalogoItem> ReservaDAO::listarVuelosDisponibles() const
{
    std::vector<CatalogoItem> items;
    auto filas = db.consultar(
        "SELECT v.id_vuelo,CONCAT(v.numero_vuelo,' ',v.origen_iata,'-',v.destino_iata,' ',DATE_FORMAT(v.fecha_salida,'%Y-%m-%d %H:%i')),v.precio_base_vuelo "
        "FROM vuelos v JOIN aviones a ON a.id_avion=v.id_avion "
        "WHERE v.estado_vuelo<>'Cancelado' AND a.estado='Activo' AND v.fecha_salida>=NOW() "
        "ORDER BY v.fecha_salida LIMIT 18");
    for (const auto &fila : filas)
        if (fila.size() >= 3)
            items.push_back({ConexionDB::convertirEntero(fila[0]), fila[1], ConexionDB::convertirDouble(fila[2])});
    return items;
}

std::vector<std::string> ReservaDAO::listarAsientosDisponibles(int idVuelo) const
{
    std::vector<std::string> asientos;
    auto datos = db.consultar("SELECT a.capacidad FROM vuelos v JOIN aviones a ON a.id_avion=v.id_avion WHERE v.id_vuelo=" + std::to_string(idVuelo) + " LIMIT 1");
    if (datos.empty() || datos[0].empty())
        return asientos;

    std::set<std::string> ocupados;
    auto filasOcupadas = db.consultar("SELECT UPPER(TRIM(numero_asiento)) FROM tickets_detalle WHERE id_vuelo=" + std::to_string(idVuelo) + " AND precio_tramo_pagado>0 AND COALESCE(numero_asiento,'')<>'' AND numero_asiento<>'REEMB'");
    for (const auto &fila : filasOcupadas)
        if (!fila.empty())
            ocupados.insert(fila[0]);

    int capacidad = ConexionDB::convertirEntero(datos[0][0]);
    const char columnas[] = {'A', 'B', 'C', 'D', 'E', 'F'};
    for (int i = 0; i < capacidad; ++i)
    {
        std::stringstream asiento;
        asiento << (i / 6 + 1) << columnas[i % 6];
        if (ocupados.find(asiento.str()) == ocupados.end())
            asientos.push_back(asiento.str());
        if ((int)asientos.size() >= 36)
            break;
    }
    return asientos;
}

std::string ReservaDAO::validarReservaPresencial(const Pasajero &pasajero, int idCliente, int idVuelo,
                                                 const std::string &asiento, int idPlan,
                                                 const std::string &codigoReserva, double precio,
                                                 int idMetodoPago, int idTipoEquipaje,
                                                 int cantidadEquipaje, double precioEquipaje,
                                                 int idServicio, double precioServicio) const
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
    if (codigoReserva.empty())
        return "Debe generar o ingresar un codigo de reserva.";
    if (precio <= 0)
        return "El precio pagado debe ser mayor a cero.";

    auto vuelo = db.consultar("SELECT COUNT(*) FROM vuelos WHERE id_vuelo=" + std::to_string(idVuelo) + " AND estado_vuelo<>'Cancelado'");
    if (vuelo.empty() || vuelo[0].empty() || ConexionDB::convertirEntero(vuelo[0][0]) == 0)
        return "El vuelo no existe o se encuentra cancelado.";

    int disponibles = contarAsientosDisponibles(idVuelo);
    if (disponibles <= 0)
        return "No quedan asientos libres para ese vuelo. La compra presencial debe pasarse a otro dia/vuelo.";

    auto asientoOcupado = db.consultar(
        "SELECT COUNT(*) FROM tickets_detalle "
        "WHERE id_vuelo=" +
        std::to_string(idVuelo) +
        " AND precio_tramo_pagado>0 "
        "AND UPPER(TRIM(COALESCE(numero_asiento,'')))=UPPER(TRIM('" +
        ConexionDB::escapar(asiento) + "'))");
    if (!asientoOcupado.empty() && !asientoOcupado[0].empty() && ConexionDB::convertirEntero(asientoOcupado[0][0]) > 0)
        return "Ese asiento ya esta ocupado para el vuelo elegido.";

    auto cliente = db.consultar("SELECT COUNT(*) FROM clientes WHERE id_cliente=" + std::to_string(idCliente) + " AND estado_cuenta=1");
    if (cliente.empty() || cliente[0].empty() || ConexionDB::convertirEntero(cliente[0][0]) == 0)
        return "El cliente de la orden no existe o no esta activo.";

    auto plan = db.consultar("SELECT COUNT(*) FROM planes_tarifas WHERE id_plan=" + std::to_string(idPlan));
    if (plan.empty() || plan[0].empty() || ConexionDB::convertirEntero(plan[0][0]) == 0)
        return "El plan tarifario no existe.";

    auto metodo = db.consultar("SELECT COUNT(*) FROM metodos_pago WHERE id_metodo_pago=" + std::to_string(idMetodoPago));
    if (metodo.empty() || metodo[0].empty() || ConexionDB::convertirEntero(metodo[0][0]) == 0)
        return "El metodo de pago no existe.";

    auto reservaExistente = db.consultar("SELECT COUNT(*) FROM tickets_detalle WHERE codigo_reserva_pnr='" + ConexionDB::escapar(codigoReserva) + "'");
    if (!reservaExistente.empty() && !reservaExistente[0].empty() && ConexionDB::convertirEntero(reservaExistente[0][0]) > 0)
        return "El codigo de reserva ya existe.";

    if (idTipoEquipaje > 0)
    {
        if (cantidadEquipaje <= 0)
            return "La cantidad de equipaje debe ser mayor a cero.";
        if (precioEquipaje < 0)
            return "El precio de equipaje no puede ser negativo.";
        auto tipoEquipaje = db.consultar("SELECT COUNT(*) FROM tipos_equipaje WHERE id_tipo_equipaje=" + std::to_string(idTipoEquipaje));
        if (tipoEquipaje.empty() || tipoEquipaje[0].empty() || ConexionDB::convertirEntero(tipoEquipaje[0][0]) == 0)
            return "El tipo de equipaje no existe.";
    }

    if (idServicio > 0)
    {
        if (precioServicio < 0)
            return "El precio del servicio no puede ser negativo.";
        auto servicio = db.consultar("SELECT COUNT(*) FROM servicios_adicionales WHERE id_servicio=" + std::to_string(idServicio));
        if (servicio.empty() || servicio[0].empty() || ConexionDB::convertirEntero(servicio[0][0]) == 0)
            return "El servicio adicional no existe.";
    }

    return "OK";
}

Reserva ReservaDAO::crearReservaPresencial(const Pasajero &pasajero, int idCliente, int idVuelo,
                                           const std::string &asiento, int idPlan,
                                           const std::string &codigoReserva, double precio,
                                           int idMetodoPago, int idTipoEquipaje,
                                           int cantidadEquipaje, double precioEquipaje,
                                           int idServicio, double precioServicio)
{
    std::string validacion = validarReservaPresencial(pasajero, idCliente, idVuelo, asiento, idPlan, codigoReserva, precio,
                                                       idMetodoPago, idTipoEquipaje, cantidadEquipaje, precioEquipaje,
                                                       idServicio, precioServicio);
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
          << idCliente << ",NOW()," << (precio + precioEquipaje + precioServicio) << "," << idMetodoPago << ")";
    if (!db.ejecutar(orden.str()))
        return Reserva();

    auto idOrden = db.consultar("SELECT id_orden FROM compras_ordenes ORDER BY id_orden DESC LIMIT 1");
    if (idOrden.empty() || idOrden[0].empty())
        return Reserva();

    std::stringstream ticket;
    ticket << "INSERT INTO tickets_detalle (id_orden,id_vuelo,id_pasajero,numero_asiento,id_plan,codigo_reserva_pnr,precio_tramo_pagado) VALUES ("
           << ConexionDB::convertirEntero(idOrden[0][0]) << "," << idVuelo << "," << guardado.getId() << ",'"
           << ConexionDB::escapar(asiento) << "'," << idPlan << ",'" << ConexionDB::escapar(codigoReserva) << "'," << precio << ")";
    if (!db.ejecutar(ticket.str()))
        return Reserva();

    Reserva reserva = buscarPorCodigoReserva(codigoReserva);
    if (reserva.getIdTicket() == 0)
        return Reserva();

    if (idTipoEquipaje > 0)
    {
        std::stringstream etiqueta;
        etiqueta << "TAG-" << reserva.getIdTicket();
        std::stringstream equipaje;
        equipaje << "INSERT INTO ticket_equipajes (id_ticket,id_tipo_equipaje,codigo_etiqueta,cantidad,precio_pagado,estado_equipaje) VALUES ("
                 << reserva.getIdTicket() << "," << idTipoEquipaje << ",'" << etiqueta.str() << "',"
                 << cantidadEquipaje << "," << precioEquipaje << ",'Registrado')";
        db.ejecutar(equipaje.str());
    }

    if (idServicio > 0)
    {
        std::stringstream servicio;
        servicio << "INSERT INTO ticket_servicios (id_ticket,id_servicio,precio_servicio_pagado) VALUES ("
                 << reserva.getIdTicket() << "," << idServicio << "," << precioServicio << ")";
        db.ejecutar(servicio.str());
    }

    return reserva;
}

Reserva ReservaDAO::crearReservaPresencial(const Pasajero &pasajero, int idCliente, int idVuelo,
                                           const std::string &asiento, int idPlan,
                                           int idMetodoPago, const std::vector<int> &tiposEquipaje,
                                           const std::vector<int> &cantidadesEquipaje,
                                           const std::vector<int> &servicios)
{
    double precioBase = 0;
    auto vuelo = db.consultar("SELECT precio_base_vuelo FROM vuelos WHERE id_vuelo=" + std::to_string(idVuelo) + " LIMIT 1");
    if (!vuelo.empty() && !vuelo[0].empty())
        precioBase = ConexionDB::convertirDouble(vuelo[0][0]);

    std::string codigoReserva = generarCodigoReserva();
    std::string validacion = validarReservaPresencial(pasajero, idCliente, idVuelo, asiento, idPlan, codigoReserva, precioBase, idMetodoPago);
    if (validacion != "OK")
        return Reserva();

    double totalEquipaje = 0;
    for (int i = 0; i < (int)tiposEquipaje.size(); ++i)
    {
        int cantidad = i < (int)cantidadesEquipaje.size() ? cantidadesEquipaje[i] : 1;
        if (cantidad <= 0)
            cantidad = 1;
        auto tipo = db.consultar("SELECT precio_unitario FROM tipos_equipaje WHERE id_tipo_equipaje=" + std::to_string(tiposEquipaje[i]) + " LIMIT 1");
        if (tipo.empty() || tipo[0].empty())
            return Reserva();
        totalEquipaje += ConexionDB::convertirDouble(tipo[0][0]) * cantidad;
    }

    double totalServicios = 0;
    for (int idServicio : servicios)
    {
        auto servicio = db.consultar("SELECT precio_servicio FROM servicios_adicionales WHERE id_servicio=" + std::to_string(idServicio) + " LIMIT 1");
        if (servicio.empty() || servicio[0].empty())
            return Reserva();
        totalServicios += ConexionDB::convertirDouble(servicio[0][0]);
    }

    PasajeroDAO pasajeroDAO;
    Pasajero guardado = pasajeroDAO.registrar(
        pasajero.getTipoDocumento(), pasajero.getNumeroDocumento(), pasajero.getNombre(),
        pasajero.getApellido(), pasajero.getFechaNacimiento(), pasajero.requiereAsistenciaEspecial(),
        pasajero.getDetallesMedicos());
    if (guardado.getId() == 0)
        return Reserva();

    std::stringstream orden;
    orden << "INSERT INTO compras_ordenes (id_cliente,fecha_compra,monto_total_pagado,id_metodo_pago) VALUES ("
          << idCliente << ",NOW()," << (precioBase + totalEquipaje + totalServicios) << "," << idMetodoPago << ")";
    if (!db.ejecutar(orden.str()))
        return Reserva();

    auto idOrden = db.consultar("SELECT id_orden FROM compras_ordenes ORDER BY id_orden DESC LIMIT 1");
    if (idOrden.empty() || idOrden[0].empty())
        return Reserva();

    std::stringstream ticket;
    ticket << "INSERT INTO tickets_detalle (id_orden,id_vuelo,id_pasajero,numero_asiento,id_plan,codigo_reserva_pnr,precio_tramo_pagado) VALUES ("
           << ConexionDB::convertirEntero(idOrden[0][0]) << "," << idVuelo << "," << guardado.getId() << ",'"
           << ConexionDB::escapar(asiento) << "'," << idPlan << ",'" << ConexionDB::escapar(codigoReserva) << "'," << precioBase << ")";
    if (!db.ejecutar(ticket.str()))
        return Reserva();

    Reserva reserva = buscarPorCodigoReserva(codigoReserva);
    if (reserva.getIdTicket() == 0)
        return Reserva();

    for (int i = 0; i < (int)tiposEquipaje.size(); ++i)
    {
        int cantidad = i < (int)cantidadesEquipaje.size() ? cantidadesEquipaje[i] : 1;
        if (cantidad <= 0)
            cantidad = 1;
        auto tipo = db.consultar("SELECT precio_unitario FROM tipos_equipaje WHERE id_tipo_equipaje=" + std::to_string(tiposEquipaje[i]) + " LIMIT 1");
        double precioUnitario = (!tipo.empty() && !tipo[0].empty()) ? ConexionDB::convertirDouble(tipo[0][0]) : 0;
        std::stringstream etiqueta;
        etiqueta << "TAG-" << reserva.getIdTicket() << "-" << (i + 1);
        std::stringstream equipaje;
        equipaje << "INSERT INTO ticket_equipajes (id_ticket,id_tipo_equipaje,codigo_etiqueta,cantidad,precio_pagado,estado_equipaje) VALUES ("
                 << reserva.getIdTicket() << "," << tiposEquipaje[i] << ",'" << etiqueta.str() << "',"
                 << cantidad << "," << (precioUnitario * cantidad) << ",'Registrado')";
        db.ejecutar(equipaje.str());
    }

    for (int idServicio : servicios)
    {
        auto servicio = db.consultar("SELECT precio_servicio FROM servicios_adicionales WHERE id_servicio=" + std::to_string(idServicio) + " LIMIT 1");
        double precioServicio = (!servicio.empty() && !servicio[0].empty()) ? ConexionDB::convertirDouble(servicio[0][0]) : 0;
        std::stringstream sqlServicio;
        sqlServicio << "INSERT INTO ticket_servicios (id_ticket,id_servicio,precio_servicio_pagado) VALUES ("
                    << reserva.getIdTicket() << "," << idServicio << "," << precioServicio << ")";
        db.ejecutar(sqlServicio.str());
    }

    return reserva;
}

bool ReservaDAO::registrarCheckIn(const std::string &codigoReserva)
{
    Reserva reserva = buscarPorCodigoReserva(codigoReserva);
    if (reserva.getIdTicket() == 0)
        return false;

    auto existe = db.consultar("SELECT COUNT(*) FROM checkins WHERE id_ticket=" + std::to_string(reserva.getIdTicket()));
    if (!existe.empty() && !existe[0].empty() && ConexionDB::convertirEntero(existe[0][0]) > 0)
        return true;

    std::stringstream sql;
    sql << "INSERT INTO checkins (id_ticket,codigo_reserva_pnr,fecha,estado) VALUES ("
        << reserva.getIdTicket() << ",'" << ConexionDB::escapar(codigoReserva) << "',NOW(),'Realizado')";
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
