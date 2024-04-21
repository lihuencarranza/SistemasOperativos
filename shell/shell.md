# shell

### Búsqueda en $PATH
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

### Procesos en segundo plano
En nuestra implementación, cuando se detecta que un comando se tiene que ejecutar en segundo plano, se hace un `fork()` y el proceso hijo ejecuta el comando mientras que el proceso padre vuelve al prompt para devolver el control de la terminal al usuario.
---

### Flujo estándar
En el primer caso, la salida de `cat out.txt` muestra primero el error y luego el resultado de lo que salió bien

Al invertir las redirecciones, el resultado es el mismo. 

En bash, el primer caso resulta igual que en nuestra implementación.
Pero el segundo imprime primero el error y en out.txt queda solo el resultado del `ls`.

Acá vemos una diferencia entre nuestra implementación y lo que hace bash; bash presenta comportamiento diferentes para ambos casos pero la nuestra no, se comporta igual para diferente orden de redirección.

---

### Tuberías múltiples
En nuestra implementación no aparece un exit code en azul como en un comando normal. Directamente devuelve el prompt sin exit code para cualquier pipe, ya sea simple o múltiple.

---

### Variables de entorno temporarias

---

### Pseudo-variables

---

### Comandos built-in

---

### Historial

---
