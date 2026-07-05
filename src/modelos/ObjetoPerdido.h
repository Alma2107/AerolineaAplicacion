#ifndef OBJETO_PERDIDO_H
#define OBJETO_PERDIDO_H

#include <string>

class ObjetoPerdido
{
private:
    int id;
    std::string tipo;
    std::string descripcion;
    int idAvion;
    std::string numeroAsiento;
    std::string codigoEquipaje;
    std::string fechaHallazgo;
    std::string ubicacionExacta;
    std::string empleadoHallazgo;
    std::string estado;
    std::string fechaRegistro;

public:
    ObjetoPerdido()
        : id(0), tipo("Objeto"), descripcion(""), idAvion(0), numeroAsiento(""), codigoEquipaje(""), fechaHallazgo(""), ubicacionExacta(""), empleadoHallazgo(""), estado(""), fechaRegistro("") {}

    ObjetoPerdido(int id, const std::string &tipo, const std::string &descripcion, int idAvion,
                  const std::string &numeroAsiento, const std::string &codigoEquipaje,
                  const std::string &fechaHallazgo, const std::string &ubicacionExacta,
                  const std::string &empleadoHallazgo,
                  const std::string &estado, const std::string &fechaRegistro)
        : id(id), tipo(tipo), descripcion(descripcion), idAvion(idAvion), numeroAsiento(numeroAsiento), codigoEquipaje(codigoEquipaje), fechaHallazgo(fechaHallazgo), ubicacionExacta(ubicacionExacta),
          empleadoHallazgo(empleadoHallazgo), estado(estado), fechaRegistro(fechaRegistro) {}

    int getId() const { return id; }
    std::string getTipo() const { return tipo; }
    std::string getDescripcion() const { return descripcion; }
    int getIdAvion() const { return idAvion; }
    std::string getNumeroAsiento() const { return numeroAsiento; }
    std::string getCodigoEquipaje() const { return codigoEquipaje; }
    std::string getFechaHallazgo() const { return fechaHallazgo; }
    std::string getUbicacionExacta() const { return ubicacionExacta; }
    std::string getEmpleadoHallazgo() const { return empleadoHallazgo; }
    std::string getEstado() const { return estado; }
    std::string getFechaRegistro() const { return fechaRegistro; }

    int getId() const { return id; }
    std::string getDescripcion() const { return descripcion; }
    int getIdAvion() const { return idAvion; }
    std::string getFechaHallazgo() const { return fechaHallazgo; }
    std::string getUbicacionExacta() const { return ubicacionExacta; }
    std::string getEmpleadoHallazgo() const { return empleadoHallazgo; }
    std::string getEstado() const { return estado; }
    std::string getFechaRegistro() const { return fechaRegistro; }
};

#endif
