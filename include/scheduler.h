#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "proceso.h"

/*
 * Inicializa la estructura del planificador.
 */
void scheduler_init(void);

/*
 * Ingresa un nuevo proceso al Scheduler en el primer espacio libre disponible
 * utilizando un algoritmo de búsqueda circular a partir del cursor para evitar
 * el sesgo hacia la izquierda del vector.
 * Requerido por la consigna académica.
 */
void ingresaProceso(void);

/*
 * Quita el proceso de la cola en el índice especificado, guardándolo en el archivo
 * de terminados y liberando la memoria.
 * Retorna el índice liberado o -1 en caso de error.
 * Requerido por la consigna académica.
 */
int terminaProceso(int indice);

/*
 * Recorre todos los procesos de la cola realizando el cambio de estado correspondiente:
 * 1) Avanza de fase los procesos ("Nuevo" -> "Listo", "Esperando" -> "Corriendo", "Corriendo" -> "Terminado").
 * 2) Libera y archiva los procesos que alcanzaron "Terminado".
 * 3) Revisa procesadores libres (1 y 2) y asigna las CPUs a los procesos "Listo"
 *    con mayor prioridad (menor número), pasándolos a "Esperando".
 * Requerido por la consigna académica.
 */
void recorreCola(void);

/*
 * Muestra por consola el estado actual de cada posición del Scheduler.
 * Requerido por la consigna académica.
 */
void mostrarScheduler(void);

/*
 * Retorna la cantidad de procesos activos actualmente en la cola.
 */
int scheduler_contar_activos(void);

/*
 * Libera toda la memoria dinámica de los procesos que sigan en el Scheduler.
 */
void scheduler_limpiar(void);

#endif /* SCHEDULER_H */
