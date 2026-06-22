#include "ModuloVuelos.h"
#include "ComponentesUI.h"
#include "raylib.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <map>
#include <sstream>
#include <vector>

struct PuntoClima
{
    const char *ciudad;
    double latitud;
    double longitud;
};

static std::string limpiarLinea(std::string texto)
{
    while (!texto.empty() && (texto.back() == '\n' || texto.back() == '\r'))
        texto.pop_back();
    return texto;
}

static std::string normalizarIata(std::string valor)
{
    std::string salida;
    for (char c : valor)
    {
        if (std::isalpha((unsigned char)c) && salida.size() < 3)
            salida.push_back((char)std::toupper((unsigned char)c));
    }
    return salida;
}

static std::string minusculas(std::string valor)
{
    std::transform(valor.begin(), valor.end(), valor.begin(), [](unsigned char c)
                   { return (char)std::tolower(c); });
    return valor;
}

static bool puntoPorIata(const std::string &iata, PuntoClima &punto)
{
    static const std::map<std::string, PuntoClima> puntos = {
        {"AEP", {"Buenos Aires / Aeroparque", -34.5589, -58.4164}},
        {"EZE", {"Buenos Aires / Ezeiza", -34.8222, -58.5358}},
        {"BRC", {"Bariloche", -41.1512, -71.1575}},
        {"COR", {"Cordoba", -31.3236, -64.2080}},
        {"MDZ", {"Mendoza", -32.8317, -68.7929}},
        {"GIG", {"Rio de Janeiro", -22.8100, -43.2506}},
        {"GRU", {"Sao Paulo", -23.4356, -46.4731}},
        {"MAD", {"Madrid", 40.4983, -3.5676}},
        {"MIA", {"Miami", 25.7932, -80.2906}},
        {"SCL", {"Santiago de Chile", -33.3928, -70.7858}}};

    auto it = puntos.find(iata);
    if (it == puntos.end())
        return false;
    punto = it->second;
    return true;
}

static std::vector<std::string> dividir(const std::string &texto, char separador)
{
    std::vector<std::string> partes;
    std::stringstream ss(texto);
    std::string parte;
    while (std::getline(ss, parte, separador))
        partes.push_back(parte);
    return partes;
}

static double numero(const std::string &valor)
{
    std::string normalizado = valor;
    std::replace(normalizado.begin(), normalizado.end(), ',', '.');
    try
    {
        return std::stod(normalizado);
    }
    catch (...)
    {
        return 0;
    }
}

static std::string formato(double valor, int decimales, const std::string &sufijo)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(decimales) << valor << sufijo;
    return ss.str();
}

static std::string condicionWmo(int codigo)
{
    if (codigo == 0)
        return "Cielo despejado";
    if (codigo == 1 || codigo == 2)
        return "Parcialmente nublado";
    if (codigo == 3)
        return "Nublado";
    if (codigo == 45 || codigo == 48)
        return "Niebla";
    if (codigo >= 51 && codigo <= 57)
        return "Llovizna";
    if (codigo >= 61 && codigo <= 67)
        return "Lluvia";
    if (codigo >= 71 && codigo <= 77)
        return "Nieve";
    if (codigo >= 80 && codigo <= 82)
        return "Chaparrones";
    if (codigo >= 95)
        return "Tormenta";
    return "Condicion variable";
}

static std::string leerProceso(const std::string &comando)
{
    std::string salida;
    FILE *pipe = _popen(comando.c_str(), "r");
    if (!pipe)
        return salida;

    char buffer[2048];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        salida += buffer;
    _pclose(pipe);
    return limpiarLinea(salida);
}

