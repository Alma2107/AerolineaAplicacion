#include "ModuloPasajeros.h"
#include "ComponentesUI.h"
#include "raylib.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

static std::string normalizarDocumento(const std::string &valor)
{
    std::string salida;
    for (char c : valor)
    {
        if (std::isdigit((unsigned char)c) && salida.size() < 8)
            salida.push_back(c);
    }
    return salida;
}

static bool contieneId(const std::vector<int> &valores, int id)
{
    return std::find(valores.begin(), valores.end(), id) != valores.end();
}

static void alternarId(std::vector<int> &valores, int id)
{
    auto it = std::find(valores.begin(), valores.end(), id);
    if (it == valores.end())
        valores.push_back(id);
    else
        valores.erase(it);
}

static double precioSeleccionado(const std::vector<CatalogoItem> &items, int id)
{
    for (const CatalogoItem &item : items)
        if (item.id == id)
            return item.precio;
    return 0;
}

static std::string nombreSeleccionado(const std::vector<CatalogoItem> &items, int id)
{
    for (const CatalogoItem &item : items)
        if (item.id == id)
            return item.nombre;
    return "";
}

ModuloPasajeros::ModuloPasajeros()
    : mensaje("Modulo activo."), vista(0), foco(0), codigoReserva("AB1234"), tipoDoc("DNI"), documento("50000111"),
      nombre("Ana"), apellido("Torres"), fechaNacimiento("1998-03-21"), asistencia("0"), detalles(""),
      idVuelo("1"), idCliente("1"), idMetodoPago("1"), asiento("14A"), idPlan("1"), tipoViaje("Solo ida"), codigoReservaNueva("CD5678"), precio("65000"),
      idTipoEquipaje(""), cantidadEquipaje("1"), precioEquipaje("0"), idServicio(""), precioServicio("0"),
      estadoAsientos(""),
      vistaAnterior(-1), notificacionesNoLeidas(0)
{
    refrescarDatos();
}

void ModuloPasajeros::refrescarAvisos()
{
    notificacionesNoLeidas = notificacionDAO.contarNoLeidas(2);
    notificacionesCache = notificacionDAO.listarPorModulo(2);
}

void ModuloPasajeros::refrescarDatos()
{
    if (vista == 1)
    {
        planesCache = reservaDAO.listarPlanes();
        vuelosCache = reservaDAO.listarVuelosDisponibles();
        tiposEquipajeCache = reservaDAO.listarTiposEquipaje();
        serviciosCache = reservaDAO.listarServicios();
        asientosCache = reservaDAO.listarAsientosDisponibles(ConexionDB::convertirEntero(idVuelo));
    }
    if (vista == 2)
        reservasCache = reservaDAO.listar();
    if (vista == 4)
        pasajerosCache = pasajeroDAO.listar();

    refrescarAvisos();
    vistaAnterior = vista;
}

