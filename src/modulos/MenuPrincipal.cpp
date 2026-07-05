#include "MenuPrincipal.h"
#include "ComponentesUI.h"
#include "ModuloEquipaje.h"
#include "ModuloMantenimiento.h"
#include "ModuloPasajeros.h"
#include "ModuloVuelos.h"
#include "../dao/ConexionDB.h"
#include "../dao/EmpleadoDAO.h"
#include "raylib.h"

MenuPrincipal::MenuPrincipal()
    : moduloSeleccionado(0), usuario(""), password(""), mensaje(""), escribiendoUsuario(true), dbLista(false), moduloParaAbrir(0)
{
    ConexionDB db;
    dbLista = db.inicializar();
    if (dbLista)
        dbLista = !db.consultar("SELECT COUNT(*) FROM vuelos").empty();
    if (!dbLista)
        mensaje = "No se pudo conectar a XAMPP/base aerolinea. Revise conexion_aerogest.log.";
}

void MenuPrincipal::capturarTexto(std::string &texto, bool ocultar)
{
    (void)ocultar;
    UI::capturarTexto(texto, 35);
}

std::string MenuPrincipal::nombreModulo(int modulo) const
{
    if (modulo == 1)
        return "Operaciones de Vuelo";
    if (modulo == 2)
        return "Atencion al Pasajero";
    if (modulo == 3)
        return "Equipaje";
    if (modulo == 4)
        return "Flota y Mantenimiento";
    return "";
}

void MenuPrincipal::seleccionarModulo(int modulo)
{
    moduloSeleccionado = modulo;
    usuario.clear();
    password.clear();
    mensaje.clear();
    escribiendoUsuario = true;
}

