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

## Entrega 2

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

### Entrega 1

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
