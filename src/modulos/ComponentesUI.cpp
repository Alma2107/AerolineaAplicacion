#include "ComponentesUI.h"
#include <algorithm>
#include <map>

namespace UI
{
static bool reemplazarAlEscribir = false;
static std::map<std::string, Texture2D> texturas;

static char caracterDesdeTecla(int tecla, bool shift)
{
    if (tecla >= KEY_A && tecla <= KEY_Z)
        return (char)(shift ? 'A' + (tecla - KEY_A) : 'a' + (tecla - KEY_A));

    if (tecla >= KEY_ZERO && tecla <= KEY_NINE)
    {
        const char normales[] = "0123456789";
        const char conShift[] = ")!@#$%^&*(";
        return shift ? conShift[tecla - KEY_ZERO] : normales[tecla - KEY_ZERO];
    }

    if (tecla >= KEY_KP_0 && tecla <= KEY_KP_9)
        return (char)('0' + (tecla - KEY_KP_0));

    switch (tecla)
    {
    case KEY_SPACE:
        return ' ';
    case KEY_MINUS:
    case KEY_KP_SUBTRACT:
        return shift ? '_' : '-';
    case KEY_EQUAL:
    case KEY_KP_ADD:
        return shift ? '+' : '=';
    case KEY_PERIOD:
    case KEY_KP_DECIMAL:
        return shift ? ':' : '.';
    case KEY_COMMA:
        return shift ? '<' : ',';
    case KEY_SLASH:
    case KEY_KP_DIVIDE:
        return shift ? '?' : '/';
    case KEY_SEMICOLON:
        return shift ? ':' : ';';
    case KEY_APOSTROPHE:
        return shift ? '"' : '\'';
    case KEY_LEFT_BRACKET:
        return shift ? '{' : '[';
    case KEY_RIGHT_BRACKET:
        return shift ? '}' : ']';
    case KEY_BACKSLASH:
        return shift ? '|' : '\\';
    default:
        return '\0';
    }
}

Color navy() { return Color{8, 28, 52, 255}; }
Color panel() { return Color{245, 248, 252, 255}; }
Color border() { return Color{211, 221, 233, 255}; }
Color muted() { return Color{91, 105, 123, 255}; }
Color blue() { return Color{0, 91, 196, 255}; }
Color green() { return Color{0, 132, 73, 255}; }
Color purple() { return Color{106, 55, 160, 255}; }
Color orange() { return Color{235, 111, 0, 255}; }

bool boton(Rectangle rect, const std::string &texto, Color color)
{
    bool hover = CheckCollisionPointRec(GetMousePosition(), rect);
    DrawRectangleRounded(Rectangle{rect.x, rect.y + 3, rect.width, rect.height}, 0.08f, 8, Fade(color, 0.18f));
    DrawRectangleRounded(rect, 0.08f, 8, hover ? Fade(color, 0.90f) : color);
    int ancho = MeasureText(texto.c_str(), 18);
    DrawText(texto.c_str(), (int)(rect.x + rect.width / 2 - ancho / 2), (int)rect.y + 12, 18, WHITE);
    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

bool botonSecundario(Rectangle rect, const std::string &texto, Color color)
{
    bool hover = CheckCollisionPointRec(GetMousePosition(), rect);
    DrawRectangleRounded(rect, 0.08f, 8, hover ? Fade(color, 0.12f) : WHITE);
    DrawRectangleLinesEx(rect, 1, color);
    int ancho = MeasureText(texto.c_str(), 18);
    DrawText(texto.c_str(), (int)(rect.x + rect.width / 2 - ancho / 2), (int)rect.y + 12, 18, color);
    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

bool input(Rectangle rect, const std::string &label, const std::string &valor, bool foco, bool password)
{
    bool hover = CheckCollisionPointRec(GetMousePosition(), rect);
    bool click = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    if (click)
        reemplazarAlEscribir = true;

    DrawText(label.c_str(), (int)rect.x, (int)rect.y - 22, 17, muted());
    DrawRectangleRounded(rect, 0.05f, 8, WHITE);
    DrawRectangleLinesEx(rect, foco ? 2 : 1, foco ? blue() : border());
    std::string visible = password ? std::string(valor.size(), '*') : valor;
    textoRecortado(visible, (int)rect.x + 12, (int)rect.y + 12, 18, DARKGRAY, 36);
    return click;
}

void capturarTexto(std::string &valor, int maximo)
{
    bool control = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    if (control && IsKeyPressed(KEY_A))
    {
        valor.clear();
        reemplazarAlEscribir = false;
        return;
    }

    if (control && IsKeyPressed(KEY_V))
    {
        const char *clip = GetClipboardText();
        if (clip != nullptr)
        {
            if (reemplazarAlEscribir)
                valor.clear();
            for (int i = 0; clip[i] != '\0' && (int)valor.size() < maximo; ++i)
            {
                if (clip[i] >= 32 && clip[i] <= 125)
                    valor.push_back(clip[i]);
            }
            reemplazarAlEscribir = false;
        }
    }

    if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_DELETE)) && !valor.empty())
    {
        if (reemplazarAlEscribir || IsKeyPressed(KEY_DELETE))
            valor.clear();
        else
            valor.pop_back();
        reemplazarAlEscribir = false;
        return;
    }

