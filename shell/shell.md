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
### Investigar el significado de 2>&1, explicar cómo funciona su forma general. Mostrar qué sucede con la salida de cat out.txt en el ejemplo. Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, 2>&1 >out.txt). ¿Cambió algo? Compararlo con el comportamiento en bash(1).

La redirección 2>&1 en un shell Unix/Linux combina el flujo de errores estándar (stderr, descriptor de archivo 2) con el flujo de salida estándar (stdout, descriptor de archivo 1), de modo que ambos fluyen al mismo destino. En otras palabras, redirige el flujo de errores estándar hacia el mismo lugar que el flujo de salida estándar.

Para entender cómo funciona, es útil conocer algunos conceptos básicos sobre los descriptores de archivo en Unix. Cada programa en Unix tiene tres descriptores de archivo abiertos por defecto:

- 0: stdin (entrada estándar)
- 1: stdout (salida estándar)
- 2: stderr (error estándar)

Por lo tanto, cuando se ejecuta 2>&1, se está diciendo que el descriptor de archivo 2 (stderr) debe apuntar al mismo lugar que el descriptor de archivo 1 (stdout). Esto significa que ambos flujos de datos ahora van al mismo lugar.

```
$ ls -C /home /noexiste >out.txt 2>&1
$ cat out.txt
/home
user
ls: cannot access '/noexiste': No such file or directory
```
- `ls -C /home /noexiste` es el comando que se ejecuta.
- `>out.txt` redirige la salida estándar al archivo `out.txt`.
- `>2>&1` redirige el flujo de errores estándar al mismo lugar que la salida estándar, que es el archivo `out.txt`.

Por lo tanto, si se observa el contenido de `out.txt`, debería contener la salida del comando `ls -C /home /noexiste`, incluido cualquier mensaje de error que genere.

En segundo lugar, al invertir el orden de las redirecciones `(2>&1 >out.txt)`, el comportamiento cambia ligeramente:
```
$ ls -C /home /noexiste 2>&1 >out.txt
```
En este caso, primero se redirige el flujo de errores estándar `(2>&1)` hacia el mismo destino que la salida estándar (que en este caso sigue siendo la consola, ya que aún no se ha redirigido la salida estándar). Luego, se redirige la salida estándar `(>out.txt)` al archivo out.txt.

Por lo tanto, en este caso, la salida estándar se envía al archivo out.txt, mientras que la salida de errores (incluidos los mensajes de error del comando `ls`) se envía a la consola, ya que se redirigió antes de la salida estándar.


---

## Tuberías múltiples
### Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe ¿Cambia en algo? ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.
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

## Comandos built-in
### ¿Entre `cd` y `pwd`, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

En el caso de `cd`, no necesariamente necesita ser un built-in, ya que la función `chdir()` de la biblioteca estándar de C permite cambiar el directorio de trabajo de un proceso. Sin embargo, si `cd` no fuera un built-in, cambiaría el directorio de trabajo solo para el proceso hijo que lo ejecuta, no para el proceso shell principal. Por lo tanto, para que `cd` tenga el efecto deseado de cambiar el directorio de trabajo del proceso shell, necesita ser un built-in.

En cambio, `pwd` es un caso diferente. `pwd` simplemente imprime el directorio de trabajo actual del proceso, que puede obtenerse utilizando la función `getcwd()` de la biblioteca estándar de C. Dado que no modifica el entorno o el estado del proceso shell, `pwd` no necesita ser un built-in. De hecho, muchos shells implementan `pwd` como un comando externo en lugar de un built-in.

La razón principal para implementar comandos como `cd` y `pwd` como built-ins es la conveniencia y eficiencia. Al ser built-ins, estos comandos se ejecutan en el mismo proceso que la shell, lo que les permite modificar directamente el entorno y el estado de la shell, en lugar de tener que crear un proceso hijo separado para ejecutar un comando externo. Esto mejora la eficiencia y evita posibles problemas con la sincronización de los procesos.


---

## Segundo plano avanzado
### Mecanismo utilizado

Se utiliza el mecanismo de señales para manejar la terminación de procesos en segundo plano de manera inmediata. Esto se logra mediante la captura y el manejo de la señal `SIGCHLD`, que se genera cuando un proceso hijo termina.
Cuando un proceso hijo finaliza, se genera la señal `SIGCHLD`, lo que desencadena la ejecución de un manejador de señales específico (`sigchld_handler()` en este caso). 

El manejo de la señal `SIGCHLD` en la shell implica llamar a `waitpid()` con la bandera WNOHANG. Esto permite a la shell verificar si algún proceso hijo ha terminado sin bloquear la ejecución. Si `waitpid()` devuelve un PID de un proceso hijo que ha terminado, la shell puede realizar acciones como mostrar un mensaje al usuario o actualizar su estado interno para reflejar la finalización del proceso.

Para registrar y configurar el manejador de señales `sigchld_handler()`, se utiliza la función `set_signal_handlers()`. En esta función, se emplea `sigaction()` para asociar el manejador de señales con la señal `SIGCHLD`. Además, se especifica el uso de `SA_RESTART`, lo que garantiza que ciertas llamadas al sistema se reinicien automáticamente si se interrumpen debido a la señal.

La inicialización de la shell (`init_shell()`) incluye la llamada a `set_signal_handlers()`, lo que asegura que los manejadores de señales estén configurados y listos para manejar señales tan pronto como la shell comience a ejecutarse. Esto garantiza un manejo eficiente de la terminación de procesos en segundo plano, mejorando la capacidad de respuesta de la shell y la experiencia del usuario al interactuar con procesos en segundo plano.

### ¿Por qué es necesario el uso de señales?

Es necesario para manejar los procesos en segundo plano de manera eficiente y notificar su finalización inmediatamente. En este trabajo, utilizamos la senal `SIGCHLD` que se genera cuando un proceso hijo termina, ya sea normalmente o debido a un error. Utilizando la función `sigaction(2)` podemos configurar un manejador de señales, también conocido como handler, para la señal `SIGCHLD`.

El mecanismo completo utilizado implica configurar el handler de `SIGCHLD` para que, cuando se reciba esta señal, se ejecute una función que liberará los recursos del proceso hijo finalizado y notificará al usuario de la shell sobre la finalización del proceso en segundo plano. Además, para evitar conflictos con los procesos en primer plano, es necesario asegurarse de que todos los procesos en segundo plano estén en el mismo grupo de procesos y utilizar un valor numérico específico en la llamada a `waitpid(2)` en el handler para restringir la espera solo a los procesos en segundo plano.

El uso de señales es necesario en este caso porque permite a la shell recibir notificaciones asincrónicas sobre eventos importantes, como la finalización de un proceso hijo. Sin señales, la shell tendría que estar constantemente verificando manualmente si algún proceso hijo ha finalizado, lo cual sería ineficiente y consumiría recursos innecesarios. Con señales, la shell puede continuar ejecutando otras tareas y ser notificada de inmediato cuando un proceso hijo haya terminado, permitiendo un manejo más eficiente de los procesos en segundo plano.

---

---
