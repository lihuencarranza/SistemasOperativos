# fisop-fs

## Decisiones de diseño

### Las estructuras en memoria

Desarrollamos un sistema de archivos basado en dos estructuras: `inodos`, donde cada archivo o directorio es representado por una estructura específica; un `super bloque`, el cual contiene todos los inodos del filesystem, un bitmap para gestionar su disponibilidad y la cantidad máxima de inodos.

La estructura del `super_block_t` está compuesta de la siguiente manera:
- Inodos: Un superblock contiene un vector de inodos (`inodes`), que representa todos los archivos y directorios en el sistema de archivos.
- Bitmap de Inodos: El superblock también incluye un bitmap (`bitmap_inodos`) que se utiliza para realizar un seguimiento de los inodos en uso y los inodos libres. Este bitmap facilita la asignación y liberación de inodos de manera eficiente.
- Máximo Número de Inodos: La estructura superblock define el número máximo de inodos que puede contener el sistema de archivos (`MAX_INODES`), lo que ayuda a gestionar los recursos y prevenir desbordamientos.

![image](https://github.com/fiubatps/sisop_2024a_g28/assets/86395729/c4947432-770d-4dcb-b65b-9c28a724d743)



![image](https://github.com/fiubatps/sisop_2024a_g28/assets/86395729/005fd3ed-cba3-453c-9925-f7e6e5253053)


Un `inodo_t` es ls estructura que almacena la metadata asociada a un archivo o directorio. Cada inodo incluye la siguiente información clave:
- Tamaño (`file_size`): Representa el tamaño del archivo. Para los directorios, se estableció que el tamaño sea 0 para simplificar la implementación.
- Metadatos: tamaño del archivo (`file_size`), identificadores de usuario y grupo (`uid, gid`), tipo de archivo (`type`), número de enlaces (`nlink`), y permisos (`mode`).
- Tiempos: Incluye tiempos de acceso (`atime`), modificación (`mtime`) y cambio (`ctime`) para realizar un seguimiento de las operaciones realizadas en el archivo.
- Rutas: Almacena el nombre del archivo o directorio (`file_name`), el path del directorio que lo contiene (`file_parent`) y el path completo (`file_path`). En el caso del root, se establece que no tiene padre con la constante `NO_PARENT` ("") y la ruta con la constante `ROOT` ("/").
- Contenido: Almacena el contenido del archivo. Los directorios no utilizan este campo.
  
![image](https://github.com/fiubatps/sisop_2024a_g28/assets/86395729/fa49cb5a-3aa7-4fb1-84e8-b3147e7c20ca)



### Cómo el sistema de archivos encuentra un archivo específico dado un path

En primer lugar, verifica que el `path` no es igual a la raíz (`ROOT`). Luego se utiliza la funcion `get_last_element()` para extraer el último componente del `path`, el cual corresponde al nombre del archivo o directorio que se busca. Se recorren todos los inodos del super bloque verificando si el inodo está ocupado y si el nombre del archivo (`file_name`) coincide con el nombre extraído. Sie encuentra el inodo que cumple con los criterios, devuelve su índice. De lo contrario, devuelve `ERROR` (una constante definida por nosotros con valos -1).

###  Persistencia en disco

El super bloque y sus inodos se guardan en un archivo en el disco llamado `fs.fisopfs`. La función `create_root` inicializa el sistema de archivos y crea el directorio raíz:
La función `fisopfs_destroy` se encarga de escribir el contenido del super bloque en el archivo de sistema de archivos en disco. Esto se realiza cuando se destruye el sistema de archivos, así nos aseguramos que todos los datos actuales se guardan.
La función `fisopfs_flush` sincroniza los datos del sistema de archivos con el disco llamando a `fisopfs_destroy` para asegurarse de que todos los cambios se escriban en el archivo

## Correr el fs y pruebas
### Montar el fs y utilizarlo
Para correr el filesystem es necesario correrlo en modo debug ya que sino no queda corriendo y no se puede utilizar.
Esto se logra con `./fisopfs -f <directorio_de_mount>`

Con eso corriendo, se puede utilizar otra instancia de la terminal para enviar comandos como `mkdir`, `ls`, `echo`, `cat`, etc. 

### Pruebas funcionales
Para probar el filesystem desarrollamos un script en python que utiliza las operaciones del sistema operativo para verificar su correcto funcionamiento.
El script funciona con un mount point en un directorio llamado "prueba". Si se quiere cambiar el nombre, hay que cambiar la primera línea del script.
Teniendo el fs corriendo como se indica en el apartado anterior, se puede utilizar `python3 test_funcional.py` para correr las pruebas de integración.


## CHALLENGE

Las pruebas se ejecutan de la mismas manera. 

``` bash
make
mkdir prueba
./fisopfs -f prueba
```

En otra terminal ejecutar:
`python3 test_funcional.py`
