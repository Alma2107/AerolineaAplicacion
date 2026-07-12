# Configuracion de Base de Datos

Este directorio contiene archivos de configuracion para la conexion a MySQL.

## Archivo `db.ini`

Edita `db.ini` para personalizar los parametros de conexion a MySQL:

- **MYSQL_HOST**: Direccion del servidor (por defecto: 127.0.0.1)
- **MYSQL_PORT**: Puerto del servidor (por defecto: 3306)
- **MYSQL_USER**: Usuario de MySQL (por defecto: root)
- **MYSQL_PASSWORD**: Contrasena (dejar vacio si es XAMPP por defecto)
- **MYSQL_DATABASE**: Nombre de la BD (por defecto: aerolinea)
- **MYSQL_CHARSET**: Codificacion de caracteres (por defecto: utf8mb4)

## Variables de Entorno (Mayor Prioridad)

Si estableces variables de entorno en Windows, estas sobrecargan los valores de `db.ini`:

```
set MYSQL_HOST=127.0.0.1
set MYSQL_PORT=3306
set MYSQL_USER=root
set MYSQL_PASSWORD=
set MYSQL_DATABASE=aerolinea
```

## Configuracion Tipica para XAMPP

En XAMPP, la configuracion por defecto es:
- **Host**: 127.0.0.1 o localhost
- **Puerto**: 3306
- **Usuario**: root
- **Contrasena**: (vacia)
- **Base**: aerolinea

No necesitas cambiar nada en este archivo si usas XAMPP por defecto.

## Verificacion

Ejecuta `scripts\verificar_mysql.bat` para confirmar que MySQL esta disponible y funcionando.

## Alternativa: Contrasena en Variables de Entorno (Mas Seguro)

En lugar de dejar la contrasena aqui, puedes establecerla como variable de entorno:

```batch
setx MYSQL_PASSWORD tu_contrasena
```

Luego deja `MYSQL_PASSWORD=` vacio en `db.ini`.
