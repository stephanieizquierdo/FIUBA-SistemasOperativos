TP2: Procesos de usuario
========================

env_alloc
---------
1. El env_id se genera en la llamada de env_alloc para cada envoriment ya que en mem_init la lista de enviroments es inicializada toda en cero. La forma de generarlos es la siguente:
    1. se agarra el primer env libre cuyo env_id es cero
    2. luego se genera una variable auxiliar (generation) que es la suma del env_id y una constante 0x00001000 ( 1 << ENVGENSHIFT) equivale a 12)
    3. a la variable se le niegan los primeros 10 bits con (~(NENV - 1)) para poder identificar de forma unica a cada uno de los NENV (1024)
    4. Si al obtener generation, este da negativo, directamente se utiliza el valor de 12 ( 1 << ENVGENSHIFT)
    5. Por ultimo se define env_id como un or entre la variable generation y la diferencia entre el puntero al primer env libre y el primer env libre de la lista        de enviroments. 
   
   Por lo que los identificadores para los primeros 5 procesos seran: `0x00001000`, `0x00001001`, `0x00001002`, `0x00001003`, `0x00001004`.
   
2. Como los procesos ya fueron lanzados, todos tiene el env_id distinto de cero, por lo que el primer proceso lanzado tendra un identificador que es la suma entre el generador (0x00001000) y el offset del 630 (0x00000276) ya que es ese proceso destruido se encontrara en la lista de env libres obteniendo el id: 0x00001276
Una vez que se muera ese proceso y se vuelva a lanzar, siguiende el mismo procedimiento, el nuevo id sera la suma entre el id previo (ahora 0x00001276) y el generador(0x00001000) y siguiendo los pasos de la pregunta anterior se obtendra para los primeros 5 procesos:

* 0x00001276
* 0x00002276
* 0x00003276
* 0x00004276
* 0x00005276

...


env_init_percpu
---------------

1. La funcion lgdt escribe 6 bytes en la global description table.(GDT) 
2. Esos bytes corresponden al tamaño de la tabla GDT, que son 2 bytes, y a la direccion de tabla, los otros 4 bytes. 

...


env_pop_tf
----------
1. el tope de la pila justo antes popal: 

Antes del  popal se hace "movl %0, %%esp" %esp es el el puntero actual del stack. y el movl con el 0 y el esp significa: pone el argumento 0 en el registro del puntero actual del stack. el argumento 0 en este caso seria tf, entonces en el tope de la pila justo antes del popal tenemos que apunta al TrapFrame tf.

2. el tope de la pila justo antes iret:

"popl %%es" y "popl %%ds" sacan de la pila los elementos del trapframe tf_es (y el padding1 debido a que el popl saca 32 bits) y tf_ds (y el padding 2).  Luego la instruccion "addl $0x8,%%esp" mueve el t"ope de la pila para que se saltee los elementos tf_trapno" y "tf_err" del struct Trapframe, entonces justo antes de ejecutar iret la pila (el registro esp) apunta al elemento tf_eip del struct Trapframe recibido como parametro.

3. el tercer elemento de la pila justo antes de iret
por los mismos razonamientos anteriores, el 3er elemento de la pila es tf_eflags.

¿Cómo determina la CPU (en x86) si hay un cambio de ring (nivel de privilegio)? Ayuda: Responder antes en qué lugar exacto guarda x86 el nivel de privilegio actual. ¿Cuántos bits almacenan ese privilegio?

Se guarda el nivel actual de privilegio (CPL) en los dos bits menos significativos del registro CS, por lo que para determinar un cambio de ring se compara con el DPL (descriptor privilege level) del segmento accedido. Si DPL es mayor a CPL, se produjo un cambio de nivel.
...


gdb_hello
---------
1. Poner un breakpoint en env_pop_tf() y continuar la ejecución hasta allí.

```console
(gdb) b env_pop_tf
Breakpoint 1 at 0xf0102fc4: file kern/env.c, line 478.
(gdb) c
Continuing.
The target architecture is assumed to be i386
=> 0xf0102fc4 <env_pop_tf>:	endbr32 

Breakpoint 1, env_pop_tf (tf=0xf01c8000) at kern/env.c:478
478	{
```

