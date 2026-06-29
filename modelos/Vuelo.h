#ifndef VUELO_H
#define VUELO_H

#include <string>

class Vuelo
{
private:
    int id;
    std::string numero;
    std::string origen;
    std::string destino;
    std::string fechaSalida;
    std::string fechaLlegada;
    double precioBase;
    std::string estado;
    int idAvion;

public:
    Vuelo() : id(0), numero(""), origen(""), destino(""), fechaSalida(""), fechaLlegada(""), precioBase(0), estado("Programado"), idAvion(0) {}
    Vuelo(int id, const std::string &numero, const std::string &origen, const std::string &destino,
          const std::string &fechaSalida, const std::string &fechaLlegada, double precioBase,
          const std::string &estado, int idAvion)
        : id(id), numero(numero), origen(origen), destino(destino), fechaSalida(fechaSalida),
          fechaLlegada(fechaLlegada), precioBase(precioBase), estado(estado), idAvion(idAvion) {}

    int getId() const { return id; }
    void setId(int valor) { id = valor; }

    std::string getNumero() const { return numero; }
    void setNumero(const std::string &valor) { numero = valor; }

    std::string getOrigen() const { return origen; }
    void setOrigen(const std::string &valor) { origen = valor; }

    std::string getDestino() const { return destino; }
    void setDestino(const std::string &valor) { destino = valor; }

    std::string getFechaSalida() const { return fechaSalida; }
    void setFechaSalida(const std::string &valor) { fechaSalida = valor; }

    std::string getFechaLlegada() const { return fechaLlegada; }
    void setFechaLlegada(const std::string &valor) { fechaLlegada = valor; }

    double getPrecioBase() const { return precioBase; }
    void setPrecioBase(double valor) { precioBase = valor; }

    std::string getEstado() const { return estado; }
    void setEstado(const std::string &valor) { estado = valor; }

    int getIdAvion() const { return idAvion; }
    void setIdAvion(int valor) { idAvion = valor; }
};

#endif
