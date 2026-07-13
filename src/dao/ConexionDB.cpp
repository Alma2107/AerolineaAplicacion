#include "ConexionDB.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <direct.h>

static bool archivoExiste(const std::string &ruta)
{
    std::ifstream archivo(ruta.c_str());
    return archivo.good();
}

static std::string rutaDirectorio(const std::string &ruta)
{
    size_t pos = ruta.find_last_of("\\/");
    return pos == std::string::npos ? ruta : ruta.substr(0, pos);
}

static std::string rutaEjecutable()
{
    char buffer[_MAX_PATH];
    DWORD resultado = GetModuleFileNameA(NULL, buffer, sizeof(buffer));
    return resultado > 0 ? std::string(buffer, resultado) : std::string();
}

static std::string rutaActual();

static std::string rutaProyecto()
{
    std::string exePath = rutaEjecutable();
    if (exePath.empty())
        return rutaActual();

    std::string dir = rutaDirectorio(exePath);
    if (dir.size() >= 6 && dir.compare(dir.size() - 6, 6, "\\build") == 0)
        dir = dir.substr(0, dir.size() - 6);

    while (!dir.empty() && (dir.back() == '\\' || dir.back() == '/'))
        dir.pop_back();

    return dir.empty() ? rutaActual() : dir;
}

static std::string rutaActual()
{
    char cwd[_MAX_PATH];
    if (_getcwd(cwd, sizeof(cwd)) != nullptr)
        return std::string(cwd);
    return std::string();
}

static std::string rutaProyectoArchivo(const std::string &ruta)
{
    std::string base = rutaProyecto();
    if (base.empty())
        return ruta;
    if (!ruta.empty() && (ruta[0] == '\\' || ruta[0] == '/'))
        return base + ruta;
    return base + "\\" + ruta;
}

static bool crearDirectorioSiNoExiste(const std::string &ruta)
{
    std::string dir = rutaDirectorio(ruta);
    if (dir.empty())
        return false;
    if (archivoExiste(dir))
        return true;
    return _mkdir(dir.c_str()) == 0 || errno == EEXIST;
}

static std::string rutaLog()
{
    return rutaProyectoArchivo("logs\\conexion_aerogest.log");
}

static std::string rutaTemp(const std::string &archivo)
{
    return rutaProyectoArchivo("build\\" + archivo);
}

static int ejecutarEnShell(const std::string &comando)
{
    std::string shell = "cmd.exe /c \"" + comando + "\"";
    return std::system(shell.c_str());
}

static int ejecutarCapturando(const std::string &comando, std::string &salida)
{
    std::string cmd = comando + " 2>&1";
    FILE *pipe = _popen(cmd.c_str(), "r");
    if (!pipe)
        return -1;
    char buffer[256];
    salida.clear();
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        salida += buffer;
    int rc = _pclose(pipe);
    return rc;
}

