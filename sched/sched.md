# Scheduler

##  Context Switch
*previo a context_switch*
![image](https://github.com/fiubatps/sisop_2024a_g28/assets/102101194/aa6972b3-a3cd-44a0-9f9d-ae5a26ad38b6)

*luego de llamar a context_switch*
![image](https://github.com/fiubatps/sisop_2024a_g28/assets/102101194/81857834-6a6d-4158-aa59-1ce9b4b598f1)

*como el primer valor es la dirección de retorno, la salteamos con +4*
![image](https://github.com/fiubatps/sisop_2024a_g28/assets/102101194/a624a9a6-8f62-4174-8494-ab588fbc1d0b)

*luego de hacer que el stack apunte a donde debería, hacemos popal y el stack queda:*
![image](https://github.com/fiubatps/sisop_2024a_g28/assets/102101194/497b2706-e590-4e0b-9cf5-733a3f7a00ec)

*luego popeamos %es y %ds y el stack:*
![image](https://github.com/fiubatps/sisop_2024a_g28/assets/102101194/554e769a-cfee-47ab-82d5-d78126ecd624)

*saltemos los próximos 2 valores porque no nos interesa guardarlos:*
![image](https://github.com/fiubatps/sisop_2024a_g28/assets/102101194/3799a504-7ba7-4908-8d77-cd2cea032cb8)

*llamamos a iret y los registros quedan:*

![image](https://github.com/fiubatps/sisop_2024a_g28/assets/102101194/c1a112e9-3aeb-4f39-a9b7-75915c49e65d)


## Scheduler con prioridades
### Lógica de Prioridades
#### Inicialización de Prioridades
Todos los procesos en el sistema comienzan con una prioridad inicial de 5 seteada en `kern/env.c` si se compila con el flag `USER_PR=1`. La elección de este número en particular es arbitraria y puede modificarse en la constante `MAX_PRIORITY_LEVEL`. Esta medida asegura que no haya preferencias hacia ningún proceso desde el inicio, permitiendo una distribución equitativa de tiempo de CPU entre todos los procesos. 

#### Mecanismo de Reducción de Prioridades
Cada vez que un proceso de alta prioridad es seleccionado y ejecutado, su prioridad se reduce en un nivel. Esta reducción progresiva previene que un proceso con alta prioridad monopolice el CPU indefinidamente. La reducción se detiene cuando la prioridad alcanza el nivel mínimo permitido (`MIN_PRIORITY_LEVEL`), asegurando que el proceso siga teniendo alguna prioridad residual.

#### Incremento Periódico de Prioridades
Para mitigar el riesgo de "starvation" (donde los procesos de menor prioridad nunca son ejecutados), implementamos un mecanismo de incremento periódico. Cada 30 invocaciones del scheduler, las prioridades de todos los procesos `ENV_RUNNABLE` se restablecen al nivel máximo (`MAX_PRIORITY_LEVEL`). Esto garantiza que todos los procesos eventualmente reciban atención, independientemente de su prioridad actual.

#### Selección y Ejecución de Procesos
El scheduler busca entre los procesos `ENV_RUNNABLE` aquel con la mayor prioridad. Si existen múltiples procesos con la misma prioridad, se selecciona el primero encontrado. Esta selección asegura que siempre se ejecute el proceso más crítico disponible. Si no se encuentra ningún proceso `ENV_RUNNABLE`, pero el proceso actual (`curenv`) está en estado `ENV_RUNNING`, se continúa ejecutando el proceso actual.

### Implementación
La función `void reduce_priority(struct Env env)` disminuye la prioridad de un proceso, respetando el límite mínimo de prioridad.
Para el boosteo, utilizamos `boost_priority()`. Esta función eleva la prioridad de todos los procesos `ENV_RUNNABLE` al nivel máximo.

