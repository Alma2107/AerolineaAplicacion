#ifndef PASAJERO_H
#define PASAJERO_H

#include <string>

class Pasajero
{
private:
    int id;
    std::string tipoDocumento;
    std::string numeroDocumento;
    std::string nombre;
    std::string apellido;
    std::string fechaNacimiento;
    bool asistenciaEspecial;
    std::string detallesMedicos;
    bool checkIn;

public:
    Pasajero() : id(0), tipoDocumento("DNI"), numeroDocumento(""), nombre(""), apellido(""),
                 fechaNacimiento("2000-01-01"), asistenciaEspecial(false), detallesMedicos(""), checkIn(false) {}
    Pasajero(int id, const std::string &tipoDocumento, const std::string &numeroDocumento,
             const std::string &nombre, const std::string &apellido, const std::string &fechaNacimiento,
             bool asistenciaEspecial, const std::string &detallesMedicos, bool checkIn)
        : id(id), tipoDocumento(tipoDocumento), numeroDocumento(numeroDocumento),
          nombre(nombre), apellido(apellido), fechaNacimiento(fechaNacimiento),
          asistenciaEspecial(asistenciaEspecial), detallesMedicos(detallesMedicos), checkIn(checkIn) {}

    int getId() const { return id; }
    void setId(int valor) { id = valor; }

    std::string getTipoDocumento() const { return tipoDocumento; }
    void setTipoDocumento(const std::string &valor) { tipoDocumento = valor; }

    std::string getNumeroDocumento() const { return numeroDocumento; }
    void setNumeroDocumento(const std::string &valor) { numeroDocumento = valor; }

    std::string getNombre() const { return nombre; }
    void setNombre(const std::string &valor) { nombre = valor; }

    std::string getApellido() const { return apellido; }
    void setApellido(const std::string &valor) { apellido = valor; }

    std::string getFechaNacimiento() const { return fechaNacimiento; }
    void setFechaNacimiento(const std::string &valor) { fechaNacimiento = valor; }

    bool requiereAsistenciaEspecial() const { return asistenciaEspecial; }
    void setAsistenciaEspecial(bool valor) { asistenciaEspecial = valor; }

    std::string getDetallesMedicos() const { return detallesMedicos; }
    void setDetallesMedicos(const std::string &valor) { detallesMedicos = valor; }

    bool tieneCheckIn() const { return checkIn; }
    void setCheckIn(bool valor) { checkIn = valor; }
};

#endif
