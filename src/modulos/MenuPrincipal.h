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
    bool mostrandoConfig;
    std::string dbHost;
    std::string dbPort;
    std::string dbUser;
    std::string dbPass;
    bool focoHost;
    bool focoPort;
    bool focoUser;
    bool focoPass;
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
    void dibujarConfig();
    void guardarConfig();
};

#endif
