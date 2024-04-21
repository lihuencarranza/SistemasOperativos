# shell

## Búsqueda en $PATH
Las funciones de la librería estándar de C, según _man_ están todas basadas en la syscall `execve`.
Por lo tanto, la principal diferencia es que los wrappers llaman a la misma syscall con ciertas diferencias en funcionamiento.

La syscall `execve()` recibe 3 parámetros:
- `const char *pathname`
- `char *const argv[]`
- `char *const envp[]`

Pero los wrappers toman los argumentos de manera diferente, algunos recibiendo argumentos en línea o como vectores; dando un nuevo nivel de abstracción para la recepción de parámetros que luego serán enviados correctamente a la syscall por el wrapper.

Observamos también que algunos wrappers permiten enviar menos argumentos cuando se requiere una implementación más sencilla que el wrapper se encarga de completar. (Por ejemplo, los wrappers son sufijo `-e` especifican el entorno de ejecución, pero si no se especifica, el wrapper se encarga de completar con el entorno actual).

En definitiva, los wrappers nos permiten utilizar lo que necesitemos de la syscall sin caer en detalles que pueden resultar en complicaciones indeseadas.

---
La llamada a `exec()` puede fallar y lo describe la syscall `execve()`.

En nuestra implementación, si falla se imprime error y se termina el proceso con un `exit(-1)`.

---

## Procesos en segundo plano
En nuestra implementación, cuando se detecta que un comando se tiene que ejecutar en segundo plano, se hace un `fork()` y el proceso hijo ejecuta el comando mientras que el proceso padre vuelve al prompt para devolver el control de la terminal al usuario.

---

## Flujo estándar
En el primer caso, la salida de `cat out.txt` muestra primero el error y luego el resultado de lo que salió bien

Al invertir las redirecciones, el resultado es el mismo. 

En bash, el primer caso resulta igual que en nuestra implementación.
Pero el segundo imprime primero el error y en out.txt queda solo el resultado del `ls`.

Acá vemos una diferencia entre nuestra implementación y lo que hace bash; bash presenta comportamiento diferentes para ambos casos pero la nuestra no, se comporta igual para diferente orden de redirección.

---

## Tuberías múltiples
En nuestra implementación no aparece un exit code en azul como en un comando normal. Directamente devuelve el prompt sin exit code para cualquier pipe, ya sea simple o múltiple.

---

## Variables de entorno temporarias
### ¿Por qué es necesario realizar la expansión de variables de entorno después de la llamada a `fork(2)`?

Es necesario hacerlo después de la llamada a `fork(2)` porque fork crea un nuevo proceso que es una copia exacta del proceso padre, incluyendo su entorno. Si se modificara el entorno antes de fork, esos cambios afectarían al proceso padre y a todos los procesos hijos existentes.

Al realizar la expansión de variables de entorno después de fork pero antes de exec, la shell garantiza que los cambios en el entorno solo afecten al proceso hijo que ejecutará el comando solicitado. Esto permite que el proceso padre (la shell) mantenga su propio entorno sin afectar la ejecución del comando solicitado en el proceso hijo. De esta manera, la shell puede continuar ejecutando comandos adicionales y mantener su entorno adecuadamente.

- - -
### En algunos de los wrappers de la familia de funciones de `exec(3)`, se les puede pasar un tercer argumento con nuevas variables de entorno para la ejecución de ese proceso. Supongamos que en vez de utilizar `setenv(3)` por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de `exec(3)`. 
### ¿El comportamiento resultante es el mismo que en el primer caso?

Cuando se utiliza una función de la familia `exec(3)` con el argumento de un arreglo envp para pasar nuevas variables de entorno, el comportamiento es similar pero no exactamente igual al de usar `setenv(3)` para establecer variables de entorno antes de llamar a exec. La diferencia principal radica en cómo se manejan las variables de entorno heredadas y las nuevas variables de entorno en el proceso hijo.

- Usando `exec..e()` con `envp`:

En este caso, el nuevo proceso creado por exec..e() hereda las variables de entorno del proceso padre, salvo que se especifiquen nuevas variables de entorno mediante el arreglo envp. Si se proporciona un arreglo envp, este reemplazará completamente las variables de entorno heredadas por las nuevas variables especificadas en envp.
- Usando `setenv()`:

Cuando se utiliza `setenv()` para establecer una variable de entorno, esta variable no será heredada por los procesos hijos creados posteriormente. Esto significa que si se establece una variable de entorno utilizando `setenv()` antes de llamar a `fork()`, esa variable no estará presente en los procesos hijos creados después de la llamada a `fork()`.
En resumen, al usar `exec..e()` con envp, se pueden especificar nuevas variables de entorno que reemplazarán completamente las variables de entorno heredadas. En cambio, al usar `setenv()`, las variables de entorno establecidas no se heredan por los procesos hijos creados posteriormente.

### Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.


Para que el comportamiento sea el mismo al utilizar un arreglo de variables de entorno con `exec..e()` y al utilizar `setenv()` seguido de `exec()`, se puede hacer lo siguiente
- Crear un arreglo de variables de entorno:

Este arreglo contendría tanto las variables de entorno existentes (las que se encuentran en environ) como las nuevas variables que se quieran utilizar. 
- Establecer las variables de entorno en el arreglo:

Se deben incluir tanto las variables existentes como las nuevas en el arreglo. Esto se puede hacer recorriendo environ y copiando sus valores al arreglo, y luego agregando las nuevas variables según sea necesario.
- Pasar el arreglo a la función `exec..e()`:

Al llamar a la función `exec..e()`, se le pasa este arreglo como argumento envp. Esto asegura que las variables de entorno se establezcan correctamente para el proceso que se está ejecutando.

Al seguir este enfoque, se garantiza que el comportamiento sea el mismo que al utilizar `setenv()` seguido de `exec()`, pero con la ventaja de poder establecer las variables de entorno de una manera más eficiente y organizada.


---

## Pseudo-variables
### Investigar al menos otras tres variables mágicas estándar, y describir su propósito. 

Además de `$?`, que almacena el código de salida del último comando ejecutado, existen otras variables mágicas...

- `$!` (PID del último proceso en segundo plano): Después de ejecutar un comando en segundo plano (con & al final), esta variable contiene el PID del proceso que se ejecutó en segundo plano.
```
$ sleep 10 &
$ [1] 1234
```
- `$0` (Nombre del comando o script): Esta variable almacena el nombre del comando o script que se está ejecutando actualmente.
```
$ echo "$0"
$ script.sh
```
- `$$` (PID del proceso actual): Esta variable almacena el ID de proceso (PID) del proceso actual.
```
$ echo $$
$ 1234
```

---

### Comandos built-in

---

### Historial

---

---
