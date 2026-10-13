Taller 5 - senales en un servidor concurrente

Cuando se ejecuta el servidor por primera vez, lo que mas llama la atencion es que no termina solo ni refleja nada del estado interno del tablero, porque el programa estaba atrapado en una espera de conexiones y no tenia ninguna estrategia de respuesta ante eventos externos. El problema real no es solo que el servicio sea concurrente, sino que ademas no sabe distinguir entre una conexion nueva y una senal del sistema. Ahi aparece el punto central de la practica: una senal no debe hacer el trabajo del programa, sino avisarle que algo ocurrio para que el ciclo principal lo resuelva en el momento correcto. La parte mas importante es que el manejador no debe imprimir nada ni tomar bloqueos. Si un manejador hace printf, se ejecuta en medio de la corrida normal del proceso y puede romper cosas que estaban a mitad de ejecutarse. Si intenta tomar el mutex del tablero, puede producir un deadlock porque el hilo que estaba corriendo justo antes de la senal ya tiene ese mismo candado. Por eso la regla del taller es clara: el manejador solo marca una bandera y el codigo principal hace el resto. Vamos con el cierre: La diferencia entre un cierre brusco y un cierre ordenado se ve muy claro con SIGINT. Cuando llega la senal, el manejador pone la bandera terminar = 1, y luego el bucle principal sale del accept(), cierra el socket de escucha y termina de forma controlada. Eso evita que el servidor se quede con el puerto escuchando y tambien permite publicar cuantas lecturas y cuantas escrituras atendio antes de apagarse.

Lo que hace que esto funcione es que la llamada accept() no se reinicia automaticamente con SA_RESTART: si el sistema reanudara la llamada, el programa nunca recuperaria el control. En cambio, accept() devuelve -1 con errno = EINTR, y entonces el ciclo principal sabe que la senal interrumpio la espera y puede responder con calma. Ese detalle es muy importante, porque la senal llega, pero el servicio no se queda atrapado en una llamada que nunca termina.

La salida real observada al cerrar el servidor con Ctrl-C fue:

Tablero de 64 casillas, politica rwlock, escuchando en el puerto 5500
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 32, tablero AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
Servidor apagado por SIGINT. Lecturas atendidas: 0, escrituras atendidas: 50

El cliente conectado tampoco quedo inmovilizado: la conexion se cerro de forma limpia y el cliente reporto que la serie de escrituras se completo bien:

50 escrituras del caracter A, 7.961 segundos

Eso muestra que el servidor no solo termina, sino que termina sin dejar huellas de un cierre brutal ni perder el resumen final de su actividad.

Para el punto 4 la parte que mas se parece a la practica anterior es la de los contadores. El servidor atiende varios clientes al mismo tiempo, cada uno puede ejecutar ordenes de lectura o escritura sobre el tablero, y esos eventos se convierten en aumento de contadores. Si dos hilos hacen esa actualizacion a la vez, existe una carrera de datos: uno puede sobrescribir el valor del otro, y el total final ya no refleja lo que realmente atendio el servidor. Por eso se eligio pthread_mutex_t para proteger tanto lecturas_atendidas como escrituras_atendidas. La decision no es arbitraria: cada incremento es una operacion corta, simple y extremadamente sensible a la concurrencia. Un mutex garantiza que la zona critica sea mutuamente excluyente, y con eso se evita la perdida de actualizaciones y se mantiene el resultado consistente.

Es decir, el problema no cambia de naturaleza: sigue siendo un recurso compartido entre hilos, y la solucion sigue siendo la misma, aunque ahora el recurso sea una variable de conteo y no una matriz que guarda el tablero. La diferencia es que aqui la proteccion no esta sobre el tablero sino sobre el historial del servidor. El criterio sigue siendo el mismo: proteger el dato compartido para que el valor final represente la realidad del sistema y no una suma incompleta por rivalidad entre hilos.

Para el punto 6 la alarma se instala con sigaction(SIGALRM, ...) y se programa con alarm(5). Cada vez que el manejador marca la bandera, el bucle principal toma el control y presenta el contenido actual del tablero junto con su version. Eso hace que el servidor no dependa de que un cliente haga la lectura, sino que emita su estado de manera periodica, como una foto del sistema en ese instante.

La salida que se observo fue:

Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 32, tablero AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

Eso demuestra dos cosas: que la alarma funciona y que el informe se genera en el flujo principal, no dentro del manejador.

Cuando se manda SIGINT, el manejador solo registra que llego la senal y pone la bandera de terminacion. El ciclo principal, al ver ese flag, sale del accept(), cierra el socket de escucha y termina con un resumen final. Eso es lo que convierte la interrupcion en un cierre ordenado y no en una desaparicion abrupta del proceso.

La senal SIGKILL es distinta de todas las demas: no puede capturarse ni ignorarse, y el proceso muere inmediatamente. Si SIGKILL pudiera atraparse, un proceso bloqueado se volveria inmune a la terminacion de emergencia.
La comprobacion del comportamiento fue:

[1] 7102
[1]+  Killed                  bash -c 'while :; do :; do :; done'
process terminated by KILL

La clave es que el sistema no permite esto por una razon muy clara: hay casos en los que la salida forzada es la unica opcion segura para recuperar el control del sistema.

Para el punto 7 no se deben ocultar los errores de las llamadas al sistema. Para comprobarlo, se arranco un segundo servidor sobre el mismo puerto 5500 mientras el primero seguia escuchando, y el sistema respondio con este mensaje real:

bind: Address already in use

Eso es exactamente lo que se espera de perror: el programa informa el problema y no se queda callado aunque la causa sea un fallo de infraestructura. En cualquier servicio real, ese tipo de diagnostico es fundamental porque dice exactamente que esta fallando y por que.


Para el punto 8 la linea signal(SIGPIPE, SIG_IGN) tiene un efecto que es que ignora la senal SIGPIPE. Esa senal se produce cuando un proceso intenta escribir en un socket donde la conexion ya fue cerrada por el otro extremo.

Sin esa ignorancia, el servidor se mataria simplemente por intentar escribir sobre un cliente que ya se fue. Con la ignorancia, la operacion falla de manera normal y el programa puede reaccionar limpiamente: cierra la conexion y sigue funcionando. Esa diferencia es enorme, porque convierte un caso extremo en un error manejado, y hace que el servidor siga vivo aunque un cliente cierre la conexion de golpe.

En otras palabras, la senal SIGPIPE no es un detalle menor: es la diferencia entre un servicio resiliente y un servicio que se cae cada vez que un cliente desaparece sin avisar.

La alarma no solo sirve para imprimir algo bonito: sirve para tener una imagen del sistema en tiempo real sin depender de un cliente externo. Eso es muy importante en un servidor concurrente, porque el estado del tablero puede cambiar a cada instante y la unica forma de observarlo sin intervenir manualmente es mediante una senal programada.

La salida periodica fue:

Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 0, tablero ................................................................
Informe periodico: version 32, tablero AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

Esa salida da una idea muy clara de como evoluciona el tablero y de como el servidor conserva la consistencia del contenido incluso cuando varios eventos ocurren al mismo tiempo.

Al final podemos decir que el manejador solo registra, el ciclo principal hace el trabajo y los datos compartidos se protegen bajo un criterio efectivo.

Con eso, el servidor no solo responde a SIGINT y a SIGALRM, sino que ademas informa errores, cierra ordenadamente y presenta el estado actual sin dejar al sistema en un estado inconsistente. En resumen, la practica muestra que la senal no es un accidente del sistema, sino una herramienta de coordinacion muy poderosa cuando se usa con disciplina.
