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
Threads: 1
El valor se extrae de la línea con un operador de patrones del shell,
```
aplicado sobre la variable que la contiene. No se admite resolverlo invo-
cando otro programa.
13. Ampliar el script para que, además del proceso consultado, recorra la
cadena de padres con un ciclo hasta llegar al proceso 1, e imprima un
renglón por cada proceso de la cadena.
14. El script termina con código de salida cero cuando el recorrido se comple-
ta.

```del sistema muestra su nombre:
$ su - milogin
$ whoami
