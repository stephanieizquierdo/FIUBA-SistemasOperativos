TP1: Memoria virtual en JOS
===========================

Segmentacion
--------------
Tarea: simulación de traducciones

Primera Ejecucion:
```
./segmentation.py -a 64 -p 256 -s 104222 -n 2
ARG seed 104222
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x0000004f (decimal 79)
  Segment 0 limit                  : 24

  Segment 1 base  (grows negative) : 0x000000dc (decimal 220)
  Segment 1 limit                  : 25

Virtual Address Trace
  VA  0: 0x00000025 (decimal:   37) --> PA or segmentation violation?
  VA  1: 0x0000003f (decimal:   63) --> PA or segmentation violation?

For each virtual address, either write down the physical address it translates to
OR write down that it is an out-of-bounds address (a segmentation violation). For
this problem, you should assume a simple address space with two segments: the top
bit of the virtual address can thus be used to check whether the virtual address
is in segment 0 (topbit=0) or segment 1 (topbit=1). Note that the base/limit pairs
given to you grow in different directions, depending on the segment, i.e., segment 0
grows in the positive direction, whereas segment 1 in the negative. 
```
1. Va1 = 37 
   Pa1 = segmentation fault 
   Va2 = 63
   Pa2 = 219
2. La primer direccion virtual a traducir es la 37(100101) donde l bit mas significativo es 1 por lo que corresponde al segmento 1. De offset nos queda el numero 5(00101). Para verificar que este en el rango de direcciones validas del segmento, agarro la direccion virtual de la base del segmento que es 32 y como esta crece de forma negativa le resto el limite(tamanio de mi segmento) y obtengo un rango entre 7 y 32. Como 5 no pertenece a ese rango, no es una direccion fisica valida por lo que hay segmentation fault.
Para la segunda direccion se realiza el mismo procedimiento. A la direccion 63(111111) le corresponde al segmento 1. De offset nos queda el numero 31(11111). Para verificar que este este en el rango de direcciones validas del segmento, agarro la direccion virtual de la base del segumento que es 32 y como esta crece negativamente, se le resta el limite obteniendo el rango entre 32 y 7. Como 31 pertenece al rango, es una direccion valida. Al proceder a calcular la direccion fisica nos paramos sobre la base fisica del segmento (220) y le restamos 32(ya que el segmento decrese) obtieniendo la direcion 188(la primer direccion valida) le sumo el offset que es 31 y obtengo la direccion fisica 219. 
3. Verificacion con flags -c
``` 
./segmentation.py -a 64 -p 256 -s 104222 -n 2 -c
ARG seed 104222
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x0000004f (decimal 79)
  Segment 0 limit                  : 24

  Segment 1 base  (grows negative) : 0x000000dc (decimal 220)
  Segment 1 limit                  : 25

Virtual Address Trace
  VA  0: 0x00000025 (decimal:   37) --> SEGMENTATION VIOLATION (SEG1)
  VA  1: 0x0000003f (decimal:   63) --> VALID in SEG1: 0x000000db (decimal:  219)
```

Segunda Simulacion:
```
./segmentation.py -a 64 -p 256 -s 104196 -n 2 
ARG seed 104196
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x0000002c (decimal 44)
  Segment 0 limit                  : 19

  Segment 1 base  (grows negative) : 0x000000c9 (decimal 201)
  Segment 1 limit                  : 26

Virtual Address Trace
  VA  0: 0x0000003c (decimal:   60) --> PA or segmentation violation?
  VA  1: 0x00000015 (decimal:   21) --> PA or segmentation violation?

For each virtual address, either write down the physical address it translates to
OR write down that it is an out-of-bounds address (a segmentation violation). For
this problem, you should assume a simple address space with two segments: the top
bit of the virtual address can thus be used to check whether the virtual address
is in segment 0 (topbit=0) or segment 1 (topbit=1). Note that the base/limit pairs
given to you grow in different directions, depending on the segment, i.e., segment 0
grows in the positive direction, whereas segment 1 in the negative. 
```
1. Va1: 60 Pa1: 197
   Va2: 21 Pa2: segmentation fault
