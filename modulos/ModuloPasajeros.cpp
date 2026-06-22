#include "ModuloPasajeros.h"
#include "ComponentesUI.h"
#include "raylib.h"
#include <sstream>

ModuloPasajeros::ModuloPasajeros()
    : mensaje("Modulo activo."), vista(0), foco(0), qr("QR1001"), tipoDoc("DNI"), documento("50000111"),
      nombre("Ana"), apellido("Torres"), fechaNacimiento("1998-03-21"), asistencia("0"), detalles(""),
      idVuelo("1"), idCliente("1"), idMetodoPago("1"), asiento("14A"), idPlan("1"), codigoQR("QR2001"), precio("65000") {}

void ModuloPasajeros::dibujarNavegacion()
{
    std::vector<std::string> opciones = {"Check-in QR", "Nueva reserva", "Cancelar/reembolso", "Notificaciones", "Pasajeros"};
    UI::menuLateral("Atencion al Pasajero", opciones, vista, UI::green());

    for (int i = 0; i < (int)opciones.size(); ++i)
    {
        Rectangle item = {18, (float)(92 + i * 50), 174, 42};
        if (CheckCollisionPointRec(GetMousePosition(), item) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            vista = i;
    }

    if (UI::botonSecundario(Rectangle{24, 748, 160, 42}, "Volver al inicio", UI::green()))
    {
        vista = -1;
        return;
    }
}

void ModuloPasajeros::dibujarCheckIn()
{
    DrawText("Check-in mediante QR", 240, 95, 30, UI::navy());
    DrawText("Ingrese o escanee el codigo QR/PNR generado por la plataforma web", 240, 130, 17, UI::muted());

    if (UI::input(Rectangle{240, 190, 280, 46}, "Codigo QR / PNR", qr, foco == 1))
        foco = 1;
    if (foco == 1)
        UI::capturarTexto(qr, 12);

    static Reserva encontrada;
    if (UI::boton(Rectangle{545, 190, 170, 46}, "Escanear QR", UI::green()))
    {
        encontrada = reservaDAO.buscarPorQR(qr);
        if (encontrada.getIdTicket() != 0)
        {
            bool checkInGuardado = reservaDAO.registrarCheckIn(qr);
            mensaje = checkInGuardado ? "QR valido. Check-in registrado en la base de datos."
                                      : "QR valido. No se pudo registrar check-in en BD: revise tabla checkins.";
        }
        else
        {
            mensaje = "QR invalido o reserva inexistente.";
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
    DrawText("Crear reserva presencial", 240, 95, 30, UI::navy());
    DrawText("Formulario conectado a pasajeros, compras_ordenes y tickets_detalle", 240, 130, 17, UI::muted());

    if (UI::input(Rectangle{240, 185, 120, 46}, "Tipo doc", tipoDoc, foco == 2))
        foco = 2;
    if (UI::input(Rectangle{390, 185, 180, 46}, "Documento", documento, foco == 3))
        foco = 3;
    if (UI::input(Rectangle{600, 185, 190, 46}, "Nombre", nombre, foco == 4))
        foco = 4;
    if (UI::input(Rectangle{820, 185, 190, 46}, "Apellido", apellido, foco == 5))
        foco = 5;

    if (UI::input(Rectangle{240, 275, 190, 46}, "Fecha nacimiento", fechaNacimiento, foco == 6))
        foco = 6;
    if (UI::input(Rectangle{460, 275, 150, 46}, "Asistencia 0/1", asistencia, foco == 7))
        foco = 7;
    if (UI::input(Rectangle{640, 275, 370, 46}, "Detalles medicos", detalles, foco == 8))
        foco = 8;

    if (UI::input(Rectangle{240, 365, 120, 46}, "ID cliente", idCliente, foco == 9))
        foco = 9;
    if (UI::input(Rectangle{390, 365, 120, 46}, "ID vuelo", idVuelo, foco == 10))
        foco = 10;
    if (UI::input(Rectangle{540, 365, 120, 46}, "Asiento", asiento, foco == 11))
        foco = 11;
    if (UI::input(Rectangle{690, 365, 120, 46}, "ID plan", idPlan, foco == 12))
        foco = 12;
    if (UI::input(Rectangle{840, 365, 120, 46}, "Pago", idMetodoPago, foco == 13))
        foco = 13;
    if (UI::input(Rectangle{240, 455, 150, 46}, "QR/PNR", codigoQR, foco == 14))
        foco = 14;
    if (UI::input(Rectangle{420, 455, 150, 46}, "Precio", precio, foco == 15))
        foco = 15;

    if (foco == 2)
        UI::capturarTexto(tipoDoc, 12);
    if (foco == 3)
        UI::capturarTexto(documento, 20);
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
    if (foco == 10)
        UI::capturarTexto(idVuelo, 8);
    if (foco == 11)
        UI::capturarTexto(asiento, 6);
    if (foco == 12)
        UI::capturarTexto(idPlan, 8);
    if (foco == 13)
        UI::capturarTexto(idMetodoPago, 8);
    if (foco == 14)
        UI::capturarTexto(codigoQR, 12);
    if (foco == 15)
        UI::capturarTexto(precio, 12);

    Pasajero pasajero(0, tipoDoc, documento, nombre, apellido, fechaNacimiento, asistencia == "1", detalles, false);
    if (UI::botonSecundario(Rectangle{610, 455, 180, 44}, "Validar reserva", UI::green()))
    {
        mensaje = reservaDAO.validarReservaPresencial(
            pasajero, ConexionDB::convertirEntero(idCliente), ConexionDB::convertirEntero(idVuelo),
            asiento, ConexionDB::convertirEntero(idPlan), codigoQR, ConexionDB::convertirDouble(precio),
            ConexionDB::convertirEntero(idMetodoPago));
        if (mensaje == "OK")
            mensaje = "Validacion API BD: reserva apta para guardar.";
        else
            mensaje = "Validacion API BD: " + mensaje;
    }

    if (UI::boton(Rectangle{810, 455, 200, 44}, "Guardar reserva", UI::green()))
    {
        std::string validacion = reservaDAO.validarReservaPresencial(
            pasajero, ConexionDB::convertirEntero(idCliente), ConexionDB::convertirEntero(idVuelo),
            asiento, ConexionDB::convertirEntero(idPlan), codigoQR, ConexionDB::convertirDouble(precio),
            ConexionDB::convertirEntero(idMetodoPago));
        if (validacion != "OK")
        {
            mensaje = "Validacion API BD: " + validacion;
            DrawText(mensaje.c_str(), 240, 545, 19, RED);
            return;
        }
        Reserva reserva = reservaDAO.crearReservaPresencial(
            pasajero, ConexionDB::convertirEntero(idCliente), ConexionDB::convertirEntero(idVuelo),
            asiento, ConexionDB::convertirEntero(idPlan), codigoQR, ConexionDB::convertirDouble(precio),
            ConexionDB::convertirEntero(idMetodoPago));
        mensaje = reserva.getIdTicket() != 0 ? "Reserva guardada en BD con QR " + reserva.getCodigoQR() : "No se pudo guardar la reserva.";
    }

    DrawText(mensaje.c_str(), 240, 545, 19, mensaje.find("Validacion API BD: reserva apta") != std::string::npos || mensaje.find("guardada") != std::string::npos ? UI::green() : RED);
}

void ModuloPasajeros::dibujarCancelaciones()
{
    DrawText("Cancelar reservas y reembolsos", 240, 95, 30, UI::navy());
    DrawText("Las acciones actualizan o eliminan registros en tickets_detalle", 240, 130, 17, UI::muted());

    int y = 180;
    for (const Reserva &reserva : reservaDAO.listar())
    {
        DrawRectangleRounded(Rectangle{240, (float)y - 8, 880, 46}, 0.04f, 8, WHITE);
        DrawRectangleLinesEx(Rectangle{240, (float)y - 8, 880, 46}, 1, UI::border());
        std::stringstream ss;
        ss << "#" << reserva.getIdTicket() << " " << reserva.getCodigoQR() << " | " << reserva.getPasajero()
           << " | " << reserva.getNumeroVuelo() << " | $" << reserva.getPrecio();
        UI::textoRecortado(ss.str(), 260, y + 4, 16, DARKGRAY, 76);

        Rectangle cancelar = {930, (float)y - 2, 82, 32};
        Rectangle reembolso = {1020, (float)y - 2, 88, 32};
        if (UI::botonSecundario(cancelar, "Cancelar", RED))
            mensaje = reservaDAO.cancelar(reserva.getIdTicket()) ? "Reserva cancelada y eliminada." : "No se pudo cancelar.";
        if (UI::botonSecundario(reembolso, "Reemb.", UI::green()))
            mensaje = reservaDAO.reembolsar(reserva.getIdTicket()) ? "Reserva reembolsada." : "No se pudo reembolsar.";
        y += 56;
    }

    DrawText(mensaje.c_str(), 240, 720, 18, UI::green());
}

void ModuloPasajeros::dibujarNotificaciones()
{
    DrawText("Panel de avisos", 240, 95, 30, UI::navy());
    DrawText("Cancelaciones y reprogramaciones enviadas por Operaciones de Vuelo", 240, 130, 17, UI::muted());
    int y = 180;
    for (const Notificacion &notificacion : notificacionDAO.listarPorModulo(2))
    {
        UI::aviso(Rectangle{240, (float)y, 760, 78}, notificacion.getTipo(), notificacion.getMensaje(), UI::green());
        y += 92;
    }
    if (UI::botonSecundario(Rectangle{240, 690, 180, 42}, "Marcar leidas", UI::green()))
        notificacionDAO.marcarLeidas(2);
}

void ModuloPasajeros::dibujarListado()
{
    DrawText("Pasajeros registrados", 240, 95, 30, UI::navy());
    int y = 150;
    for (const Pasajero &pasajero : pasajeroDAO.listar())
    {
        std::stringstream ss;
        ss << pasajero.getId() << " | " << pasajero.getTipoDocumento() << " " << pasajero.getNumeroDocumento()
           << " | " << pasajero.getNombre() << " " << pasajero.getApellido()
           << " | Asistencia: " << (pasajero.requiereAsistenciaEspecial() ? "Si" : "No");
        UI::textoRecortado(ss.str(), 240, y, 18, DARKGRAY, 96);
        y += 34;
    }
}

void ModuloPasajeros::mostrar()
{
    while (!WindowShouldClose())
    {
        BeginDrawing();
        UI::shellModulo("Modulo 2 / Atencion al Pasajero", "Check-in QR, reservas, cancelaciones y avisos", UI::green(), notificacionDAO.contarNoLeidas(2));
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
