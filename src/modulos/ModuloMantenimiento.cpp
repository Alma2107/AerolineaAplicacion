#include "ModuloMantenimiento.h"
#include "ComponentesUI.h"
#include "raylib.h"
#include <algorithm>
#include <sstream>
#include <ctime>

static std::string obtenerFechaHoraActual()
{
    time_t ahora = time(NULL);
    tm tiempoLocal;
    localtime_s(&tiempoLocal, &ahora);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tiempoLocal);
    return std::string(buffer);
}

static std::string estadoVueloSegunTiempo(const Vuelo &vuelo)
{
    std::string ahora = obtenerFechaHoraActual();
    const std::string &salida = vuelo.getFechaSalida();
    const std::string &llegada = vuelo.getFechaLlegada();
    if (salida.empty() || llegada.empty())
        return "Sin fechas";
    if (ahora < salida)
        return "Programado";
    if (ahora >= salida && ahora <= llegada)
        return "En vuelo";
    return "Aterrizado";
}

ModuloMantenimiento::ModuloMantenimiento()
    : mensaje("Modulo activo."), vista(0), foco(0), idAvion("4"), estado("Mantenimiento"),
      modeloNuevo("Airbus A320neo"), capacidadNueva("180"), estadoNuevo("Activo"),
      avionEquipajeHallado("1"), asientoSeleccionadoMapa(""), codigoEtiquetaEncontrado("TAG-001"),
      vistaAnterior(-1), avionMapaSeleccionado(0)
{
    refrescarDatos();
}

void ModuloMantenimiento::refrescarDatos()
{
    avionesCache = avionDAO.listar();
    vistaAnterior = vista;
}