2. En QEMU, entrar en modo monitor (Ctrl-a c), y mostrar las cinco primeras líneas del comando info registers.

``` console
qemu) info registers
EAX=000003fd EBX=f0100184 ECX=0000003c EDX=000003fd
ESI=00000000 EDI=00000001 EBP=effffe90 ESP=effffe84
EIP=f0100184 EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```

3. De vuelta a GDB, imprimir el valor del argumento tf:

``` console
(gdb) p tf
$1 = (struct Trapframe *) 0xf01c8000
(gdb) 
```

4. Imprimir, con x/Nx tf tantos enteros como haya en el struct Trapframe donde N = sizeof(Trapframe) / sizeof(int).

```
0xf01c8000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c8010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c8020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01c8030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01c8040:	0x00000023

```

5. Avanzar hasta justo después del movl ...,%esp, usando si M para ejecutar tantas instrucciones como sea necesario en un solo paso:
```
(gdb) disas
Dump of assembler code for function env_pop_tf:
=> 0xf0102fc4 <+0>:	endbr32 
   0xf0102fc8 <+4>:	push   %ebp
   0xf0102fc9 <+5>:	mov    %esp,%ebp
   0xf0102fcb <+7>:	sub    $0xc,%esp
   0xf0102fce <+10>:	mov    0x8(%ebp),%esp
   0xf0102fd1 <+13>:	popa   
   0xf0102fd2 <+14>:	pop    %es
   0xf0102fd3 <+15>:	pop    %ds
   0xf0102fd4 <+16>:	add    $0x8,%esp
   0xf0102fd7 <+19>:	iret   
   0xf0102fd8 <+20>:	push   $0xf0105b04
   0xf0102fdd <+25>:	push   $0x1e8
   0xf0102fe2 <+30>:	push   $0xf0105aa2
   0xf0102fe7 <+35>:	call   0xf01000ad <_panic>
End of assembler dump.
(gdb) si 5
=> 0xf0102fd1 <env_pop_tf+13>:	popa   
0xf0102fd1 in env_pop_tf (tf=0x0) at kern/env.c:479
479		asm volatile("\tmovl %0,%%esp\n"
(gdb) disas
Dump of assembler code for function env_pop_tf:
   0xf0102fc4 <+0>:	endbr32 
   0xf0102fc8 <+4>:	push   %ebp
   0xf0102fc9 <+5>:	mov    %esp,%ebp
   0xf0102fcb <+7>:	sub    $0xc,%esp
   0xf0102fce <+10>:	mov    0x8(%ebp),%esp
=> 0xf0102fd1 <+13>:	popa   
   0xf0102fd2 <+14>:	pop    %es
   0xf0102fd3 <+15>:	pop    %ds
   0xf0102fd4 <+16>:	add    $0x8,%esp
   0xf0102fd7 <+19>:	iret   
   0xf0102fd8 <+20>:	push   $0xf0105b04
   0xf0102fdd <+25>:	push   $0x1e8
   0xf0102fe2 <+30>:	push   $0xf0105aa2
   0xf0102fe7 <+35>:	call   0xf01000ad <_panic>
End of assembler dump.
```
6.Comprobar, con x/Nx $sp que los contenidos son los mismos que tf (donde N es el tamaño de tf).
```
(gdb) x/17x $sp
0xf01c8000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c8010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c8020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01c8030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01c8040:	0x00000023
(gdb) 
```
Si, es lo mismo

7. Describir cada uno de los valores. Para los valores no nulos, se debe indicar dónde se configuró inicialmente el valor, y qué representa.

Los primeros ocho 0x00000000 representan los primeros ocho registros del struct Trapframe y son los registros de proposito general: %edi, %esi, %ebp, %oesp, %ebx, %edx, %ecx, %eax. Se encuentran en 0 porque la ejecución de este env aún no comenzó por lo que en ningun momento guardó nada en registros de proposito general.

