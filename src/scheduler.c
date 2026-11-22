#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "archivo.h"

#define ID_PROCESO_MIN 500
#define ID_PROCESO_MAX 5000

/* Vector estático de punteros a procesos (capacidad máxima: TAM_SCHEDULER) */
static Proceso *scheduling[TAM_SCHEDULER] = { NULL };

/* Contador global para asignar prioridad secuencial (1, 2, 3...) */
static int next_priority = 0;

/*
 * Cursor que marca el índice a partir del cual se iniciará la búsqueda
 * de lugar libre (algoritmo circular para evitar sesgo a la izquierda).
 */
static int cursor = 0;

void scheduler_init(void) {
    for (int i = 0; i < TAM_SCHEDULER; i++) {
        scheduling[i] = NULL;
    }
    next_priority = 0;
    cursor = 0;
}

int scheduler_contar_activos(void) {
    int contador = 0;
    for (int i = 0; i < TAM_SCHEDULER; i++) {
        if (scheduling[i] != NULL) {
            contador++;
        }
    }
    return contador;
}

/*
 * Verifica si un ID de proceso ya se encuentra presente en la cola activa.
 */
static int id_esta_duplicado(int id) {
    for (int i = 0; i < TAM_SCHEDULER; i++) {
        if (scheduling[i] != NULL && scheduling[i]->id_proceso == id) {
            return 1;
        }
    }
    return 0;
}

/*
 * Genera un ID de proceso aleatorio entre [500, 5000] garantizando que no esté duplicado.
 */
static int generar_id_unico(void) {
    int intentos = 0;
    int id;
    int rango = (ID_PROCESO_MAX - ID_PROCESO_MIN + 1);

    do {
        id = ID_PROCESO_MIN + (rand() % rango);
        intentos++;
    } while (id_esta_duplicado(id) && intentos < 100);

    return id;
}

void ingresaProceso(void) {
    /* Búsqueda circular a partir del cursor para evitar sesgo a la izquierda */
    for (int count = 0; count < TAM_SCHEDULER; count++) {
        int i = (cursor + count) % TAM_SCHEDULER;

        if (scheduling[i] == NULL) {
            int nuevo_id = generar_id_unico();
            next_priority++;

            Proceso *nuevo = proceso_crear(nuevo_id, next_priority);
            if (nuevo == NULL) {
                /* Error de memoria reportado en proceso_crear */
                next_priority--; /* Revertir incremento de prioridad */
                return;
            }

            scheduling[i] = nuevo;
            cursor = (i + 1) % TAM_SCHEDULER; /* Avanza el cursor circular */

            printf("\n[+] Proceso ingresado exitosamente en slot [%d]:\n", i);
            printf("    ID: %d | Prioridad: %d | Estado: %s | Núcleo CPU: %d\n",
                   nuevo->id_proceso, nuevo->prioridad, nuevo->estado, nuevo->procesador);
            return;
        }
    }

    printf("\n[!] ¡No hay espacios disponibles en el Scheduler! (Capacidad máxima: %d procesos)\n", TAM_SCHEDULER);
}

int terminaProceso(int indice) {
    if (indice < 0 || indice >= TAM_SCHEDULER || scheduling[indice] == NULL) {
        return -1;
    }

    /* Guardar en archivo persistente */
    if (archivo_guardar_terminado(scheduling[indice]) != 0) {
        fprintf(stderr, "[!] Advertencia: No se pudo guardar el proceso %d en el archivo.\n",
                scheduling[indice]->id_proceso);
    }

    /* Liberar la memoria del proceso */
    proceso_destruir(&scheduling[indice]);

    return indice;
}

