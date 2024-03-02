# Desafío Shell
## Implementación
### History y History n
Partimos de que ahora vamos a tener un comando history, este hace que tengamos que mostrar los comandos anteriores. Para esto en run_cmd se llama a history() de built-int donde hice el caso donde si el comando tiene la palabra "history", vamos a aperturar el archivo[^1] donde se guardan los comandos[^2], luego chequeo si el cmd es history a secas, si no entonces debe ser un "history n".

Entonces luego tenemos que mostrar las n o todas las lineas del archivo de history (cada linea fue un comando ejecutado).
Para esto se abre el archivo, se cuentan las lineas que hay, volvemos a pararnos en el comienzo del historial, avanzamos hasta donde nos pidieron comenzar a mostrar y luego se muestra todo en adelante[^3].

[^1]: Si está definida la variable de entorno HISTFILE, se busca ese archivo. Si no, se usa uno por default.
[^2]: Cuando el usuario ingresa "history" en la terminal este tambien es considerado un comando por lo cual
hacer "history 1" va a mostrar "history 1 " tal y como una terminal normal lo haria.
[^3]: Se podría haber hecho una lectura al reves, partiendo desde el final y leyendo para arriba cortando en cada \n, pero lo dejé de la otra forma porque era más simple usar el get_line. 

### Desplazamientos y backspace:

Para esto se modificó la función de readline porque se tiene que actuar según lo que ingrese el usuario en la terminal.
Mientras que leemos la linea hay que fijarse si en algun momento se recibe un `[` ya que esto indica que si luego de este caracter viene
una A, es un desplazamiento para arriba o si es una B, es un desplazamiento para abajo.

Cuando recibo un desplazamiento se procede a limpiar el buffer y asignarle el comando que tiene que mostrar y se procede a mostrarlo por pantalla para
informar al usuario y además esto luego lo recibe run_cmd para que lo ejecute.

* Si nuestro historial esta vacio(porque todavia no se ejecuto nunca algo), no hay nada que mostrar.
* Si se desplazó para arriba hasta el comienzo del historial, se vuelve a mostrar desde el comando más reciente ejecutando guardado en el historial.
* En cambio si se desplazó para abajo lo más posible, se muestra el prompt vacio, tal y como la terminal lo hace.

### Event Designators

Para este caso se implementó `!!` y `!-n`.
* `!!` muestra el comando anterior, `!-1` hace lo mismo.

* `!-n` muestra el enésimo comando anterior.

Esto afecta al archivo de historial con la misma logica que lo hace mi terminal normal, es decir, no se guarda el comando referenciado por `!!` o `!-1`, pero sí por `!-n` para n > 1.

Para lograr esto modifiqué run_cmd para que guarde el cmd que recibió o el comando a ejecutar si llegó un !-n. Pero si llegó el event_designator `!!` o `!-1` se llama a otro run_cmd que no guarda en el historia el comando a ejecutar[^4] .
[^4]: Podria haber modificado el original para no tener dos pero queria mantener lo de shell a secas lo mas parecido posible a como lo entregue.

## Sección de pregunta teórica:
> ¿Cuál es la función de los parámetros MIN y TIME del modo no canónico? ¿Qué se logra en el ejemplo dado al establecer a MIN en 1 y a TIME en 0?

Cuando estamos en modo no canónico los caracteres no se agrupan en lineas y todos los caracteres de entrada se pasan al programa exactamente como se escribieron. Entonces sse usa los parametros `MIN` y `TIME` para controlar si esperar y cuánto tiempo para que la entrada esté disponible.

Si `TIME` es cero pero `MIN` es 1, read espera hasta que haya al menos 1 byte disponible en la cola. En ese momento, read devuelve tantos caracteres como estén disponibles, hasta la cantidad de número solicitado. Read puede devolver más de MIN caracteres si hay más de MIN en la cola.

## Experiencia

Me parecio bastante divertido en general el desafio pero la parte del desplazamiento me trajo bastantes bugs ya que justamente lo que se ve por terminal puede no ser lo que pasa tras bambalinas (y cai en eso tarde), entonces para debuggearlo era tener en cuenta ambas cosas y mareaba bastante. Hubiera sido muy bueno tener como una especie de "pruebas" para verificar que se esta avanzando bien en el tp, de todas formas lo que hice fue comparar siempre con mi terminal normal y apuntaba a imitar eso.

La desventaja fuerte que tuve fue que la shell fue uno de los primeros tps que hicimos y como no tenia la corrección no hice el desafío antes entonces tuve que volver a ver todo el código de la shell y volver a ponerme en onda con el tp, lo cual eso hizo que me demorara en tiempo.

Mas allá de eso me pareció re interesante y me gustó mucho. Aprendi cosas nuevas en el transcurso de la implementación y comandos re útiles (desconocia los event designators y lo de history n) :D

PD: perdón por el espanenglish.
