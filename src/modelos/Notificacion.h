#ifndef NOTIFICACION_H
#define NOTIFICACION_H

#include <string>

class Notificacion
{
private:
    int id;
    int moduloDestino;
    std::string tipo;
    std::string mensaje;
    bool leida;
    std::string fecha;

public:
    Notificacion() : id(0), moduloDestino(0), tipo(""), mensaje(""), leida(false), fecha("") {}
    Notificacion(int id, int moduloDestino, const std::string &tipo, const std::string &mensaje,
                 bool leida, const std::string &fecha)
        : id(id), moduloDestino(moduloDestino), tipo(tipo), mensaje(mensaje), leida(leida), fecha(fecha) {}

    int getId() const { return id; }
    int getModuloDestino() const { return moduloDestino; }
    std::string getTipo() const { return tipo; }
    std::string getMensaje() const { return mensaje; }
    bool estaLeida() const { return leida; }
    std::string getFecha() const { return fecha; }
};

#endif
