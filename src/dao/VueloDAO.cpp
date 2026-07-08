#include "VueloDAO.h"
#include "AvionDAO.h"
#include <ctime>
#include <iomanip>
#include <sstream>

static bool parseFechaMysql(const std::string &valor, std::tm &salida)
{
    std::stringstream ss(valor);
    ss >> std::get_time(&salida, "%Y-%m-%d %H:%M:%S");
    return !ss.fail();
}

static int anioActual()
{
    time_t ahora = time(NULL);
    tm local;
    localtime_s(&local, &ahora);
    return local.tm_year + 1900;
}

VueloDAO::VueloDAO()
{
    db.inicializar();
}

std::vector<Vuelo> VueloDAO::listar() const
{
    std::vector<Vuelo> vuelos;
    auto filas = db.consultar("SELECT id_vuelo,numero_vuelo,origen_iata,destino_iata,fecha_salida,fecha_llegada,precio_base_vuelo,estado_vuelo,id_avion FROM vuelos ORDER BY id_vuelo DESC");
    for (const auto &fila : filas)
    {
        if (fila.size() >= 9)
        {
            vuelos.push_back(Vuelo(
                ConexionDB::convertirEntero(fila[0]), fila[1], fila[2], fila[3], fila[4], fila[5],
                ConexionDB::convertirDouble(fila[6]), fila[7], ConexionDB::convertirEntero(fila[8])));
        }
    }
    return vuelos;
}

std::vector<std::string> VueloDAO::listarAeropuertos() const
{
    std::vector<std::string> aeropuertos;
    auto filas = db.consultar("SELECT codigo_iata FROM aeropuertos ORDER BY codigo_iata");
    for (const auto &fila : filas)
    {
        if (!fila.empty())
            aeropuertos.push_back(fila[0]);
    }
    return aeropuertos;
}

Vuelo VueloDAO::buscarPorId(int id) const
{
    auto filas = db.consultar("SELECT id_vuelo,numero_vuelo,origen_iata,destino_iata,fecha_salida,fecha_llegada,precio_base_vuelo,estado_vuelo,id_avion FROM vuelos WHERE id_vuelo=" + std::to_string(id) + " LIMIT 1");
    if (!filas.empty() && filas[0].size() >= 9)
    {
        return Vuelo(
            ConexionDB::convertirEntero(filas[0][0]), filas[0][1], filas[0][2], filas[0][3], filas[0][4], filas[0][5],
            ConexionDB::convertirDouble(filas[0][6]), filas[0][7], ConexionDB::convertirEntero(filas[0][8]));
    }
    return Vuelo();
}