void MenuPrincipal::dibujarMenu()
{
    int ancho = GetScreenWidth();
    int alto = GetScreenHeight();
    float contentX = 220.0f;
    float contentW = (float)ancho - contentX - 24.0f;
    float cardW = (contentW - 32.0f) / 2.0f;
    float cardH = 120.0f;
    float cardGapH = 32.0f;
    float cardGapV = 28.0f;
    float cardYTop = 228.0f;
    float cardYBottom = cardYTop + cardH + cardGapV;
    float cardXLeft = contentX;
    float cardXRight = contentX + cardW + cardGapH;

    ClearBackground(Color{244, 247, 251, 255});
    DrawRectangle(0, 0, 210, alto, UI::navy());
    DrawText("AeroGest", 28, 24, 27, WHITE);
    DrawText("Sistema de Gestion de Vuelos", 28, 62, 14, Fade(WHITE, 0.75f));
    DrawText("MODULOS", 28, 108, 15, Fade(WHITE, 0.8f));

    Rectangle side1 = {18, 132, 174, 72};
    Rectangle side2 = {18, 218, 174, 72};
    Rectangle side3 = {18, 304, 174, 72};
    Rectangle side4 = {18, 390, 174, 72};
    DrawRectangleRounded(side1, 0.08f, 8, UI::blue());
    DrawRectangleRounded(side2, 0.08f, 8, UI::green());
    DrawRectangleRounded(side3, 0.08f, 8, UI::purple());
    DrawRectangleRounded(side4, 0.08f, 8, UI::orange());
    DrawText("1  Operaciones", 42, 148, 16, WHITE);
    DrawText("de Vuelo", 65, 168, 16, WHITE);
    DrawText("2  Atencion al", 42, 234, 16, WHITE);
    DrawText("Pasajero", 65, 254, 16, WHITE);
    DrawText("3  Equipaje", 42, 320, 16, WHITE);
    DrawText("4  Flota y", 42, 406, 16, WHITE);
    DrawText("Mantenimiento", 65, 426, 16, WHITE);
    DrawText("Cerrar sesion", 28, alto - 70, 16, Fade(WHITE, 0.82f));

    DrawText("Inicio", contentX, 28, 24, UI::navy());
    DrawText("Centro de control operacional conectado a XAMPP", contentX, 60, 15, UI::muted());
    DrawText("Administrador", ancho - 190, 40, 16, UI::navy());
    DrawCircle(ancho - 220, 44, 10, Fade(UI::blue(), 0.12f));
    DrawText("!", ancho - 223, 33, 20, RED);

    UI::imagenCubierta("src/assets/aero_hero.png", Rectangle{contentX, 92, contentW, 120}, WHITE);
    DrawRectangleGradientH(contentX, 92, contentW * 0.6f, 120, Fade(UI::navy(), 0.88f), Fade(UI::navy(), 0.05f));
    DrawText("AeroGest Control Center", contentX + 24, 112, 24, WHITE);
    DrawText("Gestion de vuelos, pasajeros, equipaje y flota con datos en MariaDB", contentX + 24, 144, 14, Fade(WHITE, 0.86f));
    DrawRectangleRounded(Rectangle{contentX + 24, 168, 170, 28}, 0.22f, 8, dbLista ? Fade(UI::green(), 0.92f) : Fade(RED, 0.92f));
    DrawText(dbLista ? "BD conectada" : "BD sin conexion", contentX + 48, 174, 14, WHITE);

    UI::tarjeta(Rectangle{cardXLeft, cardYTop, cardW, cardH}, "Operaciones de Vuelo",
                {"Crear y gestionar vuelos", "Asignar aeronaves", "Verificar clima", "Cancelar o reprogramar vuelos"}, UI::blue());
    UI::tarjeta(Rectangle{cardXRight, cardYTop, cardW, cardH}, "Atencion al Pasajero",
                {"Check-in por codigo de reserva", "Reservas presenciales", "Cancelar reserva y reembolso", "Notificaciones a pasajeros"}, UI::green());
    UI::tarjeta(Rectangle{cardXLeft, cardYBottom, cardW, cardH}, "Equipaje",
                {"Registrar equipaje", "Rastrear equipaje", "Equipaje perdido o danado", "Reclamos"}, UI::purple());
    UI::tarjeta(Rectangle{cardXRight, cardYBottom, cardW, cardH}, "Flota y Mantenimiento",
                {"Estado de flota", "PIMA y mantenimiento", "Reparaciones", "Avisos a operaciones"}, UI::orange());

    float buttonYTop = cardYTop + cardH + 10.0f;
    float buttonYBottom = cardYBottom + cardH + 10.0f;
    float buttonW = std::min(160.0f, cardW - 40.0f);
    if (UI::botonSecundario(Rectangle{cardXLeft + 20, buttonYTop, buttonW, 30}, "Ir al modulo", UI::blue()))
        seleccionarModulo(1);
    if (UI::botonSecundario(Rectangle{cardXRight + 20, buttonYTop, buttonW, 30}, "Ir al modulo", UI::green()))
        seleccionarModulo(2);
    if (UI::botonSecundario(Rectangle{cardXLeft + 20, buttonYBottom, buttonW, 30}, "Ir al modulo", UI::purple()))
        seleccionarModulo(3);
    if (UI::botonSecundario(Rectangle{cardXRight + 20, buttonYBottom, buttonW, 30}, "Ir al modulo", UI::orange()))
        seleccionarModulo(4);

    UI::imagenCubierta("src/assets/runway_panel.png", Rectangle{cardXRight, cardYTop, cardW, cardH}, WHITE);
    DrawRectangleGradientV(cardXRight, cardYTop, cardW, cardH, Fade(WHITE, 0.0f), Fade(UI::navy(), 0.78f));
    DrawText("Operacion", cardXRight + 24, cardYTop + 30, 20, WHITE);
    DrawText("lista para despacho", cardXRight + 24, cardYTop + 55, 14, Fade(WHITE, 0.86f));
    UI::aviso(Rectangle{cardXRight, cardYBottom, cardW, 75}, dbLista ? "Base de datos conectada" : "Base de datos sin conexion",
              dbLista ? "Los modulos guardan, actualizan y eliminan datos en MariaDB." : mensaje, dbLista ? UI::green() : RED);
    DrawText("Usuarios: vuelos/vuelos123 | pasajeros/pasajeros123 | equipaje/equipaje123 | mantenimiento/mantenimiento123",
             contentX, alto - 30, 13, UI::muted());
}

