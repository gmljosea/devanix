# Devanix!

*   Jennifer Dos Reis 08-10323
*   José Alberto Goncalves 08-10455

La especificación del lenguaje se encuentra en **doc**

Para compilar:

    cd src
    make

El ejecutable estará en src/devanix

Para ejecutar, hacer:

    devanix archivo

En el directorio **examples** hay varios ejemplos, algunos erróneos a propósito
para probar el compilador.

La documentación del proyecto se puede generar con el siguiente comando:

    doxygen Doxifile

Se creará un directorio que contiene la version html de la documentación ubicado en doc/html. 
La página principal es index.html. 

En la documentación se incluye la representación gráfica 
de la jerarquía de clases utilizadas para la elaboración del compilador de Devanix. Esta 
puede facilitar la comprension de la estructura que lleva el proyecto.  

## Entrega 4 (Final)

Para esta entrega se logró que el compilador Devanix generará codigo MIPS para lo siguiente:

*    Todas las operaciones aritmeticas definidas en el lenguaje, tanto para enteros como flotantes.
*    Asignaciones simples y simultaneas.
*    Funciones, incluyendo recursión.
*    Read y write de enteros y flotantes.
*    Write de strings, enteros, flotantes y booleanos.
*    Instrucciones while, for e if. En el caso de for hay un bug con el acceso a la variable de iteración.
*    Técnicamente genera los casts entre enteros, flotantes y caracteres, pero no lo probamos.
*    Asignacion en las posiciones de un arreglo y acceso a las posiciones del arreglo.
     Sin embargo aún quedan varios bugs que no nos dejan confiar mucho en el código generado para arreglos.
*    Técnicamente también debe generarse el código para los tipos box (pues al traducir a código TAC se traduce en accesos a arreglos)
     pero no los probamos.

Para la asignación de registros se utilizó descriptores. Al final de cada bloque básico
se vacían todos los descriptores. Para evitar hacer spills innecesarios de temporales
a memoria, se implantó un análisis de dataflow para evaluar los temporales vivos.

Se genera un grafo de control por cada función y se guarda en una archivo aparte.
Se genera un grafo con el código TAC (en los archivos prefijados con "tac_f_") y
uno con el código MIPS generado (en los archivos prefijados con "mips_f_").
El grafo de MIPS no incluye el código para el prologo y epílogo de cada función.

En la carpeta examples hemos agregado algunos ejemplos de programas escritos en Devanix.
Existe un fibonnaci iterativo, un hello world, un factorial recursivo, potenciación
recursiva, una prueba de funciones corecursivas, y un ejemplo sencillo con operaciones
punto flotante.

## Entrega 3

Para esta entrega se implantó lo siguiente:

*   Traducción de Devanix a TAC. Hemos colocado una definición del TAC usado en
    el archivo doc/TAC.md. 
*   Durante la generación de código usamos Fall Through Optimization.
*   No se realizó ninguna optimización después de generar el código.
*   Partición de TAC generado en bloques básicos y generación del grafo de control
    de flujo.

El compilador devuelve por la salida estándar el TAC generado, y el grafo de control
se genera siempre en un archivo llamado 'flowgraph.dot'. Es necesario tener instalada
alguna herramienta capaz de traducir del lenguaje DOT a una representación gráfica.

Hemos colocado algunos ejemplos en la carpeta examples/

Detalles a considerar sobre esta entrega:

*   No hemos hecho los cambios sugeridos a nuestro lenguaje, por lo que aún no tenemos
    arreglos multidimensionales ni tipos **box** declarables en cualquier bloque.
    Hemos tratado de que el generador de TAC sea neutral con respecto a estos aspectos.
*   Se nos olvidó por completo tratar los cast y por lo tanto no funcionan. Intentar
    usar uno causa segfault.
*   Desactivamos el chequeo de referencias cíclicas entre tipos **box**. Encontramos
    un bug que luego decidimos no resolver porque pretendemos que al implantar
    la posibilidad de declarar tipos *box* en cualquier contexto, eliminaremos
    la visibilidad de los **box** hacia otros **box** declarados debajo.
*   Los bloques básicos que terminan con **return** apuntan a todos aquellos bloques
    a los que pertenezcan las instrucciones que seguían directamente a un **call** de
    esa función en el código original. Los **call** se tratan como saltos incondicionales.


### Entrega 2

Para esta entrega se implantaron los siguientes elementos:

*   Tipo arreglo y tipo box.
*   Expresiones con los operadores aritméticos, lógicos y relacionales tradicionales.
*   Todos los chequeos semánticos posibles. El compilador debería poder detectar la
    mayoría de los errores.
*   Constant folding en expresiones aritméticas, lógicas y relacionales; excepto en
    el operador == y != entre tipos char.
*   Recuperación de errores sencilla en el parser.

En los directorios examples/ y tests/ hay ejemplos, casi todos con errores, que
muestran parte de los chequeos que realiza el compilador.

#### Entrega 1

Hasta ahora el parser detecta:

*   Variables o funciones declaradas dos veces en el mismo alcance.
*   Uso de variables o funciones no declaradas
*   Constantes enteras y flotantes muy grandes.
*   Tipos incorrectos en asignación, if, while, for y return.
*   Existencia de la función main.
*   Las variables tipo string son siempre inicializados.
*   Etiquetas de iteración duplicadas.

Los errores muestran su línea y columna, pero ahora sólo muestra la línea y
columna de toda la instrucción en vez de la expresión o símbolo específico
problemático. Cuando se utiliza una función no declarada, por ahora se
muestra la línea y columna como 0:0, porque ahorita la expresiones no guardan
su ubicación.

Chequeos que no estamos realizando:

*    Número de argumentos y tipos en llamadas a funciones.
*    Que el Break y Next estén dentro de una iteración, y cuando tengan
     etiqueta, que sea igual a la de alguna iteración más arriba.
*    Variables readonly (la variable de iteración del for, argumentos pasados
     con $$)
*    Solo expresiones constantes y variables en las inicializaciones de variables
     globales.
*    Otros que no nos acordamos ahorita :)
