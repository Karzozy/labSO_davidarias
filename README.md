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
