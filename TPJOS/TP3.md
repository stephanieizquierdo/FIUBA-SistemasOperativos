## TP3: Multitarea con desalojo

### env_return

1) al terminar un proceso su función umain() ¿dónde retoma la ejecución el kernel? Describir la secuencia de llamadas desde que termina umain() hasta que el 
kernel dispone del proceso

La secuencia de instrucciones luego de llamar a `umain` y finalizar su ejecucion consiste en llamar a la funcion `exit` de libmain. En esta funcion se llama a la
syscall `sys_env_destroy()` con paraemtro 0(indica que se esta liberando el mismo proceso que se esta ejecutando). Luego se llama a 
`syscall(SYS_env_destroy, 1, envid, 0, 0, 0, 0)` luego se llama a `env_destroy` que libera al proceso y por ultimo se llama a `sched_yield` para buscar un nuevo proceso para ejercutar utilixando round robin y se finaliza con `sched_halt`.

2)en qué cambia la función env_destroy() en este TP, respecto al TP anterior?

En el tp viejo solo se tenia un unico enviroment que corria por lo que solo destruia a ese environment y luego se devolvia al monito de JOS. En este tp tienen multiples cpus por lo que primero hay que preguntarse si el proceso a destruir esta corriendo en esta cpu o en otras. En el caso que este corriendo en esta cpu se lo libera y se llama al scheduler para buscar otro proceso para correr mientras que en el caso que este corriendo en otra cpu solo se lo marca como un proceso zombi(`ENV_DYING`) y no se lo libera para que asi la proxima vez que aparezca en el kernel, este sea liberado.  

### sys_yield
1)Leer y estudiar el código del programa user/yield.c. Cambiar la función i386_init() para lanzar tres instancias de dicho programa, y mostrar y explicar la salida de make qemu-nox.

El programa yield es un cliclo de 5 iteraciones donde se llama a `sys_yield`(quien llama al scheduler Round Robin por medio de `sched_yield`). 
Luego se hace una mdoficacion es la funcion `i386_init` y la salida de `make qemu-nox` es: 

```C SMP: CPU 0 found 1 CPU(s)
enabled interrupts: 1 2
[00000000] new env 00001000
[00000000] new env 00001001
[00000000] new env 00001002
Hello, I am environment 00001000, cpu 0
Hello, I am environment 00001001, cpu 0
Hello, I am environment 00001002, cpu 0
Back in environment 00001000, iteration 0, cpu 0
Back in environment 00001001, iteration 0, cpu 0
Back in environment 00001002, iteration 0, cpu 0
Back in environment 00001000, iteration 1, cpu 0
Back in environment 00001001, iteration 1, cpu 0
Back in environment 00001002, iteration 1, cpu 0
Back in environment 00001000, iteration 2, cpu 0
Back in environment 00001001, iteration 2, cpu 0
Back in environment 00001002, iteration 2, cpu 0
Back in environment 00001000, iteration 3, cpu 0
Back in environment 00001001, iteration 3, cpu 0
Back in environment 00001002, iteration 3, cpu 0
Back in environment 00001000, iteration 4, cpu 0
All done in environment 00001000.
[00001000] exiting gracefully
[00001000] free env 00001000
Back in environment 00001002, iteration 4, cpu 0
All done in environment 00001002.
[00001002] exiting gracefully
[00001002] free env 00001002
Back in environment 00001001, iteration 4, cpu 0
All done in environment 00001001.
[00001001] exiting gracefully
[00001001] free env 00001001
No runnable environments in the system!
```
En este test se puede ver como los procesos le entregan el poder al scheduler ( se desalojan a si mismos) y como se trata de un scheduler round robin la distribucion entre los 3 procesos va a ser justa y circular. Primero se corre el proceso 0  , luego de un tile slice se corre el proceso 1 y luego el proceso 2 (todos en sus primeras iteraciones) y esto continua hasta que se completen las 5 iteraciones de los 3 procesos.


### envid2env

1) Qué ocurre en JOS si un proceso llama a sys_env_destroy(0)?.

En esa funcion se va a destruir el proceso que este siendo ejecutado actualmente ya que se llama a `envid2env` en la cual si recibe 0, se entiende como el entorno actual.

### dumbfork

1)Si una página no es modificable en el padre ¿lo es en el hijo? En otras palabras: ¿se preserva, en el hijo, el flag de solo-lectura en las páginas copiadas?

El flag de solo-lectura no se preserva ya que `sys_page_alloc()`(en `duppage()`) se llama con permiso de escritura.  Para saber si una direccion es modificable por el proceso se pueden usar los arreglos auxiliares de solo lectura uvpd (que guarda las direcciones fisicas de las tablas de paginas del proceso, asi como los permisos) y uvpt (que si se le indica el numero de una pagina especifica, a partir de una direccion virtual, permite obtener una entrada de una tabla de pagina especifica, asi como los permisos de la misma). 

2) Mostrar, con código en espacio de usuario, cómo podría dumbfork() verificar si una dirección en el padre es de solo lectura, de tal manera que pudiera pasar como tercer parámetro a duppage() un booleano llamado readonly que indicase si la página es modificable o no