static int ejecutarMysqlConStdin(const std::string &mysqlExePath, const std::string &host, const std::string &port,
                                 const std::string &usuario, const std::string &password, const std::string &charset,
                                 const std::string &sql, bool usarBase, const std::string &nombreBase, std::string &salida)
{
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;
    HANDLE hChildStd_IN_Rd = NULL;
    HANDLE hChildStd_IN_Wr = NULL;

    if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &sa, 0))
        return -1;
    if (!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
    {
        CloseHandle(hChildStd_OUT_Rd);
        CloseHandle(hChildStd_OUT_Wr);
        return -1;
    }

    if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &sa, 0))
    {
        CloseHandle(hChildStd_OUT_Rd);
        CloseHandle(hChildStd_OUT_Wr);
        return -1;
    }
    if (!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
    {
        CloseHandle(hChildStd_OUT_Rd);
        CloseHandle(hChildStd_OUT_Wr);
        CloseHandle(hChildStd_IN_Rd);
        CloseHandle(hChildStd_IN_Wr);
        return -1;
    }

    std::stringstream cmd;
    cmd << "\"" << mysqlExePath << "\"";
    cmd << " --default-character-set=" << charset;
    cmd << " --protocol=tcp";
    cmd << " -h " << host << " -P " << port;
    cmd << " --connect-timeout=5 -N -B -u " << usuario;
    if (!password.empty())
        cmd << " --password=\"" << password << "\"";
    if (usarBase)
        cmd << " --database=" << nombreBase;

    std::string cmdstr = cmd.str();
    std::vector<char> cmdline(cmdstr.begin(), cmdstr.end());
    cmdline.push_back('\0');

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hChildStd_OUT_Wr;
    si.hStdOutput = hChildStd_OUT_Wr;
    si.hStdInput = hChildStd_IN_Rd;
    si.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&pi, sizeof(pi));

    BOOL ok = CreateProcessA(NULL, cmdline.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    if (!ok)
    {
        CloseHandle(hChildStd_OUT_Rd);
        CloseHandle(hChildStd_OUT_Wr);
        CloseHandle(hChildStd_IN_Rd);
        CloseHandle(hChildStd_IN_Wr);
        return -1;
    }

    // Close handles we don't need
    CloseHandle(hChildStd_OUT_Wr);
    CloseHandle(hChildStd_IN_Rd);

    // Write SQL to child's stdin
    std::string sqlToSend = sql + "\n";
    DWORD written = 0;
    BOOL writeOk = WriteFile(hChildStd_IN_Wr, sqlToSend.c_str(), (DWORD)sqlToSend.size(), &written, NULL);
    CloseHandle(hChildStd_IN_Wr);

    // Read child's output
    const int bufSize = 4096;
    char buffer[bufSize];
    salida.clear();
    DWORD readBytes = 0;
    while (ReadFile(hChildStd_OUT_Rd, buffer, bufSize, &readBytes, NULL) && readBytes > 0)
    {
        salida.append(buffer, buffer + readBytes);
    }

    CloseHandle(hChildStd_OUT_Rd);

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return (int)exitCode;
}

static std::string obtenerVariableEntorno(const std::string &clave, const std::string &porDefecto = "")
{
    const char *valor = std::getenv(clave.c_str());
    return valor ? std::string(valor) : porDefecto;
}

static bool comandoDisponible(const std::string &comando)
{
    if (archivoExiste(comando))
        return true;
    std::string prueba = "where \"" + comando + "\" >nul 2>&1";
    return std::system(prueba.c_str()) == 0;
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

ConexionDB::ConexionDB()
    : conectada(false),
      nombreBase(obtenerVariableEntorno("MYSQL_DATABASE", obtenerVariableEntorno("DB_DATABASE", "aerolinea"))),
      mysqlExe(buscarMysqlExe()),
    dbHost(obtenerVariableEntorno("MYSQL_HOST", obtenerVariableEntorno("DB_HOST", "127.0.0.1"))),
      dbPort(obtenerVariableEntorno("MYSQL_PORT", obtenerVariableEntorno("DB_PORT", "3306"))),
      dbUsuario(obtenerVariableEntorno("MYSQL_USER", obtenerVariableEntorno("DB_USER", "root"))),
      dbPassword(obtenerVariableEntorno("MYSQL_PASSWORD", obtenerVariableEntorno("DB_PASSWORD", ""))),
      dbCharset("utf8mb4")
{
    // Si existe un archivo de configuración en config\db.ini dentro del proyecto,
    // cargar valores desde allí para permitir que la UI guarde credenciales.
    try
    {
        std::string archivoCfg = rutaProyectoArchivo("config\\db.ini");
        if (archivoExiste(archivoCfg))
        {
            std::ifstream cfg(archivoCfg);
            std::string linea;
            while (std::getline(cfg, linea))
            {
                // Ignorar líneas vacías y comentarios
                if (linea.empty() || linea[0] == '#')
                    continue;
                size_t eq = linea.find('=');
                if (eq == std::string::npos)
                    continue;
                std::string clave = linea.substr(0, eq);
                std::string valor = linea.substr(eq + 1);
                // Trim espacios
                auto trim = [](std::string &s) {
                    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t'))
                        s.pop_back();
                    size_t i = 0;
                    while (i < s.size() && (s[i] == ' ' || s[i] == '\t'))
                        ++i;
                    if (i > 0)
                        s = s.substr(i);
                };
                trim(clave);
                trim(valor);
                if (clave == "MYSQL_HOST")
                    dbHost = valor;
                else if (clave == "MYSQL_PORT")
                    dbPort = valor;
                else if (clave == "MYSQL_USER")
                    dbUsuario = valor;
                else if (clave == "MYSQL_PASSWORD")
                    dbPassword = valor;
                else if (clave == "MYSQL_DATABASE")
                    nombreBase = valor;
                else if (clave == "MYSQL_CHARSET")
                    dbCharset = valor;
                else if (clave == "MYSQL_EXE")
                    mysqlExe = valor;
            }
            cfg.close();
        }
    }
    catch (...)
    {
        // No hacer nada si falla la lectura; se usan los valores por defecto
    }
}

std::string ConexionDB::detectarMysqlExe()
{
    return buscarMysqlExe();
}

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

    std::string host = dbHost;
    if (host == "localhost")
        host = "127.0.0.1";

    std::stringstream comando;
    comando << "\"" << mysqlExe << "\" --default-character-set=" << dbCharset;
    comando << " --protocol=tcp";
    comando << " -h " << host << " -P " << dbPort;
    comando << " --connect-timeout=5 -N -B -u " << dbUsuario;
    if (!dbPassword.empty())
        comando << " --password=\"" << dbPassword << "\"";
    if (usarBase)
        comando << " --database=" << nombreBase;
    comando << " -e \"" << limpio << "\"";
    return comando.str();
}