void MenuPrincipal::dibujarLogin()
{
    Color color = moduloSeleccionado == 1 ? UI::blue() : moduloSeleccionado == 2 ? UI::green() : moduloSeleccionado == 3 ? UI::purple()
                                                                                                               : UI::orange();
    ClearBackground(Color{244, 247, 251, 255});
    DrawRectangle(0, 0, 210, 900, UI::navy());
    DrawText("AeroGest", 28, 24, 27, WHITE);
    DrawText("Acceso por empleado", 28, 62, 14, Fade(WHITE, 0.75f));

    DrawRectangleRounded(Rectangle{360, 125, 520, 430}, 0.04f, 8, WHITE);
    DrawRectangleLinesEx(Rectangle{360, 125, 520, 430}, 1, UI::border());
    DrawText(nombreModulo(moduloSeleccionado).c_str(), 405, 170, 30, color);
    DrawText("Verificacion de seguridad", 405, 212, 19, UI::muted());

    Rectangle usuarioRect = {450, 252, 350, 48};
    Rectangle passwordRect = {450, 322, 350, 48};

    DrawText("Usuario", 405, 265, 18, UI::muted());
    DrawRectangleRounded(usuarioRect, 0.05f, 8, WHITE);
    DrawRectangleLinesEx(usuarioRect, escribiendoUsuario ? 2 : 1, escribiendoUsuario ? color : UI::border());
    DrawText("Password", 405, 335, 18, UI::muted());
    DrawRectangleRounded(passwordRect, 0.05f, 8, WHITE);
    DrawRectangleLinesEx(passwordRect, !escribiendoUsuario ? 2 : 1, !escribiendoUsuario ? color : UI::border());

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Vector2 mouse = GetMousePosition();
        escribiendoUsuario = CheckCollisionPointRec(mouse, usuarioRect);
        if (CheckCollisionPointRec(mouse, passwordRect))
            escribiendoUsuario = false;
    }

    if (IsKeyPressed(KEY_TAB))
        escribiendoUsuario = !escribiendoUsuario;

    if (escribiendoUsuario)
        capturarTexto(usuario, false);
    else
        capturarTexto(password, true);

    UI::textoRecortado(usuario, (int)usuarioRect.x + 12, (int)usuarioRect.y + 14, 20, DARKGRAY, 28);
    UI::textoRecortado(std::string(password.size(), '*'), (int)passwordRect.x + 12, (int)passwordRect.y + 14, 20, DARKGRAY, 28);

    if (UI::boton(Rectangle{450, 410, 160, 48}, "Ingresar", color) || IsKeyPressed(KEY_ENTER))
    {
        EmpleadoDAO empleadoDAO;
        if (empleadoDAO.validarAcceso(moduloSeleccionado, usuario, password))
            moduloParaAbrir = moduloSeleccionado;
        else
            mensaje = "Acceso denegado. Revise usuario, password y modulo.";
    }

    if (UI::botonSecundario(Rectangle{640, 410, 160, 48}, "Volver", color))
        moduloSeleccionado = 0;

    DrawText(mensaje.c_str(), 405, 500, 18, RED);
}

void MenuPrincipal::abrirModulo(int modulo)
{
    if (modulo == 1)
    {
        ModuloVuelos pantalla;
        pantalla.mostrar();
    }
    else if (modulo == 2)
    {
        ModuloPasajeros pantalla;
        pantalla.mostrar();
    }
    else if (modulo == 3)
    {
        ModuloEquipaje pantalla;
        pantalla.mostrar();
    }
    else if (modulo == 4)
    {
        ModuloMantenimiento pantalla;
        pantalla.mostrar();
    }

    moduloSeleccionado = 0;
    usuario.clear();
    password.clear();
}

void MenuPrincipal::ejecutar()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1320, 760, "AeroGest - Sistema de Gestion de Aerolinea");
    SetWindowPosition(100, 100);
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (moduloSeleccionado == 0)
            dibujarMenu();
        else
            dibujarLogin();

        EndDrawing();

        if (moduloParaAbrir != 0)
        {
            abrirModulo(moduloParaAbrir);
            moduloParaAbrir = 0;
        }
    }

    CloseWindow();
}