    int tecla = GetCharPressed();
    bool recibioCaracter = tecla > 0;
    while (tecla > 0)
    {
        if (reemplazarAlEscribir)
        {
            valor.clear();
            reemplazarAlEscribir = false;
        }
        if (tecla >= 32 && tecla <= 125 && (int)valor.size() < maximo)
            valor.push_back((char)tecla);
        tecla = GetCharPressed();
    }

    if (!recibioCaracter && !control)
    {
        int teclas[] = {
            KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
            KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
            KEY_ZERO, KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT, KEY_NINE,
            KEY_KP_0, KEY_KP_1, KEY_KP_2, KEY_KP_3, KEY_KP_4, KEY_KP_5, KEY_KP_6, KEY_KP_7, KEY_KP_8, KEY_KP_9,
            KEY_SPACE, KEY_MINUS, KEY_EQUAL, KEY_PERIOD, KEY_COMMA, KEY_SLASH, KEY_SEMICOLON, KEY_APOSTROPHE,
            KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET, KEY_BACKSLASH, KEY_KP_DECIMAL, KEY_KP_SUBTRACT, KEY_KP_ADD, KEY_KP_DIVIDE};

        for (int teclaFisica : teclas)
        {
            if (!IsKeyPressed(teclaFisica))
                continue;

            if (reemplazarAlEscribir)
            {
                valor.clear();
                reemplazarAlEscribir = false;
            }

            char caracter = caracterDesdeTecla(teclaFisica, shift);
            if (caracter != '\0' && (int)valor.size() < maximo)
                valor.push_back(caracter);
            break;
        }
    }
}

void tarjeta(Rectangle rect, const std::string &titulo, const std::vector<std::string> &items, Color color)
{
    DrawRectangleRounded(Rectangle{rect.x, rect.y + 5, rect.width, rect.height}, 0.035f, 8, Color{224, 232, 242, 255});
    DrawRectangleRounded(rect, 0.03f, 8, WHITE);
    DrawRectangleLinesEx(rect, 1, border());
    DrawRectangleRounded(Rectangle{rect.x, rect.y, 8, rect.height}, 0.03f, 8, color);
    DrawCircleGradient(Vector2{rect.x + rect.width - 52.0f, rect.y + 48.0f}, 62, Fade(color, 0.16f), Fade(color, 0.0f));
    DrawText(titulo.c_str(), (int)rect.x + 24, (int)rect.y + 22, 22, navy());
    DrawRectangleRounded(Rectangle{rect.x + 24, rect.y + 58, 76, 4}, 0.6f, 6, color);

    int y = (int)rect.y + 92;
    for (const std::string &item : items)
    {
        DrawCircle((int)rect.x + 30, y + 8, 3, color);
        DrawText(item.c_str(), (int)rect.x + 44, y, 17, DARKGRAY);
        y += 25;
    }
}

