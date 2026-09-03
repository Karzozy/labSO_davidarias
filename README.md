# Laboratorio de Sistemas Operativos  

## Integrantes

- David Santiago Arias Narvaez <davidarias@unicauca.edu.co>

## Planteamiento del problema
# Taller 1 - llamadas al sistema
Enunciado
1. Escribir un programa en C que:
obtenga su identificador de proceso y el de su proceso padre;
escriba un mensaje en la salida estandar usando write, no printf;
termine con un codigo de salida distinto de cero, elegido por el autor.
2. Verificar el valor de retorno de cada llamada al sistema. Si una falla,
informar la causa con perror y terminar con un codigo de error.
3. Comprobar el codigo de salida del programa desde la terminal:
```sh
$ ./programa
$ echo $?
```
4. Escribir un segundo programa que imprima el mismo texto dos veces: una
con printf y otra con write, en ese orden, sin usar fflush ni terminar
el texto con salto de linea. Ejecutarlo y observar en que orden aparece
cada mensaje.
5. Explicar en la bitacora por que aparecen en ese orden. La respuesta tiene que ver con donde queda el texto antes de llegar a la pantalla.
# Taller 2 - procesos, scripts y llamadas al sistema
Enunciado
1. Crear el usuario, asignarle una contraseña y agregarlo al grupo sudo, que
es el que habilita la ejecución de tareas administrativas. La quinta parte
las necesita:
```sh
$ sudo adduser milogin
$ sudo adduser milogin sudo
```
2. Comprobar que la cuenta quedó registrada, leyendo su línea en el archivo
de cuentas del sistema, y comprobar a qué grupos pertenece. Identificar
en la primera salida el identificador de usuario, el directorio de inicio y el
intérprete asignado:
```sh
$ grep milogin /etc/passwd
$ groups milogin
```
3. Abrir una sesión del usuario recién creado y comprobar que el símbolo
del sistema muestra su nombre:
```sh
$ su - milogin
$ whoami
```
4. Listar los procesos del sistema y localizar en la salida las columnas de
identificador, identificador del padre, estado y línea de comandos:
```sh
$ ps -ef
$ ps aux
```
5. Lanzar un proceso propio en segundo plano, anotar el identificador que
informa el shell, y localizarlo en la lista de procesos:
```sh
$ sleep 300 &
$ ps -ef | grep sleep
```
6. Consultar el estado de ese proceso en /proc y comparar los campos Name,
State, PPid y Threads con lo que informa ps sobre el mismo proceso.
Preguntar después por el tamaño de ese archivo de dos maneras distintas,
y anotar en la bitácora lo que informa cada una:
```sh
$ stat -c %s /proc/self/status
$ wc -c < /proc/self/status
```
Las dos respuestas no coinciden. Explicar en la bitácora a qué se debe,
teniendo en cuenta que stat consulta los atributos del archivo y wc lee
su contenido.  
7. Terminar el proceso lanzado en el punto 5 con kill y comprobar que su
subdirectorio de /proc deja de existir.  
8. Recorrer la cadena de padres a mano: partiendo del shell, consultar el
campo PPid, consultar después el estado de ese padre, y repetir hasta
llegar al proceso 1. Anotar en la bitácora la cadena completa y qué
programa corresponde al proceso 1.
La orden pstree -p muestra esa misma jerarquía de una sola vez. No está
instalada en todas las distribuciones; si no responde, la cadena obtenida a
mano es suficiente.  
9. Escribir un script llamado infoproc.sh que reciba un identificador de
proceso como parámetro. Si no recibe ninguno, el script informa sobre el
proceso que lo está ejecutando, cuyo identificador está en la variable $$.  
10. Si el identificador recibido no corresponde a ningún proceso vivo, el script
termina con un mensaje que lo indique y con un código de salida distinto
de cero. La comprobación se hace sobre la existencia del subdirectorio en
/proc, antes de intentar leer nada.  
11. Para el proceso consultado, el script informa el nombre, el estado, el
identificador del padre y la cantidad de hilos. Los cuatro valores se leen
del archivo status.  
12. Cada línea de ese archivo trae la etiqueta, un separador y el valor:
```sh
$ grep Threads /proc/self/status
```
Threads: 1  
El valor se extrae de la línea con un operador de patrones del shell,
aplicado sobre la variable que la contiene. No se admite resolverlo invo-
cando otro programa.  
13. Ampliar el script para que, además del proceso consultado, recorra la
cadena de padres con un ciclo hasta llegar al proceso 1, e imprima un
renglón por cada proceso de la cadena.  
14. El script termina con código de salida cero cuando el recorrido se comple-
ta.  
15. Escribir un programa en C llamado infoproc.c que produzca la misma
información del punto 11 sobre el proceso cuyo identificador reciba como
argumento. Si no recibe argumento, informa sobre sí mismo.  
16. El archivo se abre con open, se lee con read y se cierra con close. No
se admite el uso de fopen, fgets ni ninguna otra función de la biblioteca
de entrada y salida con formato. La restricción es sobre la lectura del
archivo, no sobre el análisis del texto ya leído: las funciones de manejo de
cadenas de string.h sí se pueden usar.  
17. La salida se escribe con printf o con write sobre STDOUT_FILENO: las
dos formas son admisibles. La restricción del punto anterior es sobre la
lectura del archivo, no sobre la salida ni sobre el formateo de cadenas en
memoria, de modo que snprintf también se puede usar.  
18. Verificar el valor de retorno de cada llamada al sistema. Si una falla,
informar la causa con perror y terminar con un código de error.  
19. Informar también el identificador del propio programa y el de su proceso
padre, obtenidos con getpid y getppid. Ejecutar el programa desde el
shell y comprobar a qué proceso corresponde el padre.  
20. El programa debe compilar sin advertencias:
```sh
$ gcc -Wall -o infoproc infoproc.c
```
Junto al enunciado se publica un Makefile que automatiza esa orden
y agrega un objetivo clean que borra lo generado. Con él, construir y
limpiar son:
```sh
$ make
$ make clean
```
El Makefile forma parte de lo que se entrega. Las órdenes de sus reglas
van precedidas de un tabulador, que se pierde al copiar el texto desde este
documento: se usa el archivo publicado.  
21. Ejecutar el script y el programa sobre el mismo identificador de proceso y
comprobar que la información coincide.  
22. Explicar en la bitácora de dónde sale la información en cada caso, y por
qué dos programas tan distintos entregan el mismo resultado.  
23. Ejecutar el script y el programa sobre el identificador de un proceso
que esté cambiando de estado, como el navegador o un compilador en
ejecución, y observar si el informe es idéntico. Explicar en la bitácora el
resultado.  
24. El shell también es un programa. Explicar en la bitácora qué hace el shell,
en términos de servicios del sistema operativo, desde que se escribe una
línea de comandos hasta que aparece de nuevo el símbolo de espera.  
25. Instalar la herramienta de conversión y comprobar que responde:
```sh
$ sudo apt install ffmpeg
$ ffmpeg -version
```
26. Generar los archivos de prueba con la misma herramienta, en lugar de
descargar material. Cada archivo dura tres segundos y ocupa muy poco:
```sh
$ ffmpeg -f lavfi -i testsrc=duration=3:size=320x240 clase1.mkv
```
Generar al menos cuatro archivos, uno de ellos con varios puntos en el
nombre, como clase.2026.mkv.  
27. Escribir un script llamado convertir.sh que tome todos los archivos con
extensión .mkv del directorio actual y genere para cada uno el archivo
.mp4 correspondiente, conservando el nombre. El cambio de extensión
se resuelve con un operador de patrones, no invocando otro programa.  
28. Hacer que el script no vuelva a convertir un archivo cuyo .mp4 ya exista,
e informe de que lo está saltando.  
29. Aceptar como parámetro opcional el directorio donde buscar. Si no se
recibe, usar el directorio actual. Si la ruta recibida no existe o no es un
directorio, terminar con un mensaje claro y sin convertir nada.  
30. Informar al final cuántos archivos se convirtieron y cuántos se saltaron.  
31. Ejecutar el script dos veces seguidas sobre el mismo directorio. La segunda
ejecución no debe convertir nada ni producir ningún error.  
32. Registrar como evidencia tres capturas de pantalla, tomadas desde la
sesión del usuario creado en la preparación y con el símbolo del sistema
visible: la comprobación del punto 25, la primera ejecución del script y la
segunda ejecución del punto 31. En una entrega en pareja, al menos una
de las tres se toma desde la sesión de cada integrante.
$ su - milogin
$ whoami