2. La primer direccion virtual a traducir es la 60(111100) donde el bit mas significativo es 1 por lo que corresponde al segmento 1. De offset nos queda el numero 28(11100). Para verificar que este en el rango de direcciones validas del segmento, agarro la direccion virtual de la base del segmento que es 32 y como esta crece de forma negativa le resto el limite(tamanio de mi segmento) y obtengo un rango entre 6 y 32. Como 28 pertenece al rango, es una direccion valida. Al proceder a calcular la direccion fisica nos paramos sobre la base fisica del segmento (201) y le restamos 32(ya que el segmento decrese) obteniendo la direcion 169(la primer direccion valida) le sumo el offset que es 28 y obtengo la direccion fisica 197.
Para la segunda direccion se realiza el mismo procedimiento. A la direccion 28(010101) le corresponde al segmento 0. De offset nos queda el numero 21(10101). Para verificar que este este en el rango de direcciones validas del segmento, agarro la direccion virtual de la base del segumento que es 32 y como esta crece positivamente, se le suma el limite obteniendo el rango entre 32 y 51. Como 21 no pertenece a ese rango, no es una direccion fisica valida por lo que hay segmentation fault.
3. Verificacion con flag -c
```
./segmentation.py -a 64 -p 256 -s 104196 -n 2 -c
ARG seed 104196
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x0000002c (decimal 44)
  Segment 0 limit                  : 19

  Segment 1 base  (grows negative) : 0x000000c9 (decimal 201)
  Segment 1 limit                  : 26

Virtual Address Trace
  VA  0: 0x0000003c (decimal:   60) --> VALID in SEG1: 0x000000c5 (decimal:  197)
  VA  1: 0x00000015 (decimal:   21) --> SEGMENTATION VIOLATION (SEG0)
```
Tarea: traducciones inversas

Para la corrida 1 no hay solucion optima, si se quiere modoficar el limite para poder lograr que la direccion virtual 37 sea valida se debera poner un limite = 28, pero este limite no permite que se obtenga un segmentation fault(no sea valida) para la direccion virtual 63 ya que el offset obtenido seria 31 que se encuentra dentro del rango de direcciones virtual (entre 32 y 4).

Para la segunda corrida tampoco se pudo obtener una solucion optima dado que para la segunda direccion al estar en el segmento 0 no existe limite que permita que se pueda obtener una direccion valida para el offset 28, mientras que para la primera direccion el unico limite que satisfage para poder tener un segmentation fault es si el limite tiene tamaño 0, lo cual no tendria sendria sentido. 

Tarea: límites de segmentación
1. El tamaño del espacio virtual es 32 y el tamaño del espacio fisico es de 128
2. Si. Esto es posible cuando estan superpuestos los dos segmentos en memoria fisica. Esto sucede cuando la ultima direccion del segmento 0, corresponda tambien a una direccion del segmento 1. 
Ejemplo:
```
./segmentation.py -a 32 -p 128 -s 104222 -A 5,25 -b 66 -l 6 -B 78 -L 15 -c
ARG seed 104222
ARG address space size 32
ARG phys mem size 128

Segment register information:

  Segment 0 base  (grows positive) : 0x00000042 (decimal 66)
  Segment 0 limit                  : 6

  Segment 1 base  (grows negative) : 0x0000004e (decimal 78)
  Segment 1 limit                  : 15

Virtual Address Trace
  VA  0: 0x00000005 (decimal:    5) --> VALID in SEG0: 0x00000047 (decimal:   71)
  VA  1: 0x00000019 (decimal:   25) --> VALID in SEG1: 0x00000047 (decimal:   71)
```
3. Para las direcciones virtuales es posible que haya un mapeo del 90% a direcciones fisicas ya que el 90% de 32(cantidad de direcciones virtuales) corresponde a 29 direcciones aproximadamente y como hay 128 direcciones fisicas se podria hacer(28 >128). Un ejemplo seria:
Para el segemento 0:
base: 50
limite: 15
Para el segmento 1:
base: 90
limite 14
En este caso se estaria realizando el mapeo de las 29 direcciones y estas no se sopan entre si. 
4. Para las direcciones fisica no es posible que haya un mapero del 90% a virtuales ya que el 90% de 128 es 115 aproximadamente y no se tiene 115 direcciones virtuales. Solo se podria mappear el 25% que corresponde a la cantidad de direcciones virtuales(32).

boot_alloc_pos
--------------

...


page_alloc
----------
page2pa nos da la dirección física donde comienza la página recibida por parámetro.

pa2vk nos da la dirección virtual del kernel de la dirección física del comienzo de la página recibida por parámetro.
...


map_region_large
----------------

...

