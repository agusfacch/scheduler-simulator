#ifndef PROCESO_H
#define PROCESO_H

#define TAM_SCHEDULER   10
#define NUM_CPUS        2
#define ESTADO_MAX_LEN  16

#define ESTADO_NUEVO     "Nuevo"
#define ESTADO_LISTO     "Listo"
#define ESTADO_ESPERANDO "Esperando"
#define ESTADO_CORRIENDO "Corriendo"
#define ESTADO_TERMINADO "Terminado"

/*
 * Estructura que representa un proceso dentro de la simulación.
 * Cumple con la especificación del Trabajo Práctico de Taller II.
 */
typedef struct proceso {
    int procesador;                 /* Núcleo asignado: 0 = sin asignar, 1 o 2 */
    int id_proceso;                 /* Identificador único aleatorio [500, 5000] */
    int prioridad;                  /* Prioridad: menor valor numérico = mayor prioridad */
    char estado[ESTADO_MAX_LEN];    /* Estado actual del ciclo de vida del proceso */
} Proceso;

/*
 * Crea dinámicamente un nuevo proceso inicializado en estado "Nuevo".
 * Retorna un puntero a Proceso o NULL en caso de error de memoria.
 */
Proceso *proceso_crear(int id, int prioridad);

/*
 * Asigna el siguiente estado en el ciclo de vida según la consigna:
 *   "Nuevo"     -> "Listo"
 *   "Esperando" -> "Corriendo"
 *   "Corriendo" -> "Terminado"
 * Nota: La transición de "Listo" a "Esperando" ocurre en el scheduler
 * cuando se asigna un procesador libre por orden de prioridad.
 */
void asignaEstado(Proceso *p);

/*
 * Libera de forma segura la memoria de un proceso y setea el puntero a NULL.
 */
void proceso_destruir(Proceso **p);

/*
 * Imprime por consola los detalles formateados de un proceso.
 */
void proceso_mostrar(const Proceso *p, int slot);

#endif /* PROCESO_H */