Luego tenemos que en los siguientes dos y en el ultimo hay 0x00000023. Estos valores representan a los registros %es, %ds, %ss respectivamente, son registros de segmento y fueron cargados con este valor en env_alloc con "GD_UD | 3", para que estos segmentos tengan la referencia a el user data segment y ademas se le agrega un tres en los dos bits mas bajos, el cual quiere significa privilegios de user mode.

Los dos siguientes, a los primeros 0x00000023, se encuentran en cero, siendo estos el %trapno y el %err. Se encuentran en cero porque no ocurrió ninguna interupción ya que no se ejecuto env.

Luego tenemos un 0x00800020, el cual representa al %eip. Este numero es el entry point del env, la primera instrucción que va a ejecutar cuando inicie. El mismo fue cargado durante load_icode, y el valor se encontraba contenido dentro del ELF.

En el siguiente encontramos un 0x0000001b que reprensenta al %cs. El code segment fue cargado durante env_alloc con "GD_UT | 3", para que tenga referencia al segmento user text y con privilegios de usuario.

A continuecion tenemos un 0x00000000 de %eflags, que se envuentra en 0 ya que todavia no se ejecuto env.

Y en el penultimo tenemos un 0xeebfe000, el cual representa al %esp. El mismo fue cargado durante env_alloc apuntando al user stack top, ya que como se va a empezar a usar el stack debe iniciar desde el tope del mismo

8. Continuar hasta la instrucción iret, sin llegar a ejecutarla. Mostrar en este punto, de nuevo, las cinco primeras líneas de info registers en el monitor de QEMU. Explicar los cambios producidos.

```
(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01c8028
EIP=f0102fd4 EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]

```

Podemos ver que se actualizaron los valores de los registros de propósito general a los valores contenidos en el trapframe. El instruction pointer (EIP) se vió modificado debido a que se ejecutaron algunas líneas de código. Ademas se ve un cambio del DPL, de 0 (kernel mode) a 3 (user mode).

9. Ejecutar la instrucción iret. En ese momento se ha realizado el cambio de contexto y los símbolos del kernel ya no son válidos.

``` console
(gdb) disas
Dump of assembler code for function env_pop_tf:
   0xf0102fc4 <+0>:	endbr32 
   0xf0102fc8 <+4>:	push   %ebp
   0xf0102fc9 <+5>:	mov    %esp,%ebp
   0xf0102fcb <+7>:	sub    $0xc,%esp
   0xf0102fce <+10>:	mov    0x8(%ebp),%esp
   0xf0102fd1 <+13>:	popa   
   0xf0102fd2 <+14>:	pop    %es
   0xf0102fd3 <+15>:	pop    %ds
   0xf0102fd4 <+16>:	add    $0x8,%esp
=> 0xf0102fd7 <+19>:	iret   
   0xf0102fd8 <+20>:	push   $0xf0105b04
   0xf0102fdd <+25>:	push   $0x1e8
   0xf0102fe2 <+30>:	push   $0xf0105aa2
   0xf0102fe7 <+35>:	call   0xf01000ad <_panic>
End of assembler dump.
(gdb) si 1
=> 0x800020:	cmp    $0xeebfe000,%esp
0x00800020 in ?? ()
(gdb) p $pc
$2 = (void (*)()) 0x800020
```
``` console
(gdb) add-symbol-file obj/user/hello 0x800020
add symbol table from file "obj/user/hello" at
	.text_addr = 0x800020
(y or n) y
Reading symbols from obj/user/hello...
(gdb) p $pc
$3 = (void (*)()) 0x800020 <_start>
(gdb) 

```

Mostrar una última vez la salida de info registers en QEMU, y explicar los cambios producidos.

``` console
(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=eebfe000
EIP=00800020 EFL=00000002 [-------] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]

```
Ahora la CPU tiene los datos del tf de env. Vemos que se actualizo el eip por ende esta por ejecutar desde el entry de la cpu, por ende se cambio de contexto.

10. Poner un breakpoint temporal (tbreak, se aplica una sola vez) en la función syscall() y explicar qué ocurre justo tras ejecutar la instrucción int $0x30. Usar, de ser necesario, el monitor de QEMU.

