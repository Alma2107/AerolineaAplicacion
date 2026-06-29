#include "ModuloEquipaje.h"
#include "ComponentesUI.h"
#include "raylib.h"
#include <cctype>
#include <sstream>

ModuloEquipaje::ModuloEquipaje()
    : mensaje("Modulo activo."), vista(0), foco(0), idTicket("1"), idTipo("2"), peso("23.5"), precio("14000"), etiqueta("TAG-1001"), vistaAnterior(-1), notificacionesNoLeidas(0)
{
    refrescarDatos();
}

void ModuloEquipaje::refrescarAvisos()
{
    notificacionesNoLeidas = notificacionDAO.contarNoLeidas(3);
    notificacionesCache = notificacionDAO.listarPorModulo(3);
}

void ModuloEquipaje::refrescarDatos()
{
    if (vista == 2)
        equipajesCache = equipajeDAO.listar();
    if (vista == 4)
        refrescarAvisos();
    else
        refrescarAvisos();
    vistaAnterior = vista;
}

void ModuloEquipaje::dibujarNavegacion()
{
    std::vector<std::string> opciones = {"Registrar equipaje", "Buscar equipaje", "Equipajes", "Perdido/danado", "Avisos"};
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
        {
            etiqueta = equipaje.getCodigoEtiqueta();
            refrescarDatos();
        }
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
    DrawText("Ultimos equipajes cargados y su estado actual", 240, 130, 17, UI::muted());

    Rectangle panel = {240, 165, 990, 540};
    DrawRectangleRounded(Rectangle{panel.x, panel.y + 6, panel.width, panel.height}, 0.035f, 8, Color{224, 232, 242, 255});
    DrawRectangleRounded(panel, 0.03f, 8, WHITE);
    DrawRectangleLinesEx(panel, 1, UI::border());
    DrawRectangleRounded(Rectangle{panel.x, panel.y, 8, panel.height}, 0.03f, 8, UI::purple());

    DrawText("Listado general", 265, 190, 22, UI::navy());
    std::string total = std::to_string(equipajesCache.size()) + " registros";
    int totalAncho = MeasureText(total.c_str(), 16);
    Rectangle contador = {panel.x + panel.width - totalAncho - 52, 184, (float)totalAncho + 28, 32};
    DrawRectangleRounded(contador, 0.24f, 8, Fade(UI::purple(), 0.12f));
    DrawText(total.c_str(), (int)contador.x + 14, (int)contador.y + 8, 16, UI::purple());

    Rectangle cabecera = {265, 240, 930, 42};
    DrawRectangleRounded(cabecera, 0.08f, 8, Fade(UI::purple(), 0.10f));
    DrawText("ID", 285, 253, 15, UI::purple());
    DrawText("Ticket", 370, 253, 15, UI::purple());
    DrawText("Etiqueta", 510, 253, 15, UI::purple());
    DrawText("Precio", 720, 253, 15, UI::purple());
    DrawText("Estado", 930, 253, 15, UI::purple());

    if (equipajesCache.empty())
    {
        DrawRectangleRounded(Rectangle{265, 315, 930, 95}, 0.04f, 8, Color{248, 250, 253, 255});
        DrawRectangleLinesEx(Rectangle{265, 315, 930, 95}, 1, UI::border());
        DrawText("No hay equipajes registrados todavia.", 295, 348, 20, UI::muted());
        DrawText("Cuando registre un equipaje, aparecera en este cuadro.", 295, 378, 16, UI::muted());
        return;
    }

    int y = 300;
    for (int i = 0; i < (int)equipajesCache.size() && i < 7; ++i)
    {
        const Equipaje &equipaje = equipajesCache[i];
        Rectangle fila = {265, (float)y, 930, 52};
        Color fondo = i % 2 == 0 ? Color{248, 250, 253, 255} : WHITE;
        Color estadoColor = equipaje.getEstado() == "Perdido" ? RED : equipaje.getEstado() == "Danado" ? UI::orange()
                                                                                                       : UI::purple();
        DrawRectangleRounded(fila, 0.035f, 8, fondo);
        DrawRectangleLinesEx(fila, 1, Color{226, 234, 244, 255});

        std::stringstream precioPagado;
        precioPagado << "$" << equipaje.getPeso();
        std::string ticket = "#" + std::to_string(equipaje.getIdPasajero());

        DrawText(("#" + std::to_string(equipaje.getId())).c_str(), 285, y + 17, 16, UI::navy());
        DrawText(ticket.c_str(), 370, y + 17, 16, DARKGRAY);
        UI::textoRecortado(equipaje.getCodigoEtiqueta(), 510, y + 17, 17, UI::navy(), 20);
        UI::textoRecortado(precioPagado.str(), 720, y + 17, 16, DARKGRAY, 16);
        DrawRectangleRounded(Rectangle{930, (float)y + 12, 112, 28}, 0.35f, 8, Fade(estadoColor, 0.12f));
        UI::textoRecortado(equipaje.getEstado(), 942, y + 19, 14, estadoColor, 13);

        y += 58;
    }
}

