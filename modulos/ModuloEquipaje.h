#ifndef MODULO_EQUIPAJE_H
#define MODULO_EQUIPAJE_H

#include "../dao/EquipajeDAO.h"
#include <string>

class ModuloEquipaje
{
private:
    EquipajeDAO equipajeDAO;
    std::string mensaje;
    int vista;
    int foco;
    std::string idTicket;
    std::string idTipo;
    std::string peso;
    std::string precio;
    std::string etiqueta;

    void dibujarNavegacion();
    void dibujarRegistrar();
    void dibujarRastrear();
    void dibujarListado();

public:
    ModuloEquipaje();
    void mostrar();
};

#endif