```C
pde_t pde = uvpd[PDX(addr)];
if ((pde & PTE_P) != PTE_P){
	  readonly = 1;
}
pte_t pte = uvpt[PGNUM(addr)];
if (pte & (PTE_P | PTE_U | PTE_W) != (PTE_P | PTE_U | PTE_W)){
    readonly = 1;
} 
```
3)Se pide mostrar una versión en el que se implemente la misma funcionalidad readonly, pero sin usar en ningún caso más de tres llamadas al sistema.

Lo que se hace es reservar una pagina vacia en una direccion auxiliar del proceso padre y mapear la direccion virtual del hijo como solo lectura a esa pagina vacia del padre. Asi logramos mapear una pagina que en principio tiene permiso de escritura como una de solo lectura para los ojos del proceso hijo: 

```C
int perm = PTE_P | PTE_U;
if ((uvpd[PDX((uintptr_t)addr)] & PTE_P) == PTE_P && (uvpt[PGNUM((uintptr_t)addr)] & (PTE_P | PTE_U | PTE_W)) == (PTE_P | PTE_U | PTE_W))
    perm |= PTE_W;
if ((r = sys_page_alloc(0, UTEMP, PTE_P|PTE_U|PTE_W)) < 0)
    panic("sys_page_alloc: %e", r);
if ((r = sys_page_map(0, UTEMP, dstenv, addr, perm)) < 0)
    panic("sys_page_map: %e", r);
memmove(UTEMP, addr, PGSIZE);
if ((r = sys_page_unmap(0, UTEMP)) < 0)
    panic("sys_page_unmap: %e", r);

```

### multicore_init

1. ¿Qué código copia, y a dónde, la siguiente línea de la función boot_aps()?
`memmove(code, mpentry_start, mpentry_end - mpentry_start);`

Antes de esa linea, se setea code como `KADDR(MPENTRY_PADDR);` entonces al ejecutar `memmove`, lo que hacemos es copiar en la dirección virtual code, y por lo tanto en la dirección física MPENTRY_PADDR, el contenido de la dirección mpentry_start hasta el final de esta (mpentry_end - mpentry_start). En la dirección mpentry_start se encuentran las instrucciones de mpentry.S, la ejecución que debe realizar un AP al cargarse.

2. ¿Para qué se usa la variable global mpentry_kstack? ¿Qué ocurriría si el espacio para este stack se reservara en el archivo kern/mpentry.S, de manera similar a bootstack en el archivo kern/entry.S

La variable global `mpentry_kstack` apunta a la direccion mas alta del stack kernel de cada CPU. En entry.S setea el stack pointer de cada CPU. Si este stack se reservara en el archivo kern/mpentry.S, esta stack solo se reservaría una vez y todos los AP's intentarían utilizar ese mismo stack. Entonces no sera lo mismo hacerlo en mpentry.S a no ser que se comparta el mismo stack para todas las CPUs.

3. En el archivo kern/mpentry.S se puede leer:
```
 # We cannot use kern_pgdir yet because we are still
 # running at a low EIP.
 movl $(RELOC(entry_pgdir)), %eax
```
¿Qué valor tendrá el registro %eip cuando se ejecute esa línea? Responder con redondeo a 12 bits, justificando desde qué región de memoria se está ejecutando este código.

El registro %eip apuntará a la región de memoria 0x7000 (MPENTRY_PADDR), ya que todo el bloque de código de mpentry.S se mapeó allí y la linea pertenece al código entry point de un AP y el código fue mapeado a la direccion MPENTRY_PADD con la función memmove() en boot_aps().

### ipc_recv

1. Un proceso podría intentar enviar el valor númerico -E_INVAL vía ipc_send(). ¿Cómo es posible distinguir si es un error, o no?
```C
envid_t src = -1;
int r = ipc_recv(&src, 0, NULL);

if (r < 0)
  if (/* ??? */)
    puts("Hubo error.");
  else
    puts("Valor negativo correcto.")
```

Si ocurre un error en la llamada ipc_recv quiere decir que en el wrapper falló la ejecución de la syscall sys_ipc_recv. Como el argumento from_env_store es distinto de NULL, en caso de un error la syscall guardará en esa variable el valor 0. Entonces lo resolvemos con el siguiente código:
```C
// Versión A
envid_t src = -1;
int r = ipc_recv(&src, 0, NULL);

if (r < 0)
  if (!src)
     puts("Hubo error.");
else
  puts("Valor negativo correcto.")
```

### sys_ipc_try_send

1. Explicar cómo se podría implementar una función sys_ipc_send() (con los mismos parámetros que sys_ipc_try_send()) que sea bloqueante, es decir, que si un proceso A la usa para enviar un mensaje a B, pero B no está esperando un mensaje, el proceso A sea puesto en estado ENV_NOT_RUNNABLE, y despertado una vez B llame a ipc_recv() (cuya firma no debe ser cambiada).

Cuando un proceso intenta enviar un mensaje a otro proceso que no está esperando mensajes, se lo pondrá al mismo en espera hasta que el proceso destinatario del mensaje se predisponga a recibir un mensaje. En este caso, el estado del proceso emisor deberá setearse como "not runnable" y agregarse el mismo a un arreglo de procesos que aguardan ser "despertados". Cuando el proceso destinatario del mensaje en cuestión se predisponga a recibir un mensaje, deberá despertar a todos los procesos del arreglo y marcarlos como "runnable". Esto se hace justamente para evitar deadlocks
