#ifndef MENU_PRINCIPAL_H
#define MENU_PRINCIPAL_H

#include <string>

class MenuPrincipal
{
private:
    int moduloSeleccionado;
    std::string usuario;
    std::string password;
    std::string mensaje;
    bool escribiendoUsuario;
    bool dbLista;
    int moduloParaAbrir;

    void dibujarMenu();
    void dibujarLogin();
    void capturarTexto(std::string &texto, bool ocultar);
    void abrirModulo(int modulo);
    std::string nombreModulo(int modulo) const;
    void seleccionarModulo(int modulo);

public:
    MenuPrincipal();
    void ejecutar();
};

#endif
