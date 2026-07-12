# Troubleshooting - Problemas Comunes de Conexion a BD

## Problema 1: "No se pudo conectar a MySQL en 127.0.0.1:3306"

### Síntomas
- La app muestra error de conexión a base de datos
- El log muestra: `No se pudo conectar a MySQL en 127.0.0.1:3306`

### Soluciones (en orden)

**Paso 1: Verificar que MySQL está corriendo**
```batch
scripts\verificar_mysql.bat
```

Si muestra `[ERROR] MySQL no responde`, entonces:

**Paso 2: Iniciar MySQL manualmente**
- Abre `C:\xampp\xampp-control.exe`
- Busca "MySQL" en la lista
- Haz clic en "Start" (debe quedarse en verde)
- Espera 3-5 segundos hasta que inicie completamente

**Paso 3: Reintentar conexión**
```batch
scripts\ejecutar.bat
```

---

## Problema 2: "Base de datos 'aerolinea' no existe"

### Síntomas
- Puedes conectar a MySQL pero no hay tablas
- El log muestra: `SHOW TABLES LIKE 'vuelos'` devuelve vacío

### Soluciones

**Opción A: Dejar que se auto-importe** (Recomendado)
- Simplemente ejecuta `scripts\ejecutar.bat`
- La app importará `database\aerolinea.sql` automáticamente al iniciar

**Opción B: Importar manualmente**
```batch
cd c:\Users\Admin\Desktop\AerolineaAplicacion
"C:\xampp\mysql\bin\mysql.exe" -u root < database\aerolinea.sql
```

---

## Problema 3: "Access denied for user 'root'@'localhost'"

### Síntomas
- MySQL conecta pero rechaza las credenciales
- El log muestra: `Access denied`

### Causas Posibles
1. MySQL tiene contraseña en root (XAMPP estándar no la tiene)
2. Usuario no es "root"
3. Contraseña está en blanco pero debería tener una

### Soluciones

**Para XAMPP con contraseña vacía en root** (estándar):
- Los cambios ya están hechos, debería funcionar

**Si tu XAMPP tiene contraseña**:
1. Edita `config/db.ini` y establece:
   ```
   MYSQL_PASSWORD=tu_contrasena
   ```
2. O establece variable de entorno (más seguro):
   ```batch
   setx MYSQL_PASSWORD tu_contrasena
   ```

**Si cambió el usuario de root**:
1. Edita `config/db.ini`:
   ```
   MYSQL_USER=otro_usuario
   ```
2. O variable de entorno:
   ```batch
   setx MYSQL_USER otro_usuario
   ```

---

## Problema 4: "No se puede escribir archivos temporales"

### Síntomas
- Conexión funciona inicialmente pero luego falla
- Errores al ejecutar o consultar SQL
- El log muestra: `Error al ejecutar SQL`

### Causas
- La carpeta `build/` no tiene permisos de escritura
- Antivirus bloquea creación de archivos temporales
- Disco lleno

### Soluciones

**Limpiar archivos temporales**:
```batch
scripts\limpiar_temporales.bat
```

**Verificar permisos en `build/`**:
- Click derecho en carpeta `build/`
- Propiedades → Seguridad → Editar
- Asegúrate que tu usuario tiene permisos de lectura/escritura