std::string VueloDAO::validar(const Vuelo &vuelo) const
{
    if (vuelo.getNumero().empty() || vuelo.getOrigen().empty() || vuelo.getDestino().empty())
        return "Faltan numero de vuelo, origen o destino.";
    if (vuelo.getOrigen() == vuelo.getDestino())
        return "El origen y el destino no pueden ser iguales.";
    if (vuelo.getFechaSalida().empty() || vuelo.getFechaLlegada().empty())
        return "Debe cargar fecha/hora de salida y llegada.";
    std::tm salida = {};
    std::tm llegada = {};
    if (!parseFechaMysql(vuelo.getFechaSalida(), salida) || !parseFechaMysql(vuelo.getFechaLlegada(), llegada))
        return "Las fechas deben tener formato YYYY-MM-DD HH:MM:SS.";
    int limiteAnio = anioActual() + 1;
    if (salida.tm_year + 1900 > limiteAnio || llegada.tm_year + 1900 > limiteAnio)
        return "El anio del vuelo excede el limite permitido.";
    time_t salidaTime = std::mktime(&salida);
    time_t llegadaTime = std::mktime(&llegada);
    if (salidaTime == (time_t)-1 || llegadaTime == (time_t)-1 || llegadaTime <= salidaTime)
        return "La fecha/hora de llegada debe ser posterior a la salida.";
    if (vuelo.getPrecioBase() <= 0)
        return "El precio base debe ser mayor a cero.";

    auto origen = db.consultar("SELECT COUNT(*) FROM aeropuertos WHERE UPPER(TRIM(codigo_iata))=UPPER(TRIM('" + ConexionDB::escapar(vuelo.getOrigen()) + "'))");
    auto destino = db.consultar("SELECT COUNT(*) FROM aeropuertos WHERE UPPER(TRIM(codigo_iata))=UPPER(TRIM('" + ConexionDB::escapar(vuelo.getDestino()) + "'))");
    if (origen.empty() || destino.empty() || ConexionDB::convertirEntero(origen[0][0]) == 0 || ConexionDB::convertirEntero(destino[0][0]) == 0)
        return "El aeropuerto de origen o destino no existe en la base.";

    AvionDAO avionDAO;
    Avion avion = avionDAO.buscarPorId(vuelo.getIdAvion());
    if (avion.getId() == 0)
        return "La aeronave seleccionada no existe.";
    if (avion.getEstado() != "Activo")
        return "La aeronave esta en " + avion.getEstado() + " y no puede utilizarse.";

    auto solapados = db.consultar(
        "SELECT COUNT(*) FROM vuelos WHERE id_avion=" + std::to_string(vuelo.getIdAvion()) +
        " AND estado_vuelo<>'Cancelado' AND id_vuelo<>" + std::to_string(vuelo.getId()) +
        " AND '" + ConexionDB::escapar(vuelo.getFechaSalida()) + "' < fecha_llegada"
        " AND '" + ConexionDB::escapar(vuelo.getFechaLlegada()) + "' > fecha_salida");
    if (!solapados.empty() && !solapados[0].empty() && ConexionDB::convertirEntero(solapados[0][0]) > 0)
        return "La aeronave ya tiene otro vuelo en ese rango horario.";

    return "OK";
}

Vuelo VueloDAO::crear(const Vuelo &vuelo)
{
    auto siguiente = db.consultar("SELECT COALESCE(MAX(id_vuelo),0)+1 FROM vuelos");
    int nuevoId = 0;
    if (!siguiente.empty() && !siguiente[0].empty())
        nuevoId = ConexionDB::convertirEntero(siguiente[0][0]);
    if (nuevoId <= 0)
        return Vuelo();

    std::stringstream sql;
    sql << "INSERT INTO vuelos (id_vuelo,numero_vuelo,id_avion,origen_iata,destino_iata,fecha_salida,fecha_llegada,precio_base_vuelo,estado_vuelo) VALUES ("
        << nuevoId << ",'"
        << ConexionDB::escapar(vuelo.getNumero()) << "',"
        << vuelo.getIdAvion() << ",'"
        << ConexionDB::escapar(vuelo.getOrigen()) << "','"
        << ConexionDB::escapar(vuelo.getDestino()) << "','"
        << ConexionDB::escapar(vuelo.getFechaSalida()) << "','"
        << ConexionDB::escapar(vuelo.getFechaLlegada()) << "',"
        << vuelo.getPrecioBase() << ",'"
        << ConexionDB::escapar(vuelo.getEstado()) << "')";

    if (!db.ejecutar(sql.str()))
    {
        std::stringstream fallback;
        fallback << "INSERT INTO vuelos (numero_vuelo,id_avion,origen_iata,destino_iata,fecha_salida,fecha_llegada,precio_base_vuelo,estado_vuelo) VALUES ('"
                 << ConexionDB::escapar(vuelo.getNumero()) << "',"
                 << vuelo.getIdAvion() << ",'"
                 << ConexionDB::escapar(vuelo.getOrigen()) << "','"
                 << ConexionDB::escapar(vuelo.getDestino()) << "','"
                 << ConexionDB::escapar(vuelo.getFechaSalida()) << "','"
                 << ConexionDB::escapar(vuelo.getFechaLlegada()) << "',"
                 << vuelo.getPrecioBase() << ",'"
                 << ConexionDB::escapar(vuelo.getEstado()) << "')";

        if (!db.ejecutar(fallback.str()))
            return Vuelo();
    }

    auto filas = db.consultar("SELECT id_vuelo,numero_vuelo,origen_iata,destino_iata,fecha_salida,fecha_llegada,precio_base_vuelo,estado_vuelo,id_avion FROM vuelos WHERE numero_vuelo='" + ConexionDB::escapar(vuelo.getNumero()) + "' AND fecha_salida='" + ConexionDB::escapar(vuelo.getFechaSalida()) + "' ORDER BY id_vuelo DESC LIMIT 1");
    if (!filas.empty() && filas[0].size() >= 9)
        return Vuelo(ConexionDB::convertirEntero(filas[0][0]), filas[0][1], filas[0][2], filas[0][3], filas[0][4], filas[0][5], ConexionDB::convertirDouble(filas[0][6]), filas[0][7], ConexionDB::convertirEntero(filas[0][8]));
    return Vuelo();
}

