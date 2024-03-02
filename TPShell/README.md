# lab-shell

Repositorio para el esqueleto del [lab shell](https://fisop.github.io/7508/lab/shell) del curso Mendez-Fresia de **Sistemas Operativos (7508) - FIUBA**

## Compilar y ejecutar

```bash
$ make run
```

## Linter

```bash
$ make format
```

## Respuestas teóricas

### Búsqueda en $PATH

_¿Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?_

La syscall execve es proporcionada por el sistema operativo, que reemplaza la imagen del proceso actual por la del programa ejecutable, argumentos y variables de entorno pasadas por parámetro. Los wrappers de la librería estándar de C ejecutan internamente execve pero proporcionan funcionalidades adicionales, ver https://man7.org/linux/man-pages/man3/exec.3.html donde especifica funcionalidades segun cada letra que viene despues de 'exec'

_¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?_

Sí, las funciones exec() retornan solo si ha ocurrido un error. El valor de retorno es -1 y errno se establece para indicar el error.
En la shell si llega a haber un error, se informa y sale con -1.

---

### Comandos built-in

_¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)_

Sí. Se puede implementar pwd sin bulilt-in ya que imprime por salida estándar el directorio actual del usuario y sale, y eso puede hacerse tranquilamente desde cualquier proceso porque puede obtener el directorio de donde fue llamado. Es implementado como built-in porque el directorio actual es una variable que la shell debe mantener actualizada y es más eficiente imprimir esa variable que hacer un nuevo proceso (con fork y exec de pwd).

Cd no puede ser implementado como aparte porque su función es modificar el directorio de trabajo actual del usuario, y no puede hacerse un fork y ejecutarse aparte ya que se modificaría el directorio de trabajo del proceso hijo, pero el del padre (la shell) no se cambiaría.

---

### Variables de entorno adicionales
_¿Por qué es necesario hacerlo luego de la llamada a fork(2)?_

 Es necesario hacerlo luego de la llamada a fork para que estas variables de entorno solo existan en el proceso hijo. Una vez este que finalizó el proceso hijo, el padre no tendrá las variables de entorno que se agregaron 

_En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un array y se lo coloca en el tercer argumento de una de las funciones de exec(3)._
_¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo._


No, el comportamiento es distinto. Al pasarle de forma explicita las variables de entorno a la syscall execve o a las funciones de la familia de exec, el nuevo proceso solo tiene acceso a esas variables definidas por el programador, y no todas las definidas en la constante global 'environ'

Entonces para que sea el mismo lo que deberiamos hacer es: a demas de pasarle las definidas por el programador, pasarle las variables definidas en 'environ'

---

### Procesos en segundo plano
_Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano_

En `run_cmd` se modificó el codigo: Si la linea tiene un `&`, entonces debe ejecutarse en segundo plano, se imprime la info y se hace un wait no bloqueante para poder seguir ejecutando otras lineas. Si no tiene un `&`, entonces es una linea que debe ejecutarse en primer plano y se hace un wait bloqueante.
Por otro lado en `exec_cmd` se trata el caso de BACK, casteando al tipo de dato correspondiente y se manda a exec_cmd el comando a ejecutarse.

---

### Flujo estándar

_Investigar el significado de 2>&1, explicar cómo funciona su forma general y mostrar qué sucede con la salida de cat out.txt en el ejemplo. Luego repetirlo invertiendo el orden de las redirecciones. ¿Cambió algo?_

2>&1 redirecciona stderr(fd 2) a donde stdout(fd 1) apunte. En el ejemplo, out.txt además de tener el resultado de ls /home tiene el mensaje de error que generó ls /noexiste.

Invirtiendo se obtuvo el mismo resultado:
``` sh
 /home/stephi 
$ ls -C /home /noexiste 2>&1 >out.txt        
	Program: [ls -C /home /noexiste 2>&1 >out.txt] exited, status: 2 
 /home/stephi 
$ cat out.txt
ls: cannot access '/noexiste': No such file or directory
/home:
stephi
	Program: [cat out.txt] exited, status: 0 
 /home/stephi 
 $ ls -C /home /noexiste >out1.txt 2>&1   
	Program: [ls -C /home /noexiste >out1.txt 2>&1] exited, status: 2 
 /home/stephi 
$ cat out1.txt
ls: cannot access '/noexiste': No such file or directory
/home:
stephi
	Program: [cat out1.txt] exited, status: 0 
 /home/stephi 
$ 
```

---

### Tuberías simples (pipes)

_Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe ¿Cambia en algo? ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con la implementación del este lab._

Mediante la variable de bash PIPESTATUS podemos saber el valor de exitcode de los pipes y se utiliza como un vector:
``` sh
$ false | true
$ echo "${PIPESTATUS[0]} ${PIPESTATUS[1]}"
1 0
```
Si la palabra reservada "!" precede al pipe, el estado de salida es la negación lógica del estado de salida como se describe anteriormente. La shell espera a que terminen todos los comandos del pipe antes de devolver un valor.
Si pipefail está habilitado, el estado de retorno del pipe es el valor del último comando (más a la derecha) para salir con un estado distinto de cero, o cero si todos los comandos salen correctamente.

Si alguno de los comandos del pipe aborta, se corta la ejecución del pipe y envia una senal "SIGPIPE"
Ejemplo: si de un extremo cerramos el pipe y del otro extremo se esta escribiendo o leyendo entonces tendremos una pipe roto.

---

### Pseudo-variables

_Investigar al menos otras tres variables mágicas estándar, y describir su propósito. Incluir un ejemplo de su uso en bash (u otra terminal similar)._

- `$1`, `$2`, `$3`, ... parámetros de posición que hacen referencia al primer, segundo, tercer, etc. parámetro pasado al script.
- `$_` el último argumento pasado al último comando ejecutado (justo después de arrancar la shell, este valor guarda la ruta absoluta del comando que inicio la shell).
- `$#` número total de argumentos pasados al script actual.
- `$*` conjunto de todos los parametros en un solo argumento.
- `$@` conjunto de argumentos, uno por cada parametro
- `$-` la lista de opciones de la shell actual.
- `$$` el PID de la shell actual.
- `$IFS` el separador utilizado para delimitar los campos.
- `$?` el código de salida del pipe más reciente (es decir, de la última vez que se encadenaron varios comandos mediante el carácter `|`).
- `$!` el PID del último comando ejecutado en segundo plano.
- `$0` el nombre de la shell o del script de shell.

---