```
=> 0x800a47 <syscall+29>:	int    $0x30
0x00800a47	23		asm volatile("int %1\n"
(gdb) si
=> 0xf0103952 <trap_48+2>:	push   $0x30
0xf0103952 in trap_48 () at kern/trapentry.S:71
71	TRAPHANDLER_NOEC(trap_48, T_SYSCALL)
```

monitor qemu, info registers:
```
(qemu) info registers
EAX=00000000 EBX=00000000 ECX=0000000d EDX=eebfde88
ESI=00000000 EDI=00000000 EBP=eebfde40 ESP=efffffe8
EIP=f0103952 EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]

```

Podemos ver que luego de la interrupción se efectuo un cambio de contexto y que la proxima instrucción a ejecutar esta dentro del codigo del kernel.
...

kern_idt
-------- 
1. Para poder determinar si usar TRAPHANDLER o TRAPHANDLER_NOEC nos basamos en la tabla de 5-1 de Intel® 64 and IA-32 Architectures Software Developer’s Manual en la parte de Error code. Si la interrupcion devuleve un codigo de error, este es pusheado por la cpu al stack por lo que se usaria TRAPHANDLER mientras que si no devuelve un codigo de error se debe usar TRAPHANDLER_NOEC ya que este pushea un 0 extra para rellenar la sección del struct trapframe correspondiente al codigo de error.
Si solo se usaria la macro TRAPHANDLER habria que tener cuidado en que interrupciones rellenar con 0 en el stack en el atributo tf_err para no desfazar el acceso a los registros. 
2. El parametro isTrap indica si se pueden anidar o no las interrupciones. Un motivo por el que  conviene no permitir las interrupciones anidadas durante una syscall es la simplicidad en el código, pero el no permitir que se aniden las interrupciones puede causar que se pierdan algunas syscalls.
3. Cuanso se ejecuta el programa la interrupcion que se genera es de General Protection en vez de Page fault que es la que uno esperaria ya que en el programa se llama a la interrupcion 14 que corresponde a page fault en el array de idt. Esto sucede porque el privigenieo necesario para lanzar esa interrupcion es de 0 (kernel) pero esta implementatda de forma tal que de que se lance en modo usuario( nivel de privilegio 3 )(todavia no se implemento como debe responder el kernel a un page fault) por lo que no se detecta y termina lanzanado la interrupcion de General Proteccion. 
```
Booting from Hard Disk..6828 decimal is 15254 octal!
Physical memory: 131072K available, base = 640K, extended = 130432K
check_page_alloc() succeeded!
check_page() succeeded!
check_kern_pgdir() succeeded!
check_page_installed_pgdir() succeeded!
[00000000] new env 00001000
Incoming TRAP frame at 0xefffffbc
TRAP frame at 0xf01c8000
  edi  0x00000000
  esi  0x00000000
  ebp  0xeebfdff0
  oesp 0xefffffdc
  ebx  0x00000000
  edx  0x00000000
  ecx  0x00000000
  eax  0x00000000
  es   0x----0023
  ds   0x----0023
  trap 0x0000000d General Protection
  err  0x00000072
  eip  0x00800037
  cs   0x----001b
  flag 0x00000082
  esp  0xeebfdfd4
  ss   0x----0023
[00001000] free env 00001000
Destroyed the only environment - nothing more to do!
```


user_evilhello
--------------
1. La diferenmcia se encuentra en que el programa original de evilhello.c cuando llama a sys_cputs le pasa la direccion de memoria 0xf010000c sin modificar mientras que el programa modificado lo que haces es cambiar el puntero(desreferencia y luego pasa la referencia). Esto de asignar la variable first a entry permite engañar al sistema operativo por medio de un swap y asi poder acceder a una instruccion privilegiada.
2. Por medio del swap de direcciones se logra imprimir el entry point del kernel con una cadena, cosa que no sucedia en el evilhello.c original. Para que esto no suceda se debera implementar una verificacion en user_mem_check ya que se estaria accedieidno auna direccion privilegiada para el usuario y esto no seria correcto, por lo que se debera prohibir el acceso a ella.
3. 

....