static DatosClima consultarClimaTiempoReal(const std::string &iataSolicitado)
{
    DatosClima datos;
    datos.consultado = true;
    datos.iata = normalizarIata(iataSolicitado);
    if (datos.iata.empty())
        datos.iata = "AEP";

    PuntoClima punto{};
    if (!puntoPorIata(datos.iata, punto))
    {
        datos.detalleError = "Ese IATA no tiene coordenadas configuradas para clima en la app.";
        return datos;
    }

    datos.ciudad = punto.ciudad;
    std::stringstream url;
    url << "https://api.open-meteo.com/v1/forecast?latitude=" << punto.latitud
        << "&longitude=" << punto.longitud
        << "&current=temperature_2m,relative_humidity_2m,apparent_temperature,precipitation,rain,weather_code,cloud_cover,pressure_msl,wind_speed_10m,wind_direction_10m,wind_gusts_10m"
        << "&hourly=temperature_2m,precipitation_probability,wind_speed_10m,cloud_cover,visibility"
        << "&forecast_days=1&timezone=auto";

    std::stringstream comando;
    comando << "powershell -NoProfile -ExecutionPolicy Bypass -Command \"try { "
            << "[System.Threading.Thread]::CurrentThread.CurrentCulture=[System.Globalization.CultureInfo]::InvariantCulture; "
            << "[Net.ServicePointManager]::SecurityProtocol=[Net.SecurityProtocolType]::Tls12; "
            << "$r=Invoke-RestMethod -Uri '" << url.str() << "' -TimeoutSec 8; "
            << "$c=$r.current; $h=$r.hourly; "
            << "$avgTemp=($h.temperature_2m | Measure-Object -Average).Average; "
            << "$maxProb=($h.precipitation_probability | Measure-Object -Maximum).Maximum; "
            << "$avgWind=($h.wind_speed_10m | Measure-Object -Average).Average; "
            << "$avgCloud=($h.cloud_cover | Measure-Object -Average).Average; "
            << "$minVis=($h.visibility | Measure-Object -Minimum).Minimum; "
            << "Write-Output ([string]::Join('|', @($c.time,$c.temperature_2m,$c.apparent_temperature,$c.relative_humidity_2m,$c.precipitation,$c.rain,$c.weather_code,$c.cloud_cover,$c.pressure_msl,$c.wind_speed_10m,$c.wind_direction_10m,$c.wind_gusts_10m,[math]::Round($avgTemp,1),[math]::Round($maxProb,0),[math]::Round($avgWind,1),[math]::Round($avgCloud,0),[math]::Round($minVis/1000,1)))) "
            << "} catch { Write-Output ('ERROR|' + $_.Exception.Message) }\" 2>NUL";

    std::string salida = leerProceso(comando.str());
    if (salida.rfind("ERROR|", 0) == 0 || salida.empty())
    {
        datos.detalleError = salida.empty() ? "No hubo respuesta del servicio de clima." : salida.substr(6);
        return datos;
    }

    std::vector<std::string> partes = dividir(salida, '|');
    if (partes.size() < 17)
    {
        datos.detalleError = "La respuesta del clima llego incompleta.";
        return datos;
    }

    datos.ok = true;
    datos.actualizado = partes[0];
    datos.temperatura = numero(partes[1]);
    datos.sensacion = numero(partes[2]);
    datos.humedad = numero(partes[3]);
    datos.precipitacion = numero(partes[4]);
    datos.lluvia = numero(partes[5]);
    int wmo = (int)numero(partes[6]);
    datos.nubosidad = numero(partes[7]);
    datos.presion = numero(partes[8]);
    datos.viento = numero(partes[9]);
    datos.direccionViento = numero(partes[10]);
    datos.rafagas = numero(partes[11]);
    datos.tempPromedioDia = numero(partes[12]);
    datos.probLluviaMax = numero(partes[13]);
    datos.vientoPromedioDia = numero(partes[14]);
    datos.nubosidadPromedioDia = numero(partes[15]);
    datos.visibilidadMinimaKm = numero(partes[16]);
    datos.condicion = condicionWmo(wmo);
    return datos;
}