**Si el antivirus bloquea**:
- Agrega `C:\Users\Admin\Desktop\AerolineaAplicacion\build\` a excepciones
- Reinicia la app

---

## Problema 5: "Failed to import database - Syntax error in SQL"

### Síntomas
- Se intenta importar `aerolinea.sql` pero falla
- El log muestra: `Codigo: 1` en importación

### Causas
- El archivo SQL está corrupto
- Charset incompatible
- Versión de MySQL incompatible

### Soluciones

**Verificar integridad del archivo SQL**:
```batch
echo Verificando archivo...
if exist "database\aerolinea.sql" (
    echo El archivo existe
    "C:\xampp\mysql\bin\mysql.exe" -u root -e "SOURCE database\aerolinea.sql"
) else (
    echo ERROR: Archivo no encontrado
)
```

**Reimportar manualmente con charset explícito**:
```batch
"C:\xampp\mysql\bin\mysql.exe" --default-character-set=utf8mb4 -u root < database\aerolinea.sql
```

**Si falla, restaurar desde backup** (si existe):
```batch
REM Buscar backup
dir database\*.sql
REM Y usarlo en lugar de aerolinea.sql
```

---

## Problema 6: "Puerto 3306 ya está en uso"

### Síntomas
- MySQL no inicia
- Error: "Address already in use" o similar
- Otro proceso usa puerto 3306

### Soluciones

**Opción 1: Ver qué proceso usa el puerto**
```batch
netstat -ano | findstr :3306
```

**Opción 2: Usar otro puerto**
1. Edita `config/db.ini`:
   ```
   MYSQL_PORT=3307
   ```
2. Configura MySQL en XAMPP para usar ese puerto
3. O variable de entorno:
   ```batch
   setx MYSQL_PORT 3307
   ```

**Opción 3: Reiniciar Windows** (última opción)
- Esto libera el puerto

---

## Problema 7: "Ejecutable no encontrado o falta raylib.dll"

### Síntomas
- `ejecutar.bat` no encuentra `aerolinea.exe`
- Error de DLL faltante al ejecutar

### Soluciones

**Recompilar desde cero**:
```batch
scripts\compilar.bat
```

**Verificar dependencias**:
```batch
dir build\*.dll
dir build\*.exe
```

**Si faltan DLLs**:
- Copia `raylib.dll` del directorio raíz a `build/`
- Verifica que existe `C:\raylib\raylib-6.0_win64_mingw-w64\`

---

## Problema 8: "Todos los comandos SQL fallan con Codigo: 1"

### Síntomas
- Conexión inicial OK
- Pero consultas fallan: `Error al consultar SQL. Codigo: 1`
- El log muestra el mismo error para todas las consultas

### Causas
- Base de datos se cerró inesperadamente
- MySQL se cayó mientras la app estaba ejecutándose
- Timeout de conexión

### Soluciones

**Reiniciar todo**:
1. Cierra la aplicación
2. Reinicia MySQL: `scripts\verificar_mysql.bat`
3. Ejecuta nuevamente: `scripts\ejecutar.bat`

**Si persiste, verificar logs de MySQL**:
- Abre `C:\xampp\mysql\data\` y busca archivos `.err`
- Revisar eventos de Windows: `Event Viewer`

---

## Checklist de Diagnóstico

Cuando tengas problemas, verifica en orden:

- [ ] ¿MySQL está corriendo? → `scripts\verificar_mysql.bat`
- [ ] ¿Se compiló correctamente? → Buscar errores en `build\compile_errors.log`
- [ ] ¿El archivo SQL existe? → `database\aerolinea.sql` debe existir
- [ ] ¿Permisos en carpeta build/? → Click derecho → Propiedades → Seguridad
- [ ] ¿Variables de entorno correctas? → `setx` o editar `config\db.ini`
- [ ] ¿Puedes conectar manualmente? → `"C:\xampp\mysql\bin\mysql.exe" -u root`
- [ ] ¿Log de conexión tiene detalles? → Revisar `logs\conexion_aerogest.log`

---

## Contacto/Próximos Pasos

Si ninguna solución funciona:

1. **Recopila información**:
   - Contenido completo de `logs/conexion_aerogest.log`
   - Salida de `scripts\verificar_mysql.bat`
   - Mensaje de error exacto de la app

2. **Verifica versiones**:
   ```batch
   "C:\xampp\mysql\bin\mysql.exe" --version
   g++ --version
   ```

3. **Considera reinstalar XAMPP**:
   - Backup de `database/aerolinea.sql`
   - Desinstala XAMPP
   - Reinstala versión estándar
   - Copia `aerolinea.sql` de vuelta
