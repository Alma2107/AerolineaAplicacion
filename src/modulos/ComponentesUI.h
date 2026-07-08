#ifndef COMPONENTES_UI_H
#define COMPONENTES_UI_H

#include "raylib.h"
#include <string>
#include <vector>

namespace UI
{
    Color navy();
    Color panel();
    Color border();
    Color muted();
    Color blue();
    Color green();
    Color purple();
    Color orange();

    bool boton(Rectangle rect, const std::string &texto, Color color);
    bool botonSecundario(Rectangle rect, const std::string &texto, Color color);
    bool input(Rectangle rect, const std::string &label, const std::string &valor, bool foco, bool password = false);
    void capturarTexto(std::string &valor, int maximo = 80);
    void tarjeta(Rectangle rect, const std::string &titulo, const std::vector<std::string> &items, Color color);
    void shellModulo(const std::string &titulo, const std::string &subtitulo, Color color, int notificaciones);
    void menuLateral(const std::string &tituloModulo, const std::vector<std::string> &opciones, int activo, Color color);
    void aviso(Rectangle rect, const std::string &titulo, const std::string &texto, Color color);
    void textoRecortado(const std::string &texto, int x, int y, int tam, Color color, int maxChars);
    void imagenCubierta(const std::string &ruta, Rectangle rect, Color tinte = WHITE);
}

#endif