void ModuloPasajeros::dibujarNavegacion()
{
    std::vector<std::string> opciones = {"Check-in", "Nueva reserva", "Cancelar/reembolso", "Notificaciones", "Pasajeros"};
    UI::menuLateral("Atencion al Pasajero", opciones, vista, UI::green());

    for (int i = 0; i < (int)opciones.size(); ++i)
    {
        Rectangle item = {18, (float)(92 + i * 50), 174, 42};
        if (CheckCollisionPointRec(GetMousePosition(), item) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            vista = i;
    }

    if (UI::botonSecundario(Rectangle{1000, 90, 220, 42}, "Volver al menú principal", UI::green()))
    {
        vista = -1;
        return;
    }
    if (UI::botonSecundario(Rectangle{24, 690, 160, 42}, "Salir", RED))
    {
        CloseWindow();
        return;
    }
}

void ModuloPasajeros::dibujarCheckIn()`r`n{
    DrawText("Check-in por codigo de reserva", 240, 95, 30, UI::navy());
    DrawText("Ingrese el codigo de reserva para registrar el check-in", 240, 130, 17, UI::muted());

    if (UI::input(Rectangle{240, 190, 300, 46}, "Codigo de reserva", codigoReserva, foco == 1))
        foco = 1;
    if (foco == 1)
        UI::capturarTexto(codigoReserva, 20);

    static Reserva encontrada;
    if (UI::boton(Rectangle{570, 190, 190, 46}, "Registrar check-in", UI::green()) || IsKeyPressed(KEY_ENTER))
    {
        encontrada = reservaDAO.buscarPorCodigoReserva(codigoReserva);
        if (encontrada.getIdTicket() != 0)
        {
            bool checkInGuardado = reservaDAO.registrarCheckIn(codigoReserva);
            mensaje = checkInGuardado ? "Codigo valido. Check-in registrado en la base de datos."
                                      : "Codigo valido. No se pudo registrar check-in en BD: revise tabla checkins.";
        }
        else
        {
            encontrada = Reserva();
            mensaje = "Codigo invalido o reserva inexistente.";
        }
    }

    DrawText(mensaje.c_str(), 240, 270, 19, encontrada.getIdTicket() != 0 && mensaje.find("No se pudo") == std::string::npos ? UI::green() : RED);

    if (encontrada.getIdTicket() != 0)
    {
        DrawRectangleRounded(Rectangle{240, 325, 760, 210}, 0.04f, 8, WHITE);
        DrawRectangleLinesEx(Rectangle{240, 325, 760, 210}, 1, UI::border());
        DrawText("Datos de la reserva", 265, 350, 22, UI::green());
        DrawText(("Pasajero: " + encontrada.getPasajero()).c_str(), 265, 392, 18, DARKGRAY);
        DrawText(("Documento: " + encontrada.getDocumento()).c_str(), 265, 424, 18, DARKGRAY);
        DrawText(("Vuelo: " + encontrada.getNumeroVuelo() + " | " + encontrada.getOrigen() + " -> " + encontrada.getDestino()).c_str(), 265, 456, 18, DARKGRAY);
        DrawText(("Salida: " + encontrada.getFechaSalida()).c_str(), 265, 488, 18, DARKGRAY);
        DrawText(("Asiento: " + encontrada.getAsiento() + " | Ticket #" + std::to_string(encontrada.getIdTicket())).c_str(), 265, 520, 18, DARKGRAY);
    }
}

void ModuloPasajeros::dibujarNuevaReserva()
{
    DrawText("Crear reserva presencial", 240, 88, 30, UI::navy());
    DrawText("Formulario conectado a pasajeros, compras_ordenes, tickets_detalle, ticket_equipajes y ticket_servicios", 240, 122, 16, UI::muted());

    DrawRectangleRounded(Rectangle{235, 150, 1000, 120}, 0.035f, 8, WHITE);
    DrawRectangleLinesEx(Rectangle{235, 150, 1000, 120}, 1, UI::border());
    DrawRectangleRounded(Rectangle{235, 150, 8, 120}, 0.035f, 8, UI::green());
    DrawText("Datos del pasajero", 255, 165, 20, UI::navy());
    DrawText("ID pasajero y codigo de reserva se generan automaticamente.", 460, 168, 15, UI::muted());

    if (UI::input(Rectangle{255, 212, 120, 42}, "Tipo doc", tipoDoc, foco == 2))
        foco = 2;
    if (UI::input(Rectangle{395, 212, 170, 42}, "Documento DNI", documento, foco == 3))
        foco = 3;
    if (UI::input(Rectangle{585, 212, 180, 42}, "Nombre", nombre, foco == 4))
        foco = 4;
    if (UI::input(Rectangle{785, 212, 180, 42}, "Apellido", apellido, foco == 5))
        foco = 5;
    if (UI::input(Rectangle{985, 212, 130, 42}, "Pago", idMetodoPago, foco == 13))
        foco = 13;

    DrawRectangleRounded(Rectangle{235, 292, 1000, 190}, 0.035f, 8, WHITE);
    DrawRectangleLinesEx(Rectangle{235, 292, 1000, 190}, 1, UI::border());
    DrawRectangleRounded(Rectangle{235, 292, 8, 190}, 0.035f, 8, UI::green());
    DrawText("Viaje y disponibilidad", 255, 308, 20, UI::navy());

    if (UI::input(Rectangle{255, 360, 170, 42}, "Fecha nacimiento", fechaNacimiento, foco == 6))
        foco = 6;
    if (UI::input(Rectangle{445, 360, 120, 42}, "Asistencia", asistencia, foco == 7))
        foco = 7;
    if (UI::input(Rectangle{585, 360, 255, 42}, "Detalles medicos", detalles, foco == 8))
        foco = 8;

    DrawText("Tipo de viaje", 860, 340, 16, UI::muted());
    if (UI::botonSecundario(Rectangle{860, 360, 105, 30}, "Solo ida", tipoViaje == "Solo ida" ? UI::green() : UI::muted()))
        tipoViaje = "Solo ida";
    if (UI::botonSecundario(Rectangle{975, 360, 125, 30}, "Ida y vuelta", tipoViaje == "Ida y vuelta" ? UI::green() : UI::muted()))
        tipoViaje = "Ida y vuelta";
    if (UI::botonSecundario(Rectangle{1110, 360, 110, 30}, "Multitramo", tipoViaje == "Multitramo" ? UI::green() : UI::muted()))
        tipoViaje = "Multitramo";
    if (tipoViaje != "Solo ida")
    {
        DrawText("Complete el tramo complementario luego de guardar este tramo.", 860, 402, 14, UI::muted());
    }

        if (UI::input(Rectangle{255, 430, 120, 42}, "ID cliente", idCliente, foco == 9))
        foco = 9;
    if (UI::input(Rectangle{395, 430, 120, 42}, "ID vuelo", idVuelo, foco == 10))
        foco = 10;
    if (UI::input(Rectangle{535, 430, 120, 42}, "Asiento", asiento, foco == 11))
        foco = 11;
    if (UI::input(Rectangle{675, 430, 120, 42}, "ID plan", idPlan, foco == 12))
        foco = 12;
    if (UI::input(Rectangle{815, 430, 145, 42}, "Precio base", precio, foco == 15))
        foco = 15;
    DrawRectangleRounded(Rectangle{235, 505, 1000, 130}, 0.035f, 8, WHITE);
    DrawRectangleLinesEx(Rectangle{235, 505, 1000, 130}, 1, UI::border());
    DrawRectangleRounded(Rectangle{235, 505, 8, 130}, 0.035f, 8, UI::green());

    DrawText("Equipaje adicional", 255, 520, 16, UI::muted());
    if (UI::input(Rectangle{255, 550, 130, 42}, "ID tipo", idTipoEquipaje, foco == 16))
        foco = 16;
    if (UI::input(Rectangle{405, 550, 110, 42}, "Cantidad", cantidadEquipaje, foco == 17))
        foco = 17;
    if (UI::input(Rectangle{535, 550, 130, 42}, "Precio eq.", precioEquipaje, foco == 18))
        foco = 18;
    DrawText("Servicios", 705, 520, 16, UI::muted());
    if (UI::input(Rectangle{705, 550, 130, 42}, "ID servicio", idServicio, foco == 19))
        foco = 19;
    if (UI::input(Rectangle{855, 550, 130, 42}, "Precio serv.", precioServicio, foco == 20))
        foco = 20;
    if (foco == 2)
        UI::capturarTexto(tipoDoc, 12);
    if (foco == 3)
    {
        documento = normalizarDocumento(documento);
        UI::capturarTexto(documento, 8);
        documento = normalizarDocumento(documento);
    }
    if (foco == 4)
        UI::capturarTexto(nombre, 35);
    if (foco == 5)
        UI::capturarTexto(apellido, 35);
    if (foco == 6)
        UI::capturarTexto(fechaNacimiento, 10);
    if (foco == 7)
        UI::capturarTexto(asistencia, 1);
    if (foco == 8)
        UI::capturarTexto(detalles, 60);
    if (foco == 9)
        UI::capturarTexto(idCliente, 8);
    if (foco == 13)
        UI::capturarTexto(idMetodoPago, 8);
    if (foco == 15)
        UI::capturarTexto(precio, 12);

    Pasajero pasajero(0, tipoDoc, documento, nombre, apellido, fechaNacimiento, asistencia == "1", detalles, false);
    if (UI::botonSecundario(Rectangle{255, 655, 180, 40}, "Ver disponibilidad", UI::green()))
    {
        int disponibles = reservaDAO.contarAsientosDisponibles(ConexionDB::convertirEntero(idVuelo));
        estadoAsientos = disponibles >= 0 ? "Asientos libres para venta presencial: " + std::to_string(disponibles)
                                          : "No se pudo calcular disponibilidad del vuelo.";
        asientosCache = reservaDAO.listarAsientosDisponibles(ConexionDB::convertirEntero(idVuelo));
        mensaje = disponibles > 0 ? "Vuelo disponible. Seleccione un asiento libre." : "Avion lleno: informar al cliente que debe elegir otro dia/vuelo.";
    }

    if (UI::botonSecundario(Rectangle{455, 655, 170, 40}, "Validar reserva", UI::green()))
    {
        std::string codigoTemporal = reservaDAO.generarCodigoReserva();
        mensaje = reservaDAO.validarReservaPresencial(
            pasajero, ConexionDB::convertirEntero(idCliente), ConexionDB::convertirEntero(idVuelo),
            asiento, ConexionDB::convertirEntero(idPlan), codigoTemporal, ConexionDB::convertirDouble(precio),
            ConexionDB::convertirEntero(idMetodoPago));
        if (mensaje == "OK")
            mensaje = "Validacion API BD: reserva apta para guardar.";
        else
            mensaje = "Validacion API BD: " + mensaje;
        int disponibles = reservaDAO.contarAsientosDisponibles(ConexionDB::convertirEntero(idVuelo));
        estadoAsientos = disponibles >= 0 ? "Asientos libres para venta presencial: " + std::to_string(disponibles)
                                          : "No se pudo calcular disponibilidad del vuelo.";
    }

    if (UI::boton(Rectangle{645, 655, 170, 40}, "Guardar reserva", UI::green()))
    {
        std::string codigoTemporal = reservaDAO.generarCodigoReserva();
        std::string validacion = reservaDAO.validarReservaPresencial(
            pasajero, ConexionDB::convertirEntero(idCliente), ConexionDB::convertirEntero(idVuelo),
            asiento, ConexionDB::convertirEntero(idPlan), codigoTemporal, ConexionDB::convertirDouble(precio),
            ConexionDB::convertirEntero(idMetodoPago));
        if (validacion != "OK")
        {
            mensaje = "Validacion API BD: " + validacion;
            DrawText(mensaje.c_str(), 240, 720, 19, RED);
            return;
        }
        Reserva reserva = reservaDAO.crearReservaPresencial(
            pasajero, ConexionDB::convertirEntero(idCliente), ConexionDB::convertirEntero(idVuelo),
            asiento, ConexionDB::convertirEntero(idPlan), codigoTemporal, ConexionDB::convertirDouble(precio),
            ConexionDB::convertirEntero(idMetodoPago), ConexionDB::convertirEntero(idTipoEquipaje),
            ConexionDB::convertirEntero(cantidadEquipaje, 1), ConexionDB::convertirDouble(precioEquipaje),
            ConexionDB::convertirEntero(idServicio), ConexionDB::convertirDouble(precioServicio));
        mensaje = reserva.getIdTicket() != 0 ? "Reserva guardada en BD con codigo " + reserva.getCodigoReserva() : "No se pudo guardar la reserva.";
        if (reserva.getIdTicket() != 0)
        {
            int disponibles = reservaDAO.contarAsientosDisponibles(ConexionDB::convertirEntero(idVuelo));
            estadoAsientos = disponibles >= 0 ? "Asientos libres para venta presencial: " + std::to_string(disponibles)
                                              : "No se pudo calcular disponibilidad del vuelo.";
            refrescarDatos();
        }
    }

    double total = ConexionDB::convertirDouble(precio) + precioSeleccionado(planesCache, ConexionDB::convertirEntero(idPlan));
    for (int id : equipajesSeleccionados)
        total += precioSeleccionado(tiposEquipajeCache, id);
    for (int id : serviciosSeleccionados)
        total += precioSeleccionado(serviciosCache, id);
    std::stringstream totalTexto;
    totalTexto << "Total estimado: $" << (int)total << " | Vuelo " << nombreSeleccionado(vuelosCache, ConexionDB::convertirEntero(idVuelo)) << " | Asiento " << asiento;
    UI::textoRecortado(totalTexto.str(), 840, 658, 17, UI::navy(), 46);
    UI::textoRecortado(mensaje, 255, 710, 18, mensaje.find("apta") != std::string::npos || mensaje.find("guardada") != std::string::npos || mensaje.find("disponible") != std::string::npos ? UI::green() : RED, 88);
    UI::textoRecortado(estadoAsientos, 255, 735, 16, UI::muted(), 88);
}

void ModuloPasajeros::dibujarCancelaciones()
{
    DrawText("Cancelar reservas y reembolsos", 240, 95, 30, UI::navy());
    DrawText("Las acciones actualizan o eliminan registros en tickets_detalle", 240, 130, 17, UI::muted());

    Rectangle panel = {240, 165, 990, 540};
    DrawRectangleRounded(Rectangle{panel.x, panel.y + 6, panel.width, panel.height}, 0.035f, 8, Color{224, 232, 242, 255});
    DrawRectangleRounded(panel, 0.03f, 8, WHITE);
    DrawRectangleLinesEx(panel, 1, UI::border());
    DrawRectangleRounded(Rectangle{panel.x, panel.y, 8, panel.height}, 0.03f, 8, UI::green());

    DrawText("Reservas activas", 265, 190, 22, UI::navy());
    std::string total = std::to_string(reservasCache.size()) + " registros";
    int totalAncho = MeasureText(total.c_str(), 16);
    Rectangle contador = {panel.x + panel.width - totalAncho - 52, 184, (float)totalAncho + 28, 32};
    DrawRectangleRounded(contador, 0.24f, 8, Fade(UI::green(), 0.12f));
    DrawText(total.c_str(), (int)contador.x + 14, (int)contador.y + 8, 16, UI::green());

    Rectangle cabecera = {265, 240, 930, 42};
    DrawRectangleRounded(cabecera, 0.08f, 8, Fade(UI::green(), 0.10f));
    DrawText("ID", 285, 253, 15, UI::green());
    DrawText("Codigo", 360, 253, 15, UI::green());
    DrawText("Pasajero", 500, 253, 15, UI::green());
    DrawText("Vuelo", 770, 253, 15, UI::green());
    DrawText("Precio", 900, 253, 15, UI::green());
    DrawText("Acciones", 1035, 253, 15, UI::green());

    if (reservasCache.empty())
    {
        DrawRectangleRounded(Rectangle{265, 315, 930, 95}, 0.04f, 8, Color{248, 250, 253, 255});
        DrawRectangleLinesEx(Rectangle{265, 315, 930, 95}, 1, UI::border());
        DrawText("No hay reservas para mostrar.", 295, 348, 20, UI::muted());
        DrawText("Las reservas creadas se listaran aca para cancelar o reembolsar.", 295, 378, 16, UI::muted());
        DrawText(mensaje.c_str(), 240, 720, 18, UI::green());
        return;
    }

    int y = 300;
    for (int i = 0; i < (int)reservasCache.size() && i < 7; ++i)
    {
        const Reserva &reserva = reservasCache[i];
        Rectangle fila = {265, (float)y, 930, 52};
        Color fondo = i % 2 == 0 ? Color{248, 250, 253, 255} : WHITE;
        DrawRectangleRounded(fila, 0.035f, 8, fondo);
        DrawRectangleLinesEx(fila, 1, Color{226, 234, 244, 255});

        std::stringstream precioReserva;
        precioReserva << "$" << reserva.getPrecio();
        std::string ruta = reserva.getNumeroVuelo() + " " + reserva.getOrigen() + "-" + reserva.getDestino();

        DrawText(("#" + std::to_string(reserva.getIdTicket())).c_str(), 285, y + 17, 16, UI::navy());
        UI::textoRecortado(reserva.getCodigoReserva(), 360, y + 17, 16, DARKGRAY, 14);
        UI::textoRecortado(reserva.getPasajero(), 500, y + 17, 17, UI::navy(), 28);
        UI::textoRecortado(ruta, 770, y + 17, 16, DARKGRAY, 16);
        UI::textoRecortado(precioReserva.str(), 900, y + 17, 16, DARKGRAY, 12);

        Rectangle cancelar = {1025, (float)y + 10, 78, 32};
        Rectangle reembolso = {1110, (float)y + 10, 78, 32};
        if (UI::botonSecundario(cancelar, "Cancelar", RED))
        {
            mensaje = reservaDAO.cancelar(reserva.getIdTicket()) ? "Reserva cancelada y eliminada." : "No se pudo cancelar.";
            refrescarDatos();
        }
        if (UI::botonSecundario(reembolso, "Reemb.", UI::green()))
        {
            mensaje = reservaDAO.reembolsar(reserva.getIdTicket()) ? "Reserva reembolsada." : "No se pudo reembolsar.";
            refrescarDatos();
        }
        y += 58;
    }

    DrawText(mensaje.c_str(), 240, 720, 18, UI::green());
}

void ModuloPasajeros::dibujarNotificaciones()
{
    DrawText("Panel de avisos", 240, 95, 30, UI::navy());
    DrawText("Cancelaciones y reprogramaciones enviadas por Operaciones de Vuelo", 240, 130, 17, UI::muted());
    int y = 180;
    for (const Notificacion &notificacion : notificacionesCache)
    {
        UI::aviso(Rectangle{240, (float)y, 760, 78}, notificacion.getTipo(), notificacion.getMensaje(), UI::green());
        y += 92;
    }
    if (UI::botonSecundario(Rectangle{240, 690, 180, 42}, "Marcar leidas", UI::green()))
    {
        notificacionDAO.marcarLeidas(2);
        refrescarAvisos();
    }
}

void ModuloPasajeros::dibujarListado()
{
    DrawText("Pasajeros registrados", 240, 95, 30, UI::navy());
    DrawText("Ultimos pasajeros cargados en la base de datos", 240, 130, 17, UI::muted());

    Rectangle panel = {240, 165, 990, 540};
    DrawRectangleRounded(Rectangle{panel.x, panel.y + 6, panel.width, panel.height}, 0.035f, 8, Color{224, 232, 242, 255});
    DrawRectangleRounded(panel, 0.03f, 8, WHITE);
    DrawRectangleLinesEx(panel, 1, UI::border());
    DrawRectangleRounded(Rectangle{panel.x, panel.y, 8, panel.height}, 0.03f, 8, UI::green());

    DrawText("Listado general", 265, 190, 22, UI::navy());
    std::string total = std::to_string(pasajerosCache.size()) + " registros";
    int totalAncho = MeasureText(total.c_str(), 16);
    Rectangle contador = {panel.x + panel.width - totalAncho - 52, 184, (float)totalAncho + 28, 32};
    DrawRectangleRounded(contador, 0.24f, 8, Fade(UI::green(), 0.12f));
    DrawText(total.c_str(), (int)contador.x + 14, (int)contador.y + 8, 16, UI::green());

    Rectangle cabecera = {265, 240, 930, 42};
    DrawRectangleRounded(cabecera, 0.08f, 8, Fade(UI::green(), 0.10f));
    DrawText("ID", 285, 253, 15, UI::green());
    DrawText("Documento", 360, 253, 15, UI::green());
    DrawText("Pasajero", 555, 253, 15, UI::green());
    DrawText("Nacimiento", 830, 253, 15, UI::green());
    DrawText("Asistencia", 1010, 253, 15, UI::green());

    if (pasajerosCache.empty())
    {
        DrawRectangleRounded(Rectangle{265, 315, 930, 95}, 0.04f, 8, Color{248, 250, 253, 255});
        DrawRectangleLinesEx(Rectangle{265, 315, 930, 95}, 1, UI::border());
        DrawText("No hay pasajeros registrados todavia.", 295, 348, 20, UI::muted());
        DrawText("Cuando cargue una reserva presencial, el pasajero aparecera en este cuadro.", 295, 378, 16, UI::muted());
        return;
    }

    int y = 300;
    for (int i = 0; i < (int)pasajerosCache.size() && i < 7; ++i)
    {
        const Pasajero &pasajero = pasajerosCache[i];
        Rectangle fila = {265, (float)y, 930, 52};
        Color fondo = i % 2 == 0 ? Color{248, 250, 253, 255} : WHITE;
        DrawRectangleRounded(fila, 0.035f, 8, fondo);
        DrawRectangleLinesEx(fila, 1, Color{226, 234, 244, 255});

        std::string id = "#" + std::to_string(pasajero.getId());
        std::string documento = pasajero.getTipoDocumento() + " " + pasajero.getNumeroDocumento();
        std::string nombreCompleto = pasajero.getNombre() + " " + pasajero.getApellido();
        bool asistencia = pasajero.requiereAsistenciaEspecial();
        std::string asistenciaTexto = asistencia ? "Especial" : "Sin asistencia";
        Color asistenciaColor = asistencia ? UI::orange() : UI::green();
        int chipAncho = asistencia ? 86 : 118;

        DrawText(id.c_str(), 285, y + 17, 16, UI::navy());
        UI::textoRecortado(documento, 360, y + 17, 16, DARKGRAY, 22);
        UI::textoRecortado(nombreCompleto, 555, y + 17, 17, UI::navy(), 30);
        DrawText(pasajero.getFechaNacimiento().c_str(), 830, y + 17, 16, DARKGRAY);
        DrawRectangleRounded(Rectangle{1010, (float)y + 12, (float)chipAncho, 28}, 0.35f, 8, Fade(asistenciaColor, 0.12f));
        DrawText(asistenciaTexto.c_str(), 1022, y + 19, 14, asistenciaColor);

        y += 58;
    }
}

void ModuloPasajeros::mostrar()
{
    while (!WindowShouldClose())
    {
        if (vistaAnterior != vista)
            refrescarDatos();

        BeginDrawing();
        UI::shellModulo("Modulo 2 / Atencion al Pasajero", "Check-in, reservas, cancelaciones y avisos", UI::green(), notificacionesNoLeidas);
        dibujarNavegacion();
        if (vista < 0)
        {
            EndDrawing();
            return;
        }

        if (vista == 0)
            dibujarCheckIn();
        else if (vista == 1)
            dibujarNuevaReserva();
        else if (vista == 2)
            dibujarCancelaciones();
        else if (vista == 3)
            dibujarNotificaciones();
        else
            dibujarListado();

        EndDrawing();
    }
}

