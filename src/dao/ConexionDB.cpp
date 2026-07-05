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

static std::string buscarMysqlExe()
{
    const std::string rutas[] = {
        "C:\\xampp\\mysql\\bin\\mysql.exe",
        "C:\\Program Files\\MySQL\\MySQL Server 8.0\\bin\\mysql.exe",
        "C:\\Program Files\\MySQL\\MySQL Server 5.7\\bin\\mysql.exe",
        "C:\\Program Files\\MariaDB\\bin\\mysql.exe",
        "C:\\Program Files (x86)\\MySQL\\MySQL Server 8.0\\bin\\mysql.exe",
        "C:\\Program Files (x86)\\MariaDB\\bin\\mysql.exe"
    };

    for (const std::string &ruta : rutas)
    {
        if (archivoExiste(ruta))
            return ruta;
    }

    FILE *pipe = _popen("where mysql 2>nul", "r");
    if (pipe)
    {
        char buffer[512];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            std::string resultado(buffer);
            while (!resultado.empty() && (resultado.back() == '\n' || resultado.back() == '\r'))
                resultado.pop_back();
            _pclose(pipe);
            if (!resultado.empty())
                return resultado;
        }
        else
        {
            _pclose(pipe);
        }
    }

    return "mysql";
}

ConexionDB::ConexionDB() : conectada(false), nombreBase("aerolinea"), mysqlExe(buscarMysqlExe()) {}

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
    comando << "\"" << mysqlExe << "\" --default-character-set=utf8mb4 -h 127.0.0.1 -P 3306 -N -B -u root";
    if (usarBase)
        comando << " --database=" << nombreBase;
    comando << " -e \"" << limpio << "\"";
    return comando.str();
}

bool ConexionDB::conectar()
{
    if (!archivoExiste(mysqlExe))
    {
        std::ofstream log("logs/conexion_aerogest.log", std::ios::app);
        log << "Archivo mysql.exe no encontrado: " << mysqlExe << std::endl;
        return false;
    }
    
    std::string tempFile = "build\\temp_test.txt";
    std::string cmd = "\"" + mysqlExe + "\" -h 127.0.0.1 -u root -e \"SELECT 1\" > \"" + tempFile + "\" 2>&1";
    
    int resultado = std::system(cmd.c_str());
    
    bool ok = false;
    if (archivoExiste(tempFile))
    {
        std::ifstream archivo(tempFile);
        std::string contenido;
        if (std::getline(archivo, contenido) && contenido == "1")
            ok = true;
        archivo.close();
        std::remove(tempFile.c_str());
    }
    
    if (!ok)
    {
        std::ofstream log("logs/conexion_aerogest.log", std::ios::app);
        log << "No se pudo conectar a MySQL en 127.0.0.1. Verifique que XAMPP este ejecutando con MySQL iniciado." << std::endl;
    }
    
    conectada = ok;
    return ok;
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

    std::string tempFile = "build\\temp_exec.txt";
    std::string cmd = "\"" + mysqlExe + "\" -h 127.0.0.1 -u root --database=" + nombreBase + " -e \"" + sql + "\" > \"" + tempFile + "\" 2>&1";
    
    int resultado = std::system(cmd.c_str());
    
    bool ok = (resultado == 0);
    
    if (archivoExiste(tempFile))
        std::remove(tempFile.c_str());
    
    if (!ok)
    {
        std::ofstream log("logs/conexion_aerogest.log", std::ios::app);
        log << "Error al ejecutar SQL. Codigo: " << resultado << " - SQL: " << sql << std::endl;
    }
    
    return ok;
}

std::vector<std::vector<std::string>> ConexionDB::consultar(const std::string &sql) const
{
    std::vector<std::vector<std::string>> filas;
    if (!archivoExiste(mysqlExe))
        return filas;

    std::string tempFile = "build\\temp_query.txt";
    std::string sqlLimpio = sql;
    for (char &c : sqlLimpio)
    {
        if (c == '"')
            c = '\'';
    }
    
    std::string cmd = "\"" + mysqlExe + "\" -h 127.0.0.1 -u root --database=" + nombreBase + " -N -B -e \"" + sqlLimpio + "\" > \"" + tempFile + "\" 2>&1";
    
    int resultado = std::system(cmd.c_str());
    
    if (resultado == 0 && archivoExiste(tempFile))
    {
        std::ifstream archivo(tempFile);
        std::string linea;
        while (std::getline(archivo, linea))
        {
            std::vector<std::string> columnas;
            std::stringstream ss(linea);
            std::string columna;
            while (std::getline(ss, columna, '\t'))
                columnas.push_back(columna == "NULL" ? "" : columna);
            if (!linea.empty())
                filas.push_back(columnas);
        }
        archivo.close();
    }
    
    if (archivoExiste(tempFile))
        std::remove(tempFile.c_str());

    return filas;
}

bool ConexionDB::inicializar()
{
    static bool esquemaPreparado = false;

    if (!archivoExiste(mysqlExe))
        return false;

    conectada = ejecutar("SELECT 1");
    if (conectada && !esquemaPreparado)
    {
        ejecutar("ALTER TABLE tickets_detalle MODIFY codigo_reserva_pnr VARCHAR(20) NOT NULL");

        ejecutar("CREATE TABLE IF NOT EXISTS checkins ("
                 "id_checkin INT NOT NULL AUTO_INCREMENT,"
                 "id_ticket INT NOT NULL,"
                 "codigo_reserva_pnr VARCHAR(20) NOT NULL,"
                 "fecha DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                 "estado VARCHAR(30) NOT NULL DEFAULT 'Realizado',"
                 "PRIMARY KEY (id_checkin),"
                 "UNIQUE KEY uq_checkins_ticket (id_ticket),"
                 "KEY idx_checkins_codigo (codigo_reserva_pnr),"
                 "CONSTRAINT fk_checkins_ticket FOREIGN KEY (id_ticket) REFERENCES tickets_detalle(id_ticket) ON DELETE CASCADE"
                 ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci");

        ejecutar("CREATE TABLE IF NOT EXISTS notificaciones ("
                 "id_notificacion INT NOT NULL AUTO_INCREMENT,"
                 "modulo_destino INT NOT NULL,"
                 "tipo VARCHAR(50) NOT NULL,"
                 "mensaje TEXT NOT NULL,"
                 "leida TINYINT(1) NOT NULL DEFAULT 0,"
                 "fecha DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                 "PRIMARY KEY (id_notificacion),"
                 "KEY idx_notificaciones_modulo (modulo_destino, leida, fecha)"
                 ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci");

        esquemaPreparado = true;
    }
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