void ModuloEquipaje::dibujarAvisos()
{
    DrawText("Avisos de objetos perdidos", 240, 95, 30, UI::navy());
    DrawText("Notificaciones enviadas por Flota y Mantenimiento", 240, 130, 17, UI::muted());

    int y = 180;
    if (notificacionesCache.empty())
    {
        UI::aviso(Rectangle{240, 180, 760, 88}, "Sin avisos", "No hay objetos pendientes para verificar.", UI::purple());
    }
    for (const Notificacion &notificacion : notificacionesCache)
    {
        UI::aviso(Rectangle{240, (float)y, 880, 78}, notificacion.getTipo(), notificacion.getMensaje(), UI::purple());

        if (notificacion.getTipo().find("Objeto perdido") != std::string::npos)
        {
            Rectangle accion = {240, (float)y + 84, 220, 34};
            if (UI::botonSecundario(accion, "Notificar Pasajeros", UI::green()))
            {
                std::string mensajePasajero;
                std::string codigoEquipajeEncontrado;
                size_t pos = notificacion.getMensaje().find("codigo:");
                if (pos != std::string::npos)
                {
                    pos += 6;
                    while (pos < notificacion.getMensaje().size() && std::isspace((unsigned char)notificacion.getMensaje()[pos]))
                        pos++;
                    size_t end = notificacion.getMensaje().find_first_of(". ,)", pos);
                    if (end == std::string::npos)
                        end = notificacion.getMensaje().size();
                    codigoEquipajeEncontrado = notificacion.getMensaje().substr(pos, end - pos);
                }

                if (!codigoEquipajeEncontrado.empty())
                {
                    std::string propietario = equipajeDAO.buscarPropietarioPorEtiqueta(codigoEquipajeEncontrado);
                    if (!propietario.empty())
                    {
                        mensajePasajero = "Equipaje " + codigoEquipajeEncontrado + " corresponde a " + propietario + ". Coordinar devolucion con el pasajero.";
                    }
                    else
                    {
                        mensajePasajero = "No se identifico propietario para el equipaje " + codigoEquipajeEncontrado + ". Verificar con Atencion al Pasajero.";
                    }
                }
                else
                {
                    mensajePasajero = "Objeto perdido verificado. Verificar propietario y coordinar devolucion. " + notificacion.getMensaje();
                }

                notificacionDAO.crear(2, "Objeto perdido - Propietario identificado", mensajePasajero);
                mensaje = "Se notifico a Atencion al Pasajero para contactar al pasajero.";
                refrescarAvisos();
            }
        }

        y += 92;
    }

    if (UI::botonSecundario(Rectangle{240, 720, 180, 42}, "Marcar leidas", UI::purple()))
    {
        notificacionDAO.marcarLeidas(3);
        refrescarAvisos();
    }

    if (!mensaje.empty())
        DrawText(mensaje.c_str(), 240, 768, 18, UI::green());
}

void ModuloEquipaje::mostrar()
{
    while (!WindowShouldClose())
    {
        if (vistaAnterior != vista)
            refrescarDatos();

        BeginDrawing();
        UI::shellModulo("Modulo 3 / Equipaje", "Registro, rastreo y gestion de equipaje", UI::purple(), notificacionesNoLeidas);
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
        else if (vista == 2)
            dibujarListado();
        else if (vista == 4)
            dibujarAvisos();
        else
            dibujarRastrear();

        EndDrawing();
    }
}