static void dibujarHeroAereo(Rectangle rect, Color color)
{
    DrawRectangleGradientV((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, Fade(color, 0.95f), Color{6, 23, 44, 255});
    DrawCircleGradient((int)(rect.x + rect.width - 80), (int)(rect.y + 44), 112, Fade(WHITE, 0.28f), Fade(WHITE, 0.0f));
    DrawCircleGradient((int)(rect.x + 120), (int)(rect.y + rect.height - 10), 150, Fade(Color{23, 178, 148, 255}, 0.24f), Fade(color, 0.0f));

    Vector2 nose = {rect.x + rect.width - 70, rect.y + rect.height * 0.48f};
    Vector2 tail = {rect.x + 90, rect.y + rect.height * 0.58f};
    DrawLineEx(tail, nose, 8, Fade(WHITE, 0.88f));
    DrawTriangle(Vector2{rect.x + rect.width - 90, rect.y + rect.height * 0.42f}, nose, Vector2{rect.x + rect.width - 100, rect.y + rect.height * 0.56f}, WHITE);
    DrawTriangle(Vector2{rect.x + 305, rect.y + rect.height * 0.54f}, Vector2{rect.x + 430, rect.y + rect.height * 0.20f}, Vector2{rect.x + 455, rect.y + rect.height * 0.56f}, Fade(WHITE, 0.86f));
    DrawTriangle(Vector2{rect.x + 292, rect.y + rect.height * 0.59f}, Vector2{rect.x + 430, rect.y + rect.height * 0.78f}, Vector2{rect.x + 450, rect.y + rect.height * 0.58f}, Fade(WHITE, 0.72f));
    DrawTriangle(Vector2{rect.x + 100, rect.y + rect.height * 0.55f}, Vector2{rect.x + 168, rect.y + rect.height * 0.30f}, Vector2{rect.x + 188, rect.y + rect.height * 0.58f}, Fade(WHITE, 0.75f));
}

static void dibujarMetrica(Rectangle rect, const std::string &titulo, const std::string &valor, const std::string &pie, Color color)
{
    DrawRectangleRounded(rect, 0.05f, 8, WHITE);
    DrawRectangleLinesEx(rect, 1, UI::border());
    DrawCircle((int)rect.x + 18, (int)rect.y + 20, 5, color);
    DrawText(titulo.c_str(), (int)rect.x + 32, (int)rect.y + 12, 15, UI::muted());
    UI::textoRecortado(valor, (int)rect.x + 16, (int)rect.y + 42, 24, UI::navy(), 14);
    UI::textoRecortado(pie, (int)rect.x + 16, (int)rect.y + 74, 13, UI::muted(), 26);
}

static void dibujarBarra(Rectangle rect, double valor, double maximo, Color color)
{
    DrawRectangleRounded(rect, 0.2f, 8, Fade(color, 0.12f));
    float ancho = (float)(rect.width * std::min(1.0, std::max(0.0, valor / maximo)));
    DrawRectangleRounded(Rectangle{rect.x, rect.y, ancho, rect.height}, 0.2f, 8, color);
}

ModuloVuelos::ModuloVuelos()
    : mensaje("Modulo activo."), vista(0), foco(0), numero("JA2001"), origen("AEP"), destino("MAD"),
      fechaSalida("2026-09-01 08:30:00"), fechaLlegada("2026-09-01 23:45:00"), precio("95000"),
      idAvion("1"), estado("Programado"), accionIdVuelo("1"), accionIdAvion("2"),
      accionFechaSalida("2026-09-02 10:00:00"), accionFechaLlegada("2026-09-02 12:15:00"),
      accionMotivo("clima"), climaIata("AEP"), filtroHistorial("") {}

void ModuloVuelos::dibujarNavegacion()
{
    std::vector<std::string> opciones = {"Crear vuelo", "Asignar aeronave", "Clima", "Cancelar vuelo", "Reprogramar", "Historial"};
    UI::menuLateral("Operaciones de Vuelo", opciones, vista, UI::blue());

    for (int i = 0; i < (int)opciones.size(); ++i)
    {
        Rectangle item = {18, (float)(92 + i * 50), 174, 42};
        if (CheckCollisionPointRec(GetMousePosition(), item) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            vista = i;
    }

    if (UI::botonSecundario(Rectangle{24, 748, 160, 42}, "Volver al inicio", UI::blue()))
    {
        vista = -1;
        return;
    }
}

void ModuloVuelos::dibujarCrearVuelo()
{
    DrawText("Crear vuelo", 240, 95, 30, UI::navy());
    DrawText("Campos requeridos por la tabla vuelos", 240, 130, 17, UI::muted());

    if (UI::input(Rectangle{240, 185, 230, 46}, "Numero de vuelo", numero, foco == 1))
        foco = 1;
    if (UI::input(Rectangle{500, 185, 190, 46}, "Origen IATA", origen, foco == 2))
        foco = 2;
    if (UI::input(Rectangle{720, 185, 190, 46}, "Destino IATA", destino, foco == 3))
        foco = 3;

    if (UI::input(Rectangle{240, 275, 310, 46}, "Fecha salida YYYY-MM-DD HH:MM:SS", fechaSalida, foco == 4))
        foco = 4;
    if (UI::input(Rectangle{580, 275, 310, 46}, "Fecha llegada YYYY-MM-DD HH:MM:SS", fechaLlegada, foco == 5))
        foco = 5;

    if (UI::input(Rectangle{240, 365, 190, 46}, "ID aeronave", idAvion, foco == 6))
        foco = 6;
    if (UI::input(Rectangle{460, 365, 190, 46}, "Precio base", precio, foco == 7))
        foco = 7;
    if (UI::input(Rectangle{680, 365, 210, 46}, "Estado", estado, foco == 8))
        foco = 8;

    if (foco == 1)
        UI::capturarTexto(numero);
    if (foco == 2)
        UI::capturarTexto(origen, 3);
    if (foco == 3)
        UI::capturarTexto(destino, 3);
    if (foco == 4)
        UI::capturarTexto(fechaSalida);
    if (foco == 5)
        UI::capturarTexto(fechaLlegada);
    if (foco == 6)
        UI::capturarTexto(idAvion, 5);
    if (foco == 7)
        UI::capturarTexto(precio, 12);
    if (foco == 8)
        UI::capturarTexto(estado, 30);

    std::string origenNormalizado = normalizarIata(origen);
    std::string destinoNormalizado = normalizarIata(destino);
    Vuelo vuelo(0, numero, origenNormalizado, destinoNormalizado, fechaSalida, fechaLlegada, ConexionDB::convertirDouble(precio), estado, ConexionDB::convertirEntero(idAvion));

    DrawText("Aeronaves disponibles", 240, 650, 20, UI::navy());
    int yDisponible = 682;
    for (const Avion &avion : avionDAO.listarDisponibles())
    {
        Rectangle item = {240, (float)yDisponible, 300, 30};
        bool elegido = std::to_string(avion.getId()) == idAvion;
        DrawRectangleRounded(item, 0.05f, 8, elegido ? Fade(UI::blue(), 0.14f) : WHITE);
        DrawRectangleLinesEx(item, 1, elegido ? UI::blue() : UI::border());
        std::stringstream ss;
        ss << "#" << avion.getId() << " " << avion.getModelo() << " (" << avion.getCapacidad() << ")";
        UI::textoRecortado(ss.str(), 252, yDisponible + 7, 14, elegido ? UI::blue() : DARKGRAY, 38);
        if (CheckCollisionPointRec(GetMousePosition(), item) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            idAvion = std::to_string(avion.getId());
        yDisponible += 36;
        if (yDisponible > 790)
            break;
    }

    DrawText("Bloqueadas por mantenimiento", 580, 650, 20, UI::navy());
    int yBloqueada = 682;
    for (const Avion &avion : avionDAO.listar())
    {
        if (avion.getEstado() == "Activo")
            continue;
        std::stringstream ss;
        ss << "#" << avion.getId() << " " << avion.getModelo() << " - " << avion.getEstado();
        UI::textoRecortado(ss.str(), 580, yBloqueada, 14, RED, 44);
        yBloqueada += 28;
        if (yBloqueada > 790)
            break;
    }

    if (UI::botonSecundario(Rectangle{240, 455, 170, 42}, "Validar datos", UI::blue()))
    {
        origen = origenNormalizado;
        destino = destinoNormalizado;
        std::string resultado = vueloDAO.validar(vuelo);
        mensaje = resultado == "OK" ? "Validacion API BD: vuelo apto para guardar." : "Validacion API BD: " + resultado;
    }
    if (UI::boton(Rectangle{430, 455, 170, 42}, "Guardar vuelo", UI::blue()))
    {
        origen = origenNormalizado;
        destino = destinoNormalizado;
        std::string resultado = vueloDAO.validar(vuelo);
        if (resultado == "OK")
        {
            Vuelo guardado = vueloDAO.crear(vuelo);
            mensaje = guardado.getId() != 0 ? "Vuelo guardado en BD: " + guardado.getNumero() : "No se pudo guardar el vuelo.";
        }
        else
        {
            mensaje = "Validacion API BD: " + resultado;
        }
    }

    DrawRectangleRounded(Rectangle{240, 535, 650, 82}, 0.04f, 8, WHITE);
    DrawRectangleLinesEx(Rectangle{240, 535, 650, 82}, 1, UI::border());
    DrawText("Resultado de validacion", 260, 555, 19, UI::blue());
    UI::textoRecortado(mensaje, 260, 585, 18, mensaje.find("apto") != std::string::npos || mensaje.find("guardado") != std::string::npos ? UI::green() : RED, 78);
}

void ModuloVuelos::dibujarAcciones()
{
    const char *titulo = vista == 1 ? "Asignar aeronave" : vista == 2 ? "Verificar clima" : vista == 3 ? "Cancelar vuelo" : "Reprogramar vuelo";
    DrawText(titulo, 240, 95, 30, UI::navy());

    if (vista == 2)
    {
        DrawText("Centro meteorologico operativo", 240, 95, 30, UI::navy());
        DrawText("Datos actuales del mismo dia para aeropuertos de la base", 240, 130, 17, UI::muted());

        UI::imagenCubierta("assets/weather_live.png", Rectangle{240, 160, 740, 124}, WHITE);
        DrawRectangleGradientH(240, 160, 420, 124, Fade(UI::navy(), 0.82f), Fade(UI::navy(), 0.0f));
        DrawText("EN VIVO", 270, 184, 18, Color{23, 178, 148, 255});
        DrawText("Clima de ruta y riesgo operacional", 270, 214, 28, WHITE);
        DrawText("Open-Meteo + aeropuertos IATA configurados en la aplicacion", 270, 252, 16, Fade(WHITE, 0.78f));

        if (UI::input(Rectangle{240, 340, 150, 46}, "IATA", climaIata, foco == 30))
            foco = 30;
        if (foco == 30)
            UI::capturarTexto(climaIata, 3);

        if (UI::boton(Rectangle{420, 340, 210, 46}, "Actualizar ahora", UI::blue()))
        {
            climaIata = normalizarIata(climaIata);
            clima = consultarClimaTiempoReal(climaIata);
            mensaje = clima.ok ? "Clima actualizado en tiempo real para " + clima.iata + "." : "No se pudo consultar clima real: " + clima.detalleError;
        }

        std::vector<std::string> codigos = vueloDAO.listarAeropuertos();
        int chipX = 660;
        int chipY = 346;
        for (const std::string &codigo : codigos)
        {
            Rectangle chip = {(float)chipX, (float)chipY, 48, 30};
            bool activo = normalizarIata(climaIata) == codigo;
            DrawRectangleRounded(chip, 0.28f, 8, activo ? Fade(UI::blue(), 0.16f) : WHITE);
            DrawRectangleLinesEx(chip, 1, activo ? UI::blue() : UI::border());
            DrawText(codigo.c_str(), chipX + 10, chipY + 8, 14, activo ? UI::blue() : UI::muted());
            if (CheckCollisionPointRec(GetMousePosition(), chip) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                climaIata = codigo;
            chipX += 56;
            if (chipX > 930)
            {
                chipX = 660;
                chipY += 36;
            }
            if (chipY > 420)
                break;
        }

        Color estadoColor = UI::green();
        std::string estadoVuelo = "OPERACION NORMAL";
        if (clima.ok && (clima.rafagas >= 55 || clima.probLluviaMax >= 75 || clima.visibilidadMinimaKm < 5))
        {
            estadoColor = RED;
            estadoVuelo = "ALERTA OPERATIVA";
        }
        else if (clima.ok && (clima.viento >= 35 || clima.probLluviaMax >= 45 || clima.nubosidad >= 80))
        {
            estadoColor = UI::orange();
            estadoVuelo = "SEGUIMIENTO";
        }

        DrawRectangleRounded(Rectangle{240, 420, 280, 165}, 0.04f, 8, WHITE);
        DrawRectangleLinesEx(Rectangle{240, 420, 280, 165}, 1, UI::border());
        DrawText((clima.ok ? clima.iata.c_str() : normalizarIata(climaIata).c_str()), 265, 440, 24, UI::blue());
        UI::textoRecortado(clima.ok ? clima.ciudad : "Seleccione y actualice", 265, 472, 17, UI::muted(), 28);
        DrawText(clima.ok ? formato(clima.temperatura, 1, " C").c_str() : "--.- C", 265, 506, 38, UI::navy());
        UI::textoRecortado(clima.ok ? clima.condicion : "Sin consulta activa", 265, 550, 17, estadoColor, 26);

        DrawRectangleRounded(Rectangle{540, 420, 440, 165}, 0.04f, 8, WHITE);
        DrawRectangleLinesEx(Rectangle{540, 420, 440, 165}, 1, UI::border());
        DrawText(estadoVuelo.c_str(), 565, 440, 22, estadoColor);
        UI::textoRecortado(clima.ok ? "Actualizado: " + clima.actualizado : "Presione actualizar para traer clima real", 565, 472, 15, UI::muted(), 46);
        DrawText("Riesgo por viento", 565, 510, 14, UI::muted());
        dibujarBarra(Rectangle{700, 511, 240, 11}, clima.ok ? clima.rafagas : 0, 80, estadoColor);
        DrawText("Lluvia maxima hoy", 565, 538, 14, UI::muted());
        dibujarBarra(Rectangle{700, 539, 240, 11}, clima.ok ? clima.probLluviaMax : 0, 100, UI::blue());
        DrawText("Nubosidad", 565, 566, 14, UI::muted());
        dibujarBarra(Rectangle{700, 567, 240, 11}, clima.ok ? clima.nubosidad : 0, 100, Color{23, 178, 148, 255});

        dibujarMetrica(Rectangle{240, 612, 140, 84}, "Sensacion", clima.ok ? formato(clima.sensacion, 1, " C") : "--", "termica", UI::blue());
        dibujarMetrica(Rectangle{390, 612, 140, 84}, "Humedad", clima.ok ? formato(clima.humedad, 0, "%") : "--", "relativa", Color{23, 178, 148, 255});
        dibujarMetrica(Rectangle{540, 612, 140, 84}, "Viento", clima.ok ? formato(clima.viento, 1, " km/h") : "--", "dir " + formato(clima.direccionViento, 0, " deg"), UI::orange());
        dibujarMetrica(Rectangle{690, 612, 140, 84}, "Rafagas", clima.ok ? formato(clima.rafagas, 1, " km/h") : "--", "pico", estadoColor);
        dibujarMetrica(Rectangle{840, 612, 140, 84}, "Presion", clima.ok ? formato(clima.presion, 0, " hPa") : "--", "mar", UI::purple());

        dibujarMetrica(Rectangle{240, 710, 140, 82}, "Lluvia", clima.ok ? formato(clima.lluvia + clima.precipitacion, 1, " mm") : "--", "actual", UI::blue());
        dibujarMetrica(Rectangle{390, 710, 140, 82}, "Prob. lluvia", clima.ok ? formato(clima.probLluviaMax, 0, "%") : "--", "max hoy", UI::orange());
        dibujarMetrica(Rectangle{540, 710, 140, 82}, "Temp. media", clima.ok ? formato(clima.tempPromedioDia, 1, " C") : "--", "dia", UI::green());
        dibujarMetrica(Rectangle{690, 710, 140, 82}, "Nubes media", clima.ok ? formato(clima.nubosidadPromedioDia, 0, "%") : "--", "dia", Color{23, 178, 148, 255});
        dibujarMetrica(Rectangle{840, 710, 140, 82}, "Visibilidad", clima.ok ? formato(clima.visibilidadMinimaKm, 1, " km") : "--", "min hoy", UI::purple());

        if (clima.consultado && !clima.ok)
            UI::textoRecortado("Clima no disponible: " + clima.detalleError, 240, 800, 16, RED, 90);
        return;
    }

    if (UI::input(Rectangle{240, 180, 200, 46}, "ID vuelo", accionIdVuelo, foco == 20))
        foco = 20;
    if (foco == 20)
        UI::capturarTexto(accionIdVuelo, 8);

    if (vista == 1)
    {
        if (UI::input(Rectangle{470, 180, 200, 46}, "Nuevo ID avion", accionIdAvion, foco == 21))
            foco = 21;
        if (foco == 21)
            UI::capturarTexto(accionIdAvion, 8);
        if (UI::boton(Rectangle{240, 265, 210, 42}, "Asignar avion", UI::blue()))
        {
            bool ok = vueloDAO.asignarAvion(ConexionDB::convertirEntero(accionIdVuelo), ConexionDB::convertirEntero(accionIdAvion));
            mensaje = ok ? "Avion asignado y vuelo marcado como reprogramado." : "No se pudo asignar: avion inexistente o no activo.";
            if (ok)
            {
                bool aviso = notificacionDAO.crear(2, "Reprogramacion", "El vuelo ID " + accionIdVuelo + " cambio a la aeronave " + accionIdAvion + ".");
                if (!aviso)
                    mensaje += " No se pudo guardar aviso en BD: revise tabla notificaciones.";
            }
        }
    }
    else if (vista == 3)
    {
        if (UI::input(Rectangle{470, 180, 300, 46}, "Motivo", accionMotivo, foco == 22))
            foco = 22;
        if (foco == 22)
            UI::capturarTexto(accionMotivo, 45);
        if (UI::boton(Rectangle{240, 265, 210, 42}, "Cancelar vuelo", UI::blue()))
        {
            bool ok = vueloDAO.cancelar(ConexionDB::convertirEntero(accionIdVuelo), accionMotivo);
            mensaje = ok ? "Vuelo cancelado en BD y aviso enviado a pasajeros." : "No se pudo cancelar el vuelo.";
            if (ok)
            {
                bool aviso = notificacionDAO.crear(2, "Cancelacion", "El vuelo ID " + accionIdVuelo + " fue cancelado por " + accionMotivo + ". Informar pasajeros afectados.");
                if (!aviso)
                    mensaje = "Vuelo cancelado en BD. No se pudo guardar aviso en BD: revise tabla notificaciones.";
            }
        }
    }
    else if (vista == 4)
    {
        if (UI::input(Rectangle{470, 180, 300, 46}, "Nueva salida", accionFechaSalida, foco == 23))
            foco = 23;
        if (UI::input(Rectangle{800, 180, 300, 46}, "Nueva llegada", accionFechaLlegada, foco == 24))
            foco = 24;
        if (foco == 23)
            UI::capturarTexto(accionFechaSalida);
        if (foco == 24)
            UI::capturarTexto(accionFechaLlegada);
        if (UI::boton(Rectangle{240, 265, 210, 42}, "Reprogramar", UI::blue()))
        {
            bool ok = vueloDAO.reprogramar(ConexionDB::convertirEntero(accionIdVuelo), accionFechaSalida, accionFechaLlegada);
            mensaje = ok ? "Vuelo reprogramado en BD y aviso enviado a pasajeros." : "No se pudo reprogramar el vuelo.";
            if (ok)
            {
                bool aviso = notificacionDAO.crear(2, "Reprogramacion", "El vuelo ID " + accionIdVuelo + " cambio de fecha u horario.");
                if (!aviso)
                    mensaje = "Vuelo reprogramado en BD. No se pudo guardar aviso en BD: revise tabla notificaciones.";
            }
        }
    }

    DrawText(mensaje.c_str(), 240, 350, 19, UI::green());
}

void ModuloVuelos::dibujarNotificaciones()
{
    auto notificaciones = notificacionDAO.listarPorModulo(1);
    DrawText("Avisos de mantenimiento", 1010, 95, 22, UI::navy());
    int y = 132;
    for (const Notificacion &notificacion : notificaciones)
    {
        UI::aviso(Rectangle{1010, (float)y, 330, 78}, notificacion.getTipo(), notificacion.getMensaje(), UI::orange());
        y += 90;
    }

    int yAvion = 570;
    DrawText("Aeronaves no disponibles", 1010, yAvion, 20, UI::navy());
    yAvion += 34;
    for (const Avion &avion : avionDAO.listar())
    {
        if (avion.getEstado() != "Activo")
        {
            std::stringstream ss;
            ss << "Avion " << avion.getId() << " - " << avion.getEstado();
            DrawText(ss.str().c_str(), 1010, yAvion, 17, RED);
            yAvion += 26;
        }
    }

    if (UI::botonSecundario(Rectangle{1010, 740, 160, 38}, "Marcar leidas", UI::orange()))
        notificacionDAO.marcarLeidas(1);
}

void ModuloVuelos::dibujarListado()
{
    DrawText("Vuelos registrados", 240, 95, 30, UI::navy());
    int y = 145;
    DrawText("ID   Nro       Ruta       Salida               Avion   Estado", 240, y, 17, UI::muted());
    y += 30;
    for (const Vuelo &vuelo : vueloDAO.listar())
    {
        std::stringstream ss;
        ss << vuelo.getId() << "   " << vuelo.getNumero() << "   " << vuelo.getOrigen() << "-" << vuelo.getDestino()
           << "   " << vuelo.getFechaSalida() << "   " << vuelo.getIdAvion() << "   " << vuelo.getEstado();
        UI::textoRecortado(ss.str(), 240, y, 17, DARKGRAY, 94);
        y += 30;
    }
}

void ModuloVuelos::mostrar()
{
    while (!WindowShouldClose())
    {
        BeginDrawing();
        UI::shellModulo("Modulo 1 / Operaciones de Vuelo", "Vuelos > Crear, asignar, cancelar y reprogramar", UI::blue(), notificacionDAO.contarNoLeidas(1));
        dibujarNavegacion();
        if (vista < 0)
        {
            EndDrawing();
            return;
        }

        if (vista == 0)
            dibujarCrearVuelo();
        else if (vista >= 1 && vista <= 4)
            dibujarAcciones();
        else
            dibujarListado();

        dibujarNotificaciones();
        EndDrawing();
    }
}