void shellModulo(const std::string &titulo, const std::string &subtitulo, Color color, int notificaciones)
{
    ClearBackground(panel());
    DrawRectangleGradientH(0, 0, 1500, 64, navy(), Color{15, 46, 82, 255});
    DrawCircleGradient(Vector2{1140, 22}, 120, Fade(color, 0.22f), Fade(color, 0.0f));
    DrawText(titulo.c_str(), 235, 15, 23, WHITE);
    DrawText(subtitulo.c_str(), 235, 43, 15, Fade(WHITE, 0.76f));
    DrawText("Administrador", 1210, 22, 16, WHITE);
    DrawCircle(1175, 28, 12, Fade(WHITE, 0.16f));
    DrawText("!", 1171, 18, 20, notificaciones > 0 ? Color{255, 88, 88, 255} : Fade(WHITE, 0.7f));
    if (notificaciones > 0)
    {
        DrawCircle(1185, 18, 8, Color{255, 88, 88, 255});
        DrawText(std::to_string(notificaciones).c_str(), 1182, 12, 12, WHITE);
    }
}

void menuLateral(const std::string &tituloModulo, const std::vector<std::string> &opciones, int activo, Color color)
{
    DrawRectangleGradientV(0, 0, 210, 900, Color{5, 18, 35, 255}, Color{14, 33, 58, 255});
    DrawText(tituloModulo.c_str(), 24, 26, 18, WHITE);
    DrawRectangleRounded(Rectangle{24, 58, 72, 4}, 0.6f, 8, color);
    int y = 92;
    for (int i = 0; i < (int)opciones.size(); ++i)
    {
        Rectangle item = {18, (float)y, 174, 42};
        bool selected = i == activo;
        bool hover = CheckCollisionPointRec(GetMousePosition(), item);
        DrawRectangleRounded(item, 0.08f, 8, selected ? Fade(color, 0.28f) : hover ? Fade(WHITE, 0.08f) : Fade(WHITE, 0.0f));
        DrawText(opciones[i].c_str(), 36, y + 12, 16, selected ? WHITE : Fade(WHITE, 0.76f));
        y += 50;
    }
}

void aviso(Rectangle rect, const std::string &titulo, const std::string &texto, Color color)
{
    DrawRectangleRounded(rect, 0.06f, 8, Fade(color, 0.08f));
    DrawRectangleLinesEx(rect, 1, Fade(color, 0.5f));
    DrawText(titulo.c_str(), (int)rect.x + 12, (int)rect.y + 10, 17, color);
    textoRecortado(texto, (int)rect.x + 12, (int)rect.y + 34, 15, DARKGRAY, 62);
}

void textoRecortado(const std::string &texto, int x, int y, int tam, Color color, int maxChars)
{
    std::string visible = texto;
    if ((int)visible.size() > maxChars)
        visible = visible.substr(0, maxChars - 3) + "...";
    DrawText(visible.c_str(), x, y, tam, color);
}

void imagenCubierta(const std::string &ruta, Rectangle rect, Color tinte)
{
    if (!FileExists(ruta.c_str()))
        return;

    if (texturas.find(ruta) == texturas.end())
        texturas[ruta] = LoadTexture(ruta.c_str());

    Texture2D textura = texturas[ruta];
    if (textura.id == 0 || textura.width == 0 || textura.height == 0)
        return;

    float escala = std::max(rect.width / textura.width, rect.height / textura.height);
    float origenAncho = rect.width / escala;
    float origenAlto = rect.height / escala;
    Rectangle origen = {
        (textura.width - origenAncho) / 2.0f,
        (textura.height - origenAlto) / 2.0f,
        origenAncho,
        origenAlto};
    DrawTexturePro(textura, origen, rect, Vector2{0, 0}, 0.0f, tinte);
}
}
