#include "ModuloMantenimiento.h"
#include "ComponentesUI.h"
#include "raylib.h"
#include <sstream>

ModuloMantenimiento::ModuloMantenimiento()
    : mensaje("Modulo activo."), vista(0), foco(0), idAvion("4"), estado("Mantenimiento"),
      modeloNuevo("Airbus A320neo"), capacidadNueva("180"), estadoNuevo("Activo") {}

void ModuloMantenimiento::dibujarNavegacion()
{
    std::vector<std::string> opciones = {"Estado de flota", "Registrar avion", "Actualizar estado", "PIMA", "Reportes"};
    UI::menuLateral("Flota y Mantenimiento", opciones, vista, UI::orange());

    for (int i = 0; i < (int)opciones.size(); ++i)
    {
        Rectangle item = {18, (float)(92 + i * 50), 174, 42};
        if (CheckCollisionPointRec(GetMousePosition(), item) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            vista = i;
    }

    if (UI::botonSecundario(Rectangle{24, 748, 160, 42}, "Volver al inicio", UI::orange()))
    {
        vista = -1;
        return;
    }
}

bool ModuloMantenimiento::notificarOperacion(int avionId, const std::string &nuevoEstado)
{
    if (nuevoEstado != "Mantenimiento" && nuevoEstado != "En reparacion")
        return true;

    std::vector<Vuelo> afectados = vueloDAO.listarPorAvion(avionId);
    std::stringstream mensajeNotificacion;
    mensajeNotificacion << "Avion " << avionId << " paso a " << nuevoEstado << ". ";
    if (nuevoEstado == "Mantenimiento")
        mensajeNotificacion << "No puede utilizarse para nuevos vuelos.";
    else
        mensajeNotificacion << afectados.size() << " vuelo(s) asociados deben reprogramarse o cancelarse.";

    return notificacionDAO.crear(1, nuevoEstado, mensajeNotificacion.str());
}

void ModuloMantenimiento::dibujarEstadoFlota()
{
    DrawText("Estado de flota", 240, 95, 30, UI::navy());
    dibujarListado();
}

void ModuloMantenimiento::dibujarRegistrarAeronave()
{
    DrawText("Registrar aeronave", 240, 95, 30, UI::navy());
    DrawText("Guarda una nueva aeronave en la tabla aviones", 240, 130, 17, UI::muted());

    if (UI::input(Rectangle{240, 190, 320, 46}, "Modelo", modeloNuevo, foco == 10))
        foco = 10;
    if (UI::input(Rectangle{590, 190, 170, 46}, "Capacidad", capacidadNueva, foco == 11))
        foco = 11;
    if (UI::input(Rectangle{790, 190, 220, 46}, "Estado inicial", estadoNuevo, foco == 12))
        foco = 12;

    if (foco == 10)
        UI::capturarTexto(modeloNuevo, 45);
    if (foco == 11)
        UI::capturarTexto(capacidadNueva, 5);
    if (foco == 12)
        UI::capturarTexto(estadoNuevo, 30);

    if (UI::botonSecundario(Rectangle{240, 280, 150, 42}, "Activo", UI::green()))
        estadoNuevo = "Activo";
    if (UI::botonSecundario(Rectangle{410, 280, 180, 42}, "Mantenimiento", UI::orange()))
        estadoNuevo = "Mantenimiento";
    if (UI::botonSecundario(Rectangle{610, 280, 180, 42}, "En reparacion", RED))
        estadoNuevo = "En reparacion";

    if (UI::boton(Rectangle{240, 355, 210, 44}, "Guardar aeronave", UI::orange()))
    {
        Avion avion = avionDAO.crear(modeloNuevo, ConexionDB::convertirEntero(capacidadNueva), estadoNuevo);
        if (avion.getId() != 0)
        {
            bool aviso = notificarOperacion(avion.getId(), avion.getEstado());
            mensaje = "Aeronave guardada en BD con ID " + std::to_string(avion.getId()) + ".";
            if (!aviso)
                mensaje += " No se pudo guardar aviso en BD: revise tabla notificaciones.";
            idAvion = std::to_string(avion.getId());
        }
        else
        {
            mensaje = "No se pudo guardar: revise modelo, capacidad y estado.";
        }
    }

    DrawText(mensaje.c_str(), 240, 440, 19, mensaje.find("guardada") != std::string::npos ? UI::green() : RED);
    dibujarListado();
}

void ModuloMantenimiento::dibujarActualizarEstado()
{
    DrawText("Actualizar estado de aeronave", 240, 95, 30, UI::navy());
    DrawText("Al cambiar a Mantenimiento o En reparacion se avisa a Operaciones de Vuelo", 240, 130, 17, UI::muted());

    if (UI::input(Rectangle{240, 190, 180, 46}, "ID aeronave", idAvion, foco == 1))
        foco = 1;
    if (UI::input(Rectangle{450, 190, 260, 46}, "Estado", estado, foco == 2))
        foco = 2;
    if (foco == 1)
        UI::capturarTexto(idAvion, 8);
    if (foco == 2)
        UI::capturarTexto(estado, 30);

    if (UI::botonSecundario(Rectangle{240, 280, 180, 42}, "Mantenimiento", UI::orange()))
        estado = "Mantenimiento";
    if (UI::botonSecundario(Rectangle{440, 280, 180, 42}, "En reparacion", RED))
        estado = "En reparacion";
    if (UI::botonSecundario(Rectangle{640, 280, 180, 42}, "Activo", UI::green()))
        estado = "Activo";

    if (UI::boton(Rectangle{240, 355, 210, 44}, "Guardar estado", UI::orange()))
    {
        int avion = ConexionDB::convertirEntero(idAvion);
        bool ok = avionDAO.actualizarEstado(avion, estado);
        if (ok)
        {
            bool aviso = notificarOperacion(avion, estado);
            mensaje = "Estado actualizado en BD.";
            if (!aviso)
                mensaje += " No se pudo guardar aviso en BD: revise tabla notificaciones.";
        }
        else
        {
            mensaje = "No se pudo actualizar la aeronave.";
        }
    }

    DrawText(mensaje.c_str(), 240, 440, 19, UI::green());
    dibujarListado();
}

void ModuloMantenimiento::dibujarListado()
{
    int y = vista == 0 ? 155 : 500;
    DrawText("ID   Modelo                         Capacidad   Estado", 240, y, 17, UI::muted());
    y += 34;
    for (const Avion &avion : avionDAO.listar())
    {
        std::stringstream ss;
        ss << avion.getId() << "   " << avion.getModelo() << "        " << avion.getCapacidad() << "        " << avion.getEstado();
        UI::textoRecortado(ss.str(), 240, y, 18, avion.getEstado() == "Activo" ? DARKGRAY : RED, 90);
        y += 34;
    }
}

void ModuloMantenimiento::mostrar()
{
    while (!WindowShouldClose())
    {
        BeginDrawing();
        UI::shellModulo("Modulo 4 / Flota y Mantenimiento", "Estado de flota, inspeccion y reparaciones", UI::orange(), 0);
        dibujarNavegacion();
        if (vista < 0)
        {
            EndDrawing();
            return;
        }

        if (vista == 0)
            dibujarEstadoFlota();
        else if (vista == 1)
            dibujarRegistrarAeronave();
        else if (vista == 2)
            dibujarActualizarEstado();
        else if (vista == 3)
        {
            DrawText("Plan de Inspeccion y Mantenimiento (PIMA)", 240, 95, 30, UI::navy());
            UI::aviso(Rectangle{240, 160, 740, 110}, "PIMA", "Use Actualizar estado para registrar limpieza, inspeccion, mantenimiento y reparaciones.", UI::orange());
            dibujarListado();
        }
        else
        {
            DrawText("Reportes de mantenimiento", 240, 95, 30, UI::navy());
            UI::aviso(Rectangle{240, 160, 740, 110}, "Reporte", "La flota se consulta desde la base y los cambios se reflejan en Operaciones de Vuelo.", UI::orange());
            dibujarListado();
        }

        EndDrawing();
    }
}