void ModuloMantenimiento::dibujarNavegacion()
{
    std::vector<std::string> opciones = {"Estado de flota", "Aviones", "Registrar avion", "Actualizar estado", "PIMA", "Reportes"};
    UI::menuLateral("Flota y Mantenimiento", opciones, vista, UI::orange());

    for (int i = 0; i < (int)opciones.size(); ++i)
    {
        Rectangle item = {18, (float)(92 + i * 50), 174, 42};
        if (CheckCollisionPointRec(GetMousePosition(), item) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            vista = i;
    }

    if (UI::botonSecundario(Rectangle{1000, 90, 220, 42}, "Volver al menú principal", UI::orange()))
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
    if (UI::botonSecundario(Rectangle{1080, 20, 220, 42}, "Volver al menú principal", UI::orange()))
    {
        vista = -1;
        return;
    }
    DrawText("Estado de flota", 240, 95, 30, UI::navy());
    dibujarListado();
}

void ModuloMantenimiento::dibujarMapaAviones()
{
    DrawText("Aviones", 240, 95, 30, UI::navy());
    DrawText("Mapa operativo de aeronaves con estado actualizado desde la base", 240, 130, 17, UI::muted());

    Rectangle mapa = {240, 165, 760, 465};
    DrawRectangleRounded(Rectangle{mapa.x, mapa.y + 6, mapa.width, mapa.height}, 0.035f, 8, Color{224, 232, 242, 255});
    DrawRectangleRounded(mapa, 0.03f, 8, WHITE);
    DrawRectangleLinesEx(mapa, 1, UI::border());
    DrawRectangleRounded(Rectangle{mapa.x, mapa.y, 8, mapa.height}, 0.03f, 8, UI::orange());
    DrawText("Mapa de flota", 265, 190, 22, UI::navy());
    DrawRectangleRounded(Rectangle{820, 188, 145, 30}, 0.28f, 8, Fade(UI::green(), 0.12f));
    DrawText("Tiempo real", 842, 196, 15, UI::green());
    DrawText("Leyenda: verde programado, azul en vuelo, gris aterrizado, naranja mantenimiento", 240, 220, 16, UI::muted());

    DrawLineEx(Vector2{310, 410}, Vector2{455, 310}, 3, Fade(UI::blue(), 0.22f));
    DrawLineEx(Vector2{455, 310}, Vector2{650, 350}, 3, Fade(UI::blue(), 0.22f));
    DrawLineEx(Vector2{650, 350}, Vector2{865, 260}, 3, Fade(UI::blue(), 0.22f));
    DrawLineEx(Vector2{310, 410}, Vector2{565, 500}, 3, Fade(UI::orange(), 0.18f));
    DrawLineEx(Vector2{565, 500}, Vector2{870, 480}, 3, Fade(UI::orange(), 0.18f));
    DrawCircle(310, 410, 6, UI::blue());
    DrawText("AEP", 292, 425, 14, UI::muted());
    DrawCircle(455, 310, 6, UI::blue());
    DrawText("COR", 437, 325, 14, UI::muted());
    DrawCircle(650, 350, 6, UI::blue());
    DrawText("SCL", 632, 365, 14, UI::muted());
    DrawCircle(865, 260, 6, UI::blue());
    DrawText("MAD", 847, 275, 14, UI::muted());
    DrawCircle(565, 500, 6, UI::orange());
    DrawText("MDZ", 547, 515, 14, UI::muted());
    DrawCircle(870, 480, 6, UI::orange());
    DrawText("MIA", 852, 495, 14, UI::muted());

    if (avionMapaSeleccionado == 0 && !avionesCache.empty())
        avionMapaSeleccionado = avionesCache[0].getId();

    int detalleId = avionMapaSeleccionado;
    Avion avionSeleccionado;
    for (int i = 0; i < (int)avionesCache.size(); ++i)
    {
        const Avion &avion = avionesCache[i];
        std::vector<Vuelo> vuelos = vueloDAO.listarPorAvion(avion.getId());
        bool enVuelo = false;
        bool programado = false;
        bool aterrizado = false;
        for (const Vuelo &vuelo : vuelos)
        {
            std::string estadoVuelo = estadoVueloSegunTiempo(vuelo);
            if (estadoVuelo == "En vuelo")
                enVuelo = true;
            else if (estadoVuelo == "Programado")
                programado = true;
            else if (estadoVuelo == "Aterrizado")
                aterrizado = true;
        }

        std::string estadoOperativo = avion.getEstado();
        if (estadoOperativo == "Activo")
        {
            if (enVuelo)
                estadoOperativo = "En vuelo";
            else if (programado)
                estadoOperativo = "Programado";
            else if (aterrizado)
                estadoOperativo = "Aterrizado";
        }

        float x = 330.0f + (float)((avion.getId() * 137) % 560);
        float y;
        if (estadoOperativo == "En vuelo")
            y = 310.0f + (float)((avion.getId() * 43) % 120);
        else if (estadoOperativo == "Programado")
            y = 255.0f + (float)((avion.getId() * 53) % 190);
        else if (estadoOperativo == "Aterrizado")
            y = 520.0f + (float)((avion.getId() * 23) % 50);
        else if (estadoOperativo == "Mantenimiento")
            y = 500.0f;
        else
            y = 545.0f;

        Color estadoColor = estadoOperativo == "Programado" ? UI::green()
                             : estadoOperativo == "En vuelo" ? UI::blue()
                             : estadoOperativo == "Aterrizado" ? UI::muted()
                             : estadoOperativo == "Mantenimiento" ? UI::orange()
                             : RED;
        bool seleccionado = avion.getId() == avionMapaSeleccionado;
        Rectangle hit = {x - 18, y - 18, 36, 36};
        if (CheckCollisionPointRec(GetMousePosition(), hit) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            avionMapaSeleccionado = avion.getId();
            avionEquipajeHallado = std::to_string(avion.getId());
            detalleId = avion.getId();
        }

        DrawCircle((int)x, (int)y, seleccionado ? 16 : 13, Fade(estadoColor, seleccionado ? 0.28f : 0.18f));
        DrawCircle((int)x, (int)y, seleccionado ? 8 : 6, estadoColor);
        DrawText(std::to_string(avion.getId()).c_str(), (int)x - 4, (int)y - 30, 14, UI::navy());
        if (avion.getId() == detalleId)
            avionSeleccionado = avion;
    }

    Rectangle detalle = {1030, 165, 300, 465};
    DrawRectangleRounded(detalle, 0.03f, 8, WHITE);
    DrawRectangleLinesEx(detalle, 1, UI::border());
    DrawText("Detalle", 1055, 190, 22, UI::navy());
    if (avionSeleccionado.getId() != 0)
    {
        std::vector<Vuelo> vuelos = vueloDAO.listarPorAvion(avionSeleccionado.getId());
        bool enVuelo = false;
        bool programado = false;
        bool aterrizado = false;
        for (const Vuelo &vuelo : vuelos)
        {
            std::string estadoVuelo = estadoVueloSegunTiempo(vuelo);
            if (estadoVuelo == "En vuelo")
                enVuelo = true;
            else if (estadoVuelo == "Programado")
                programado = true;
            else if (estadoVuelo == "Aterrizado")
                aterrizado = true;
        }

        std::string estadoOperativo = avionSeleccionado.getEstado();
        if (estadoOperativo == "Activo")
        {
            if (enVuelo)
                estadoOperativo = "En vuelo";
            else if (programado)
                estadoOperativo = "Programado";
            else if (aterrizado)
                estadoOperativo = "Aterrizado";
        }

        Color estadoColor = estadoOperativo == "Programado" ? UI::green()
                             : estadoOperativo == "En vuelo" ? UI::blue()
                             : estadoOperativo == "Aterrizado" ? UI::muted()
                             : avionSeleccionado.getEstado() == "Mantenimiento" ? UI::orange()
                             : RED;
        DrawText(("#" + std::to_string(avionSeleccionado.getId())).c_str(), 1055, 235, 26, UI::orange());
        UI::textoRecortado(avionSeleccionado.getModelo(), 1055, 274, 18, UI::navy(), 28);
        DrawText(("Capacidad: " + std::to_string(avionSeleccionado.getCapacidad())).c_str(), 1055, 312, 17, DARKGRAY);
        DrawRectangleRounded(Rectangle{1055, 345, 170, 30}, 0.28f, 8, Fade(estadoColor, 0.12f));
        UI::textoRecortado(estadoOperativo, 1070, 353, 15, estadoColor, 18);

        DrawText("Plano de asientos", 1055, 395, 18, UI::muted());
        if (estadoOperativo == "En vuelo" || estadoOperativo == "Aterrizado")
        {
            dibujarPlanoAsientos(avionSeleccionado);
        }
        else
        {
            DrawText("Plano oculto: disponible solo mientras el vuelo está en movimiento o aterrizado.", 1055, 420, 14, UI::muted());
        }

        DrawText("Vuelos asociados", 1055, 520, 18, UI::muted());
        int y = 445;
        if (vuelos.empty())
        {
            DrawText("Sin vuelos asociados.", 1055, 445, 16, UI::muted());
        }
        for (int i = 0; i < (int)vuelos.size() && i < 4; ++i)
        {
            std::string estadoVuelo = estadoVueloSegunTiempo(vuelos[i]);
            std::string linea = vuelos[i].getNumero() + " " + vuelos[i].getOrigen() + "-" + vuelos[i].getDestino() + " (" + estadoVuelo + ")";
            UI::textoRecortado(linea, 1055, y, 14, DARKGRAY, 29);
            y += 28;
        }
    }

    DrawText("Haga click en un avion para ver su detalle operacional.", 240, 660, 17, UI::muted());
}

void ModuloMantenimiento::dibujarPlanoAsientos(const Avion &avion)
{
    const int capacidad = avion.getCapacidad();
    if (capacidad <= 0)
    {
        DrawText("Sin datos de capacidad.", 1055, 445, 16, UI::muted());
        return;
    }

    Rectangle panel = {1055, 430, 280, 180};
    DrawRectangleRounded(panel, 0.03f, 8, Color{245, 248, 252, 255});
    DrawRectangleLinesEx(panel, 1, UI::border());

    float seatAreaWidth = panel.width - 20;
    float seatAreaHeight = panel.height - 30;
    float hGap = 4.0f;
    float vGap = 4.0f;

    int maxCols = std::min(capacidad, 16);
    int cols = 4;
    float bestSize = 0.0f;
    for (int testCols = 4; testCols <= maxCols; ++testCols)
    {
        int testRows = (capacidad + testCols - 1) / testCols;
        float testWidth = (seatAreaWidth - (testCols - 1) * hGap) / testCols;
        float testHeight = (seatAreaHeight - (testRows - 1) * vGap) / testRows;
        float testSize = std::min(testWidth, testHeight);
        if (testSize > bestSize)
        {
            bestSize = testSize;
            cols = testCols;
        }
    }

    if (bestSize < 6.0f)
        bestSize = 6.0f;
    float seatSize = std::min(20.0f, bestSize);
    int rows = (capacidad + cols - 1) / cols;
    float totalWidth = cols * seatSize + (cols - 1) * hGap;
    float totalHeight = rows * seatSize + (rows - 1) * vGap;
    float startX = panel.x + (panel.width - totalWidth) / 2;
    float startY = panel.y + 12;

    bool compact = seatSize < 10.0f;
    int labelSize = seatSize >= 12.0f ? 12 : seatSize >= 9.0f ? 8 : 6;
    bool drawLabels = seatSize >= 9.0f;

    BeginScissorMode((int)panel.x, (int)panel.y, (int)panel.width, (int)panel.height);
    int index = 0;
    for (int fila = 0; fila < rows; ++fila)
    {
        for (int col = 0; col < cols && index < capacidad; ++col, ++index)
        {
            std::string label = std::to_string(fila + 1) + (char)('A' + col);
            Rectangle seatRect = {startX + col * (seatSize + hGap), startY + fila * (seatSize + vGap), seatSize, seatSize};
            bool seleccionado = asientoSeleccionadoMapa == label;
            Color fill = seleccionado ? UI::orange() : Fade(UI::green(), compact ? 0.08f : 0.12f);
            DrawRectangleRounded(seatRect, 0.18f, 4, fill);
            DrawRectangleRoundedLines(seatRect, 0.18f, 2, seleccionado ? UI::orange() : UI::border());
            if (drawLabels)
            {
                int textWidth = MeasureText(label.c_str(), labelSize);
                DrawText(label.c_str(), (int)(seatRect.x + (seatSize - textWidth) / 2), (int)(seatRect.y + (seatSize - labelSize) / 2), labelSize, DARKGRAY);
            }
            if (CheckCollisionPointRec(GetMousePosition(), seatRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                asientoSeleccionadoMapa = label;
        }
    }
    EndScissorMode();

    if (compact)
    {
        DrawText("Plano compacto", panel.x + 10, panel.y + panel.height - 20, 13, UI::muted());
    }

    std::string seleccion = asientoSeleccionadoMapa.empty() ? "Ninguno" : asientoSeleccionadoMapa;
    DrawText(("Asiento seleccionado: " + seleccion).c_str(), panel.x + 10, panel.y + panel.height - 20, 15, UI::muted());
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
            refrescarDatos();
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

    DrawText("Seleccione aeronave", 240, 168, 17, UI::muted());
    int xAvion = 240;
    int yAvion = 194;
    for (int i = 0; i < (int)avionesCache.size() && i < 8; ++i)
    {
        const Avion &avion = avionesCache[i];
        std::stringstream texto;
        texto << "#" << avion.getId() << " " << avion.getModelo() << " - " << avion.getEstado();
        if (UI::botonSecundario(Rectangle{(float)xAvion, (float)yAvion, 330, 32}, texto.str(), std::to_string(avion.getId()) == idAvion ? UI::orange() : UI::muted()))
            idAvion = std::to_string(avion.getId());
        yAvion += 38;
        if (yAvion > 310)
        {
            yAvion = 194;
            xAvion += 350;
        }
    }
    if (UI::input(Rectangle{240, 360, 260, 46}, "Estado", estado, foco == 2))
        foco = 2;
    if (foco == 2)
        UI::capturarTexto(estado, 30);

    if (UI::botonSecundario(Rectangle{530, 360, 180, 42}, "Mantenimiento", UI::orange()))
        estado = "Mantenimiento";
    if (UI::botonSecundario(Rectangle{730, 360, 180, 42}, "En reparacion", RED))
        estado = "En reparacion";
    if (UI::botonSecundario(Rectangle{930, 360, 180, 42}, "Activo", UI::green()))
        estado = "Activo";

    if (UI::boton(Rectangle{240, 430, 210, 44}, "Guardar estado", UI::orange()))
    {
        int avion = ConexionDB::convertirEntero(idAvion);
        bool ok = avionDAO.actualizarEstado(avion, estado);
        if (ok)
        {
            bool aviso = notificarOperacion(avion, estado);
            mensaje = "Estado actualizado en BD.";
            if (!aviso)
                mensaje += " No se pudo guardar aviso en BD: revise tabla notificaciones.";
            refrescarDatos();
        }
        else
        {
            mensaje = "No se pudo actualizar la aeronave.";
        }
    }

    DrawText(mensaje.c_str(), 240, 485, 19, UI::green());
    dibujarListado();
}

void ModuloMantenimiento::dibujarListado()
{
    int y = vista == 0 ? 155 : 500;
    DrawText("ID   Modelo                         Capacidad   Estado        Operativo", 240, y, 17, UI::muted());
    y += 34;
    for (const Avion &avion : avionesCache)
    {
        std::vector<Vuelo> vuelos = vueloDAO.listarPorAvion(avion.getId());
        bool enVuelo = false;
        bool programado = false;
        bool aterrizado = false;
        for (const Vuelo &vuelo : vuelos)
        {
            std::string estadoVuelo = estadoVueloSegunTiempo(vuelo);
            if (estadoVuelo == "En vuelo")
                enVuelo = true;
            else if (estadoVuelo == "Programado")
                programado = true;
            else if (estadoVuelo == "Aterrizado")
                aterrizado = true;
        }
        std::string estadoOperativo = avion.getEstado();
        if (estadoOperativo == "Activo")
        {
            if (enVuelo)
                estadoOperativo = "En vuelo";
            else if (programado)
                estadoOperativo = "Programado";
            else if (aterrizado)
                estadoOperativo = "Aterrizado";
        }
        std::stringstream ss;
        ss << avion.getId() << "   " << avion.getModelo() << "        " << avion.getCapacidad() << "        " << avion.getEstado() << "        " << estadoOperativo;
        UI::textoRecortado(ss.str(), 240, y, 18, avion.getEstado() == "Activo" ? DARKGRAY : RED, 100);
        y += 34;
    }
}

void ModuloMantenimiento::mostrar()
{
    while (!WindowShouldClose())
    {
        if (vistaAnterior != vista)
            refrescarDatos();

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
            dibujarMapaAviones();
        else if (vista == 2)
            dibujarRegistrarAeronave();
        else if (vista == 3)
            dibujarActualizarEstado();
        else if (vista == 4)
        {
            DrawText("Plan de Inspeccion y Mantenimiento (PIMA)", 240, 95, 30, UI::navy());
            UI::aviso(Rectangle{240, 150, 740, 80}, "PIMA", "Use Actualizar estado para registrar limpieza, inspeccion, mantenimiento y reparaciones.", UI::orange());
            DrawText("Equipaje perdido encontrado", 240, 260, 24, UI::navy());
            DrawText("Seleccione avion y cargue el codigo de etiqueta para avisar a Equipaje.", 240, 292, 16, UI::muted());
            int yAv = 330;
            for (int i = 0; i < (int)avionesCache.size() && i < 6; ++i)
            {
                const Avion &avion = avionesCache[i];
                std::stringstream texto;
                texto << "#" << avion.getId() << " " << avion.getModelo();
                if (UI::botonSecundario(Rectangle{240, (float)yAv, 280, 30}, texto.str(), std::to_string(avion.getId()) == avionEquipajeHallado ? UI::orange() : UI::muted()))
                    avionEquipajeHallado = std::to_string(avion.getId());
                yAv += 36;
            }
            if (UI::input(Rectangle{560, 330, 220, 46}, "Codigo etiqueta", codigoEtiquetaEncontrado, foco == 30))
                foco = 30;
            if (foco == 30)
                UI::capturarTexto(codigoEtiquetaEncontrado, 20);
            if (UI::boton(Rectangle{800, 330, 210, 44}, "Avisar equipaje", UI::orange()))
            {
                if (codigoEtiquetaEncontrado.empty())
                {
                    mensaje = "Debe ingresar el codigo de etiqueta del equipaje encontrado.";
                }
                else
                {
                    std::string texto = "Mantenimiento encontro equipaje perdido en avion " + avionEquipajeHallado + " el " + obtenerFechaHoraActual() + " con codigo: " + codigoEtiquetaEncontrado + ". Validar contra reclamos del vuelo asociado.";
                    bool ok = notificacionDAO.crear(3, "Equipaje perdido encontrado", texto);
                    mensaje = ok ? "Aviso enviado al modulo Equipaje." : "No se pudo enviar aviso a Equipaje.";
                }
            }
            DrawText(mensaje.c_str(), 560, 395, 18, mensaje.find("enviado") != std::string::npos ? UI::green() : RED);
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