bool VueloDAO::asignarAvion(int idVuelo, int idAvion)
{
    AvionDAO avionDAO;
    Avion avion = avionDAO.buscarPorId(idAvion);
    if (avion.getId() == 0 || avion.getEstado() != "Activo")
        return false;

    Vuelo vuelo = buscarPorId(idVuelo);
    if (vuelo.getId() == 0)
        return false;
    vuelo.setIdAvion(idAvion);
    if (validar(vuelo) != "OK")
        return false;

    std::stringstream sql;
    sql << "UPDATE vuelos SET id_avion=" << idAvion << ", estado_vuelo='Reprogramado' WHERE id_vuelo=" << idVuelo;
    return db.ejecutar(sql.str());
}

bool VueloDAO::cancelar(int idVuelo, const std::string &motivo)
{
    std::stringstream sql;
    sql << "UPDATE vuelos SET estado_vuelo='Cancelado' WHERE id_vuelo=" << idVuelo;
    return db.ejecutar(sql.str());
}

bool VueloDAO::reprogramar(int idVuelo, const std::string &nuevaSalida, const std::string &nuevaLlegada)
{
    Vuelo vuelo = buscarPorId(idVuelo);
    if (vuelo.getId() == 0)
        return false;
    vuelo.setFechaSalida(nuevaSalida);
    vuelo.setFechaLlegada(nuevaLlegada);
    if (validar(vuelo) != "OK")
        return false;

    std::stringstream sql;
    sql << "UPDATE vuelos SET fecha_salida='" << ConexionDB::escapar(nuevaSalida)
        << "', fecha_llegada='" << ConexionDB::escapar(nuevaLlegada)
        << "', estado_vuelo='Reprogramado' WHERE id_vuelo=" << idVuelo;
    return db.ejecutar(sql.str());
}

std::vector<Vuelo> VueloDAO::listarPorAvion(int idAvion) const
{
    std::vector<Vuelo> vuelos;
    auto filas = db.consultar("SELECT id_vuelo,numero_vuelo,origen_iata,destino_iata,fecha_salida,fecha_llegada,precio_base_vuelo,estado_vuelo,id_avion FROM vuelos WHERE id_avion=" + std::to_string(idAvion) + " AND estado_vuelo NOT LIKE 'Cancelado%' ORDER BY fecha_salida");
    for (const auto &fila : filas)
    {
        if (fila.size() >= 9)
        {
            vuelos.push_back(Vuelo(
                ConexionDB::convertirEntero(fila[0]), fila[1], fila[2], fila[3], fila[4], fila[5],
                ConexionDB::convertirDouble(fila[6]), fila[7], ConexionDB::convertirEntero(fila[8])));
        }
    }
    return vuelos;
}
