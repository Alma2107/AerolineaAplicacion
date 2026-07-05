#ifndef EMPLEADO_H
#define EMPLEADO_H

#include <string>

class Empleado
{
private:
    int id;
    std::string nombre;
    std::string usuario;
    std::string password;
    int modulo;

public:
    Empleado() : id(0), nombre(""), usuario(""), password(""), modulo(0) {}
    Empleado(int id, const std::string &nombre, const std::string &usuario,
             const std::string &password, int modulo)
        : id(id), nombre(nombre), usuario(usuario), password(password), modulo(modulo) {}

    int getId() const { return id; }
    void setId(int valor) { id = valor; }

    std::string getNombre() const { return nombre; }
    void setNombre(const std::string &valor) { nombre = valor; }

    std::string getUsuario() const { return usuario; }
    void setUsuario(const std::string &valor) { usuario = valor; }

    std::string getPassword() const { return password; }
    void setPassword(const std::string &valor) { password = valor; }

    int getModulo() const { return modulo; }
    void setModulo(int valor) { modulo = valor; }
};

#endif
