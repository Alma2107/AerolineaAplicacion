#include "ModuloEquipaje.h"
#include "ComponentesUI.h"
#include "raylib.h"
#include <stream>

ModuloEquipaje::ModuloEquipaje()
    : mensaje("Modulo activo."), vista(0), foco(0), idTicket("1"), idTipo("2"), peso("23.5"), precio("14000"), etiqueta("TAG-1001") {}

void ModuloEquipaje::dibujarNavegacion()
{
    std::vector<std::string> opciones = {"Registrar equipaje", "Buscar equipaje", "Equipajes", "Perdido/danado"};
    UI::menuLateral("Equipaje", opciones, vista, UI::purple());

    for (int i = 0; i < (int)opciones.size(); ++i)
    {
        Rectangle item = {18, (float)(92 + i * 50), 174, 42};
        if (CheckCollisionPointRec(GetMousePosition(), item) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            vista = i;
    }

    if (UI::botonSecundario(Rectangle{24, 748, 160, 42}, "Volver al inicio", UI::purple()))
    {
        vista = -1;
        return;
    }
}

void ModuloEquipaje::dibujarRegistrar()
{
    DrawText("Registrar equipaje", 240, 95, 30, UI::navy());
    DrawText("Registra el equipaje en ticket_equipajes", 240, 130, 17, UI::muted());

    if (UI::input(Rectangle{240, 190, 180, 46}, "ID ticket", idTicket, foco == 1))
        foco = 1;
    if (UI::input(Rectangle{450, 190, 180, 46}, "ID tipo equipaje", idTipo, foco == 2))
        foco = 2;
    if (UI::input(Rectangle{660, 190, 180, 46}, "Peso kg", peso, foco == 3))
        foco = 3;
    if (UI::input(Rectangle{870, 190, 180, 46}, "Precio", precio, foco == 4))
        foco = 4;

    if (foco == 1)
        UI::capturarTexto(idTicket, 8);
    if (foco == 2)
        UI::capturarTexto(idTipo, 8);
    if (foco == 3)
        UI::capturarTexto(peso, 8);
    if (foco == 4)
        UI::capturarTexto(precio, 12);

    if (UI::boton(Rectangle{240, 285, 210, 44}, "Registrar equipaje", UI::purple()))
    {
        Equipaje equipaje = equipajeDAO.registrar(ConexionDB::convertirEntero(idTicket), ConexionDB::convertirEntero(idTipo),
                                                  ConexionDB::convertirDouble(peso), ConexionDB::convertirDouble(precio));
        mensaje = equipaje.getId() != 0 ? "Equipaje guardado: " + equipaje.getCodigoEtiqueta() : "No se pudo guardar el equipaje.";
        if (equipaje.getId() != 0)
            etiqueta = equipaje.getCodigoEtiqueta();
    }

    DrawText(mensaje.c_str(), 240, 370, 19, UI::green());
}

void ModuloEquipaje::dibujarRastrear()
{
    DrawText(vista == 1 ? "Buscar equipaje" : "Gestionar equipaje perdido o danado", 240, 95, 30, UI::navy());

    if (UI::input(Rectangle{240, 180, 240, 46}, "Codigo etiqueta", etiqueta, foco == 5))
        foco = 5;
    if (foco == 5)
        UI::capturarTexto(etiqueta, 20);

    if (UI::boton(Rectangle{505, 180, 160, 44}, "Rastrear", UI::purple()))
    {
        Equipaje equipaje = equipajeDAO.rastrear(etiqueta);
        mensaje = equipaje.getId() != 0 ? etiqueta + " esta: " + equipaje.getEstado() : "Etiqueta no encontrada.";
    }

    if (vista == 3)
    {
        if (UI::botonSecundario(Rectangle{240, 270, 180, 42}, "Marcar perdido", RED))
            mensaje = equipajeDAO.actualizarEstado(etiqueta, "Perdido") ? "Equipaje marcado como perdido." : "Etiqueta no encontrada.";
        if (UI::botonSecundario(Rectangle{450, 270, 180, 42}, "Marcar danado", UI::orange()))
            mensaje = equipajeDAO.actualizarEstado(etiqueta, "Danado") ? "Equipaje marcado como danado." : "Etiqueta no encontrada.";
    }

    DrawText(mensaje.c_str(), 240, 355, 19, UI::green());
}

void ModuloEquipaje::dibujarListado()
{
    DrawText("Equipajes registrados", 240, 95, 30, UI::navy());
    int y = 150;
    DrawText("ID   Ticket   Etiqueta      Precio/Peso   Estado", 240, y, 17, UI::muted());
    y += 32;
    for (const Equipaje &equipaje : equipajeDAO.listar())
    {
        std::stringstream ss;
        ss << equipaje.getId() << "   " << equipaje.getIdPasajero() << "       "
           << equipaje.getCodigoEtiqueta() << "     " << equipaje.getPeso()
           << "        " << equipaje.getEstado();
        DrawText(ss.str().c_str(), 240, y, 18, DARKGRAY);
        y += 34;
    }
}

void ModuloEquipaje::mostrar()
{
    while (!WindowShouldClose())
    {
        BeginDrawing();
        UI::shellModulo("Modulo 3 / Equipaje", "Registro, rastreo y gestion de equipaje", UI::purple(), 0);
        dibujarNavegacion();
        if (vista < 0)
        {
            EndDrawing();
            return;
        }

        if (vista == 0)
            dibujarRegistrar();
        else if (vista == 1 || vista == 3)
            dibujarRastrear();
        else
            dibujarListado();

        EndDrawing();
    }
}