void recorreCola(void) {
    if (scheduler_contar_activos() == 0) {
        printf("\n[!] El Scheduler está vacío. ¡Nada para recorrer!\n\n");
        return;
    }

    printf("\n---> Estado del Scheduler antes de la transición:\n");
    mostrarScheduler();

    /*
     * PASO 1: Transición de ciclo de estados:
     * - "Nuevo" pasa a "Listo"
     * - "Esperando" pasa a "Corriendo"
     * - "Corriendo" pasa a "Terminado"
     */
    for (int i = 0; i < TAM_SCHEDULER; i++) {
        if (scheduling[i] != NULL) {
            asignaEstado(scheduling[i]);
        }
    }

    printf("\n---> Transición de ciclos aplicada:\n");
    mostrarScheduler();

    /*
     * PASO 2: Liberar y registrar en archivo los procesos en estado "Terminado".
     */
    int terminados_en_ciclo = 0;
    for (int i = 0; i < TAM_SCHEDULER; i++) {
        if (scheduling[i] != NULL && strcmp(scheduling[i]->estado, ESTADO_TERMINADO) == 0) {
            int id_liberado = scheduling[i]->id_proceso;
            int slot = terminaProceso(i);
            printf("[-] Proceso ID %d en slot [%d] finalizó su ejecución y fue archivado.\n", id_liberado, slot);
            terminados_en_ciclo++;
        }
    }

    if (terminados_en_ciclo > 0) {
        printf("\n---> Estado tras finalizar procesos concluidos:\n");
        mostrarScheduler();
    }

    /*
     * PASO 3: Asignar procesadores libres (CPU 1 y CPU 2) a los procesos en "Listo"
     * de mayor prioridad (menor valor numérico), pasándolos a estado "Esperando".
     */
    while (1) {
        int cpu1_ocupada = 0;
        int cpu2_ocupada = 0;

        /* Verificar qué núcleos están actualmente ocupados */
        for (int i = 0; i < TAM_SCHEDULER; i++) {
            if (scheduling[i] != NULL) {
                if (scheduling[i]->procesador == 1) {
                    cpu1_ocupada = 1;
                } else if (scheduling[i]->procesador == 2) {
                    cpu2_ocupada = 1;
                }
            }
        }

        /* Si no hay núcleos libres, salir del bucle */
        if (cpu1_ocupada && cpu2_ocupada) {
            break;
        }

        /* Buscar el proceso en estado "Listo" con mayor prioridad (menor valor numérico) */
        Proceso *candidato = NULL;
        for (int i = 0; i < TAM_SCHEDULER; i++) {
            if (scheduling[i] != NULL && strcmp(scheduling[i]->estado, ESTADO_LISTO) == 0) {
                if (candidato == NULL || scheduling[i]->prioridad < candidato->prioridad) {
                    candidato = scheduling[i];
                }
            }
        }

        /* Si no quedan procesos en estado "Listo", salir del bucle */
        if (candidato == NULL) {
            break;
        }

        /* Asignar la primera CPU libre disponible y cambiar a "Esperando" */
        strncpy(candidato->estado, ESTADO_ESPERANDO, ESTADO_MAX_LEN - 1);
        candidato->estado[ESTADO_MAX_LEN - 1] = '\0';

        if (!cpu1_ocupada) {
            candidato->procesador = 1;
        } else {
            candidato->procesador = 2;
        }

        printf("[*] Proceso ID %d (Prioridad %d) pasó a 'Esperando' asignado al Núcleo CPU %d.\n",
               candidato->id_proceso, candidato->prioridad, candidato->procesador);
    }

    printf("\n---> Estado final del Scheduler tras completar el ciclo:\n");
    mostrarScheduler();
}

void mostrarScheduler(void) {
    int activos = 0;
    printf("===============================================================================\n");
    printf(" ESTADO ACTUAL DEL SCHEDULER  |  Próximo índice de inserción (cursor): [%d]    \n", cursor);
    printf("===============================================================================\n");

    for (int i = 0; i < TAM_SCHEDULER; i++) {
        if (scheduling[i] != NULL) {
            activos++;
        }
        proceso_mostrar(scheduling[i], i);
    }

    printf("-------------------------------------------------------------------------------\n");
    printf("Total de procesos activos en cola: %d / %d\n", activos, TAM_SCHEDULER);
    printf("===============================================================================\n");
}

void scheduler_limpiar(void) {
    for (int i = 0; i < TAM_SCHEDULER; i++) {
        if (scheduling[i] != NULL) {
            proceso_destruir(&scheduling[i]);
        }
    }
}
