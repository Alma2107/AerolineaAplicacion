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
      objetoEncontrado("Auriculares"), avionObjeto("1"), numeroAsiento("14A"),
      codigoEquipaje(""), fechaHallazgo("2026-06-29 10:30:00"), ubicacionHallazgo("Cabina central"),
      empleadoHallazgo("mantenimiento"), asientoSeleccionadoMapa(""), vistaAnterior(-1), avionMapaSeleccionado(0)
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
    std::vector<std::string> opciones = {"Estado de flota", "Aviones", "Registrar avion", "Actualizar estado", "Objetos perdidos", "PIMA", "Reportes"};
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
            avionObjeto = std::to_string(avion.getId());
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
            bool seleccionado = numeroAsiento == label;
            Color fill = seleccionado ? UI::orange() : Fade(UI::green(), compact ? 0.08f : 0.12f);
            DrawRectangleRounded(seatRect, 0.18f, 4, fill);
            DrawRectangleRoundedLines(seatRect, 0.18f, 2, seleccionado ? UI::orange() : UI::border());
            if (drawLabels)
            {
                int textWidth = MeasureText(label.c_str(), labelSize);
                DrawText(label.c_str(), (int)(seatRect.x + (seatSize - textWidth) / 2), (int)(seatRect.y + (seatSize - labelSize) / 2), labelSize, DARKGRAY);
            }
            if (CheckCollisionPointRec(GetMousePosition(), seatRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                numeroAsiento = label;
        }
    }
    EndScissorMode();

    if (compact)
    {
        DrawText("Plano compacto", panel.x + 10, panel.y + panel.height - 20, 13, UI::muted());
    }

    std::string seleccion = numeroAsiento.empty() ? "Ninguno" : numeroAsiento;
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
            refrescarDatos();
        }
        else
        {
            mensaje = "No se pudo actualizar la aeronave.";
        }
    }

    DrawText(mensaje.c_str(), 240, 440, 19, UI::green());
    dibujarListado();
}

void ModuloMantenimiento::dibujarRegistroObjetosPerdidos()
{
    DrawText("Registro de Objetos Perdidos", 240, 95, 30, UI::navy());
    DrawText("Cargue hallazgos encontrados durante inspecciones o reparaciones", 240, 130, 17, UI::muted());

    Rectangle panel = {240, 165, 940, 440};
    DrawRectangleRounded(Rectangle{panel.x, panel.y + 6, panel.width, panel.height}, 0.035f, 8, Color{224, 232, 242, 255});
    DrawRectangleRounded(panel, 0.03f, 8, WHITE);
    DrawRectangleLinesEx(panel, 1, UI::border());
    DrawRectangleRounded(Rectangle{panel.x, panel.y, 8, panel.height}, 0.03f, 8, UI::orange());
    DrawText("Registro de equipaje perdido", 265, 190, 22, UI::navy());

    DrawText("Tipo: Equipaje", 265, 250, 18, UI::muted());

    if (UI::input(Rectangle{265, 310, 360, 46}, "Descripcion detallada", objetoEncontrado, foco == 30))
        foco = 30;
    if (UI::input(Rectangle{655, 310, 140, 46}, "ID avion", avionObjeto, foco == 31))
        foco = 31;
    if (UI::input(Rectangle{825, 310, 300, 46}, "Fecha YYYY-MM-DD HH:MM:SS", fechaHallazgo, foco == 32))
        foco = 32;
    if (UI::input(Rectangle{265, 390, 360, 46}, "Ubicacion exacta", ubicacionHallazgo, foco == 33))
        foco = 33;
    if (UI::input(Rectangle{655, 390, 140, 46}, "Asiento", numeroAsiento, foco == 34))
        foco = 34;
    if (UI::input(Rectangle{825, 390, 300, 46}, "Codigo equipaje", codigoEquipaje, foco == 35))
        foco = 35;
    if (UI::input(Rectangle{265, 470, 360, 46}, "Empleado que encontro", empleadoHallazgo, foco == 36))
        foco = 36;

    if (foco == 30)
        UI::capturarTexto(objetoEncontrado, 120);
    if (foco == 31)
        UI::capturarTexto(avionObjeto, 8);
    if (foco == 32)
        UI::capturarTexto(fechaHallazgo, 25);
    if (foco == 33)
        UI::capturarTexto(ubicacionHallazgo, 80);
    if (foco == 34)
        UI::capturarTexto(numeroAsiento, 6);
    if (foco == 35)
        UI::capturarTexto(codigoEquipaje, 12);
    if (foco == 36)
        UI::capturarTexto(empleadoHallazgo, 60);

    int chipX = 265;
    DrawText("Aviones", 265, 520, 17, UI::muted());
    for (const Avion &avion : avionesCache)
    {
        Rectangle chip = {(float)chipX, 550, 74, 32};
        bool activo = avionObjeto == std::to_string(avion.getId());
        DrawRectangleRounded(chip, 0.28f, 8, activo ? Fade(UI::orange(), 0.16f) : WHITE);
        DrawRectangleLinesEx(chip, 1, activo ? UI::orange() : UI::border());
        DrawText(("#" + std::to_string(avion.getId())).c_str(), chipX + 18, 559, 15, activo ? UI::orange() : UI::muted());
        if (CheckCollisionPointRec(GetMousePosition(), chip) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            avionObjeto = std::to_string(avion.getId());
        chipX += 84;
        if (chipX > 1080)
            break;
    }

    if (UI::boton(Rectangle{265, 620, 230, 44}, "Registrar equipaje", UI::orange()))
    {
        int idAvionObjeto = ConexionDB::convertirEntero(avionObjeto);
        int idObjeto = objetoPerdidoDAO.registrar("Equipaje", objetoEncontrado, idAvionObjeto, numeroAsiento, codigoEquipaje, fechaHallazgo, ubicacionHallazgo, empleadoHallazgo);
        if (idObjeto != 0)
        {
            std::stringstream aviso;
            aviso << "Equipaje perdido #" << idObjeto << ": " << objetoEncontrado
                  << " encontrado en avion " << idAvionObjeto << " (" << ubicacionHallazgo << ", asiento " << numeroAsiento
                  << ", codigo: " << codigoEquipaje << "). Verificar y gestionar recuperacion.";
            bool notificado = notificacionDAO.crear(3, "Objetos perdidos", aviso.str());
            mensaje = notificado ? "Equipaje registrado y notificado a Equipaje."
                                 : "Equipaje registrado. No se pudo notificar a Equipaje.";
            objetoEncontrado.clear();
            ubicacionHallazgo.clear();
            numeroAsiento.clear();
            codigoEquipaje.clear();
        }
        else
        {
            mensaje = "No se pudo registrar. Revise objeto, avion, fecha, ubicacion y empleado.";
        }
    }

    UI::aviso(Rectangle{240, 635, 780, 78}, "Flujo de recuperacion",
              "Equipaje verifica el objeto. Si identifica al propietario, avisa a Atencion al Pasajero para coordinar devolucion.",
              UI::orange());
    DrawText(mensaje.c_str(), 265, 580, 18, mensaje.find("registrado") != std::string::npos ? UI::green() : RED);
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
            dibujarRegistroObjetosPerdidos();
        else if (vista == 5)
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