bool ConexionDB::conectar()
{
    if (!comandoDisponible(mysqlExe))
    {
        crearDirectorioSiNoExiste(rutaLog());
        std::ofstream log(rutaLog(), std::ios::app);
        log << "Archivo mysql.exe no encontrado: " << mysqlExe << std::endl;
        return false;
    }
    
    crearDirectorioSiNoExiste(rutaLog());
    std::string tempFile = rutaTemp("temp_test.txt");
    std::string contenido;
    std::string hostReal = dbHost == "localhost" ? "127.0.0.1" : dbHost;
        // Intentar ejecutar el cliente mysql y capturar salida directamente usando CreateProcess
        std::string salida;
        int rc = ejecutarMysqlConStdin(mysqlExe, hostReal, dbPort, dbUsuario, dbPassword, dbCharset, "SELECT 1", false, nombreBase, salida);
    bool ok = (rc == 0);
    if (!salida.empty())
    {
        crearDirectorioSiNoExiste(tempFile);
        std::ofstream archivo(tempFile);
        archivo << salida;
        archivo.close();
        contenido = salida;
    }
    
    if (!ok)
    {
        std::ofstream log(rutaLog(), std::ios::app);
        std::string hostReal = dbHost == "localhost" ? "127.0.0.1" : dbHost;
        log << "No se pudo conectar a MySQL en " << hostReal << ":" << dbPort << " con usuario=" << dbUsuario << ". Codigo: " << rc << std::endl;
        log << "Comando ejecutado: " << mysqlExe << " --protocol=tcp -h " << hostReal << " -P " << dbPort << " -u " << dbUsuario << " (SQL via stdin)" << std::endl;
        if (!contenido.empty())
            log << "Salida mysql: " << contenido << std::endl;
        else
            log << "No se obtuvo salida de mysql. Compruebe credenciales y configuracion de MySQL." << std::endl;
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
    if (!comandoDisponible(mysqlExe))
        return false;

    std::string tempFile = rutaTemp("temp_exec.txt");
    std::string sqlLimpio = sql;
    for (char &c : sqlLimpio)
    {
        if (c == '"')
            c = '\'';
        if (c == '\n' || c == '\r')
            c = ' ';
    }
        // Ejecutar y capturar salida sin usar redireccionamientos en system()
        std::string salida;
        std::string hostReal = dbHost == "localhost" ? "127.0.0.1" : dbHost;
        int resultado = ejecutarMysqlConStdin(mysqlExe, hostReal, dbPort, dbUsuario, dbPassword, dbCharset, sqlLimpio, true, nombreBase, salida);
    bool ok2 = (resultado == 0);
    if (!ok2)
    {
        crearDirectorioSiNoExiste(rutaLog());
        std::ofstream log(rutaLog(), std::ios::app);
        log << "Error al ejecutar SQL. Codigo: " << resultado << " - SQL: " << sql << std::endl;
        log << "Comando ejecutado: " << comandoMysql(sqlLimpio, true) << std::endl;
        if (!salida.empty())
            log << "Salida mysql: " << salida << std::endl;
    }
    return ok2;
}

std::vector<std::vector<std::string>> ConexionDB::consultar(const std::string &sql) const
{
    std::vector<std::vector<std::string>> filas;
    if (!comandoDisponible(mysqlExe))
        return filas;

    std::string tempFile = rutaTemp("temp_query.txt");
    std::string sqlLimpio = sql;
    for (char &c : sqlLimpio)
    {
        if (c == '"')
            c = '\'';
    }
    
    // Ejecutar el comando y capturar la salida
    std::string salida;
    int resultado = ejecutarCapturando(comandoMysql(sqlLimpio, true), salida);
    if (resultado == 0 && !salida.empty())
    {
        std::istringstream iss(salida);
        std::string linea;
        while (std::getline(iss, linea))
        {
            std::vector<std::string> columnas;
            std::stringstream ss(linea);
            std::string columna;
            while (std::getline(ss, columna, '\t'))
                columnas.push_back(columna == "NULL" ? "" : columna);
            if (!linea.empty())
                filas.push_back(columnas);
        }
    }
    else if (resultado != 0)
    {
        crearDirectorioSiNoExiste(rutaLog());
        std::ofstream log(rutaLog(), std::ios::app);
        log << "Error al consultar SQL. Codigo: " << resultado << " - SQL: " << sql << std::endl;
        log << "Comando ejecutado: " << comandoMysql(sqlLimpio, true) << std::endl;
        if (!salida.empty())
            log << "Salida mysql: " << salida << std::endl;
    }

    return filas;
}

bool ConexionDB::inicializar()
{
    static bool esquemaPreparado = false;

    if (!conectar())
        return false;

    std::string crearDbCmd = comandoMysql("CREATE DATABASE IF NOT EXISTS " + nombreBase + " CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci", false);
        // Crear la base si no existe usando el cliente mysql vía stdin
        std::string crearSql = "CREATE DATABASE IF NOT EXISTS " + nombreBase + " CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci";
        std::string salidaCrear;
        std::string hostReal = dbHost == "localhost" ? "127.0.0.1" : dbHost;
        if (ejecutarMysqlConStdin(mysqlExe, hostReal, dbPort, dbUsuario, dbPassword, dbCharset, crearSql, false, nombreBase, salidaCrear) != 0)
            return false;
    if (rcCrear != 0)
        return false;

    conectada = ejecutar("SELECT 1");
    if (conectada && !esquemaPreparado)
    {
        auto tablasBase = consultar("SHOW TABLES LIKE 'vuelos'");
        std::string archivoSql = rutaProyectoArchivo("database\\aerolinea.sql");
        if (tablasBase.empty() && archivoExiste(archivoSql))
        {
                    // Importar el archivo SQL leyendo su contenido y enviándolo por stdin al cliente mysql
                    std::ifstream fsql(archivoSql, std::ios::in | std::ios::binary);
                    if (fsql)
                    {
                        std::stringstream buffer;
                        buffer << fsql.rdbuf();
                        std::string sqlContent = buffer.str();
                        std::string salidaImport;
                        int resultadoImportacion = ejecutarMysqlConStdin(mysqlExe, hostReal, dbPort, dbUsuario, dbPassword, dbCharset, sqlContent, true, nombreBase, salidaImport);
                        if (resultadoImportacion != 0)
                        {
                            crearDirectorioSiNoExiste(rutaLog());
                            std::ofstream log(rutaLog(), std::ios::app);
                            log << "No se pudo importar " << archivoSql << ". Codigo: " << resultadoImportacion << std::endl;
                            log << "Nota: Verifique que MySQL este ejecutandose y que el archivo SQL exista." << std::endl;
                            if (!salidaImport.empty())
                                log << "Salida mysql: " << salidaImport << std::endl;
                        }
                    }
                    else
                    {
                        crearDirectorioSiNoExiste(rutaLog());
                        std::ofstream log(rutaLog(), std::ios::app);
                        log << "Archivo SQL no encontrado: " << archivoSql << std::endl;
                    }
        }

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
