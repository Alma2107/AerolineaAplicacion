#include "ConexionDB.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

static bool archivoExiste(const std::string &ruta)
{
    std::ifstream archivo(ruta.c_str());
    return archivo.good();
}

ConexionDB::ConexionDB() : conectada(false), nombreBase("aerolinea"), mysqlExe("C:\\xampp\\mysql\\bin\\mysql.exe") {}

std::string ConexionDB::comandoMysql(const std::string &sql, bool usarBase) const
{
    std::string limpio = sql;
    for (char &c : limpio)
    {
        if (c == '"')
            c = '\'';
        if (c == '\n' || c == '\r')
            c = ' ';
    }

    std::stringstream comando;
    comando << "\"" << mysqlExe << "\" --default-character-set=utf8mb4 --protocol=tcp -h 127.0.0.1 -P 3306 --connect-timeout=5 -N -B -u root ";
    if (usarBase)
        comando << "--database=" << nombreBase << " ";
    comando << "-e \"" << limpio << "\" 2>>conexion_aerogest.log";
    return comando.str();
}

bool ConexionDB::conectar()
{
    conectada = archivoExiste(mysqlExe) && ejecutar("SELECT 1");
    return conectada;
}

void ConexionDB::desconectar()
{
    conectada = false;
}

bool ConexionDB::estaConectada() const
{
    return conectada;
}

std::string ConexionDB::getNombreBase() const
{
    return nombreBase;
}

bool ConexionDB::ejecutar(const std::string &sql) const
{
    if (!archivoExiste(mysqlExe))
        return false;

    int resultado = std::system(comandoMysql(sql, true).c_str());
    return resultado == 0;
}

std::vector<std::vector<std::string>> ConexionDB::consultar(const std::string &sql) const
{
    std::vector<std::vector<std::string>> filas;
    if (!archivoExiste(mysqlExe))
        return filas;

    FILE *pipe = _popen(comandoMysql(sql, true).c_str(), "r");
    if (!pipe)
        return filas;

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        std::string linea(buffer);
        while (!linea.empty() && (linea.back() == '\n' || linea.back() == '\r'))
            linea.pop_back();

        std::vector<std::string> columnas;
        std::stringstream ss(linea);
        std::string columna;
        while (std::getline(ss, columna, '\t'))
            columnas.push_back(columna == "NULL" ? "" : columna);
        filas.push_back(columnas);
    }

    _pclose(pipe);
    return filas;
}

bool ConexionDB::inicializar()
{
    if (!archivoExiste(mysqlExe))
        return false;

    conectada = ejecutar("SELECT 1");
    return conectada;
}

std::string ConexionDB::escapar(const std::string &valor)
{
    std::string salida;
    for (char c : valor)
    {
        if (c == '\\' || c == '\'')
            salida.push_back('\\');
        salida.push_back(c);
    }
    return salida;
}

int ConexionDB::convertirEntero(const std::string &valor, int defecto)
{
    try
    {
        return std::stoi(valor);
    }
    catch (...)
    {
        return defecto;
    }
}

double ConexionDB::convertirDouble(const std::string &valor, double defecto)
{
    try
    {
        return std::stod(valor);
    }
    catch (...)
    {
        return defecto;
    }
}
