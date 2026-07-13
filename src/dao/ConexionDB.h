#ifndef CONEXION_DB_H
#define CONEXION_DB_H

#include <string>
#include <vector>

class ConexionDB
{
private:
    bool conectada;
    std::string nombreBase;
    std::string mysqlExe;
    std::string dbHost;
    std::string dbPort;
    std::string dbUsuario;
    std::string dbPassword;
    std::string dbCharset;

    std::string comandoMysql(const std::string &sql, bool usarBase) const;

public:
    ConexionDB();

    bool conectar();
    void desconectar();
    bool estaConectada() const;
    std::string getNombreBase() const;

    bool ejecutar(const std::string &sql) const;
    std::vector<std::vector<std::string>> consultar(const std::string &sql) const;
    bool inicializar();

    static std::string escapar(const std::string &valor);
    static int convertirEntero(const std::string &valor, int defecto = 0);
    static double convertirDouble(const std::string &valor, double defecto = 0);
    static std::string detectarMysqlExe();
};

#endif
