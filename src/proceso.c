#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proceso.h"

Proceso *proceso_crear(int id, int prioridad) {
    Proceso *p = (Proceso *)malloc(sizeof(Proceso));
    if (p == NULL) {
        perror("Error de asignación de memoria para nuevo proceso");
        return NULL;
    }

    p->procesador = 0;
    p->id_proceso = id;
    p->prioridad = prioridad;
    strncpy(p->estado, ESTADO_NUEVO, ESTADO_MAX_LEN - 1);
    p->estado[ESTADO_MAX_LEN - 1] = '\0';

    return p;
}

void asignaEstado(Proceso *p) {
    if (p == NULL) {
        return;
    }

    /*
     * Transiciones automáticas de ciclo de vida:
     * - "Nuevo" pasa a "Listo"
     * - "Esperando" (que ya tiene CPU asignada) pasa a "Corriendo"
     * - "Corriendo" pasa a "Terminado"
     * Nota: La transición de "Listo" a "Esperando" se realiza en el scheduler
     * al asignar una CPU libre según orden de prioridad.
     */
    if (strcmp(p->estado, ESTADO_NUEVO) == 0) {
        strncpy(p->estado, ESTADO_LISTO, ESTADO_MAX_LEN - 1);
    } else if (strcmp(p->estado, ESTADO_ESPERANDO) == 0) {
        strncpy(p->estado, ESTADO_CORRIENDO, ESTADO_MAX_LEN - 1);
    } else if (strcmp(p->estado, ESTADO_CORRIENDO) == 0) {
        strncpy(p->estado, ESTADO_TERMINADO, ESTADO_MAX_LEN - 1);
    }
    p->estado[ESTADO_MAX_LEN - 1] = '\0';
}

void proceso_destruir(Proceso **p) {
    if (p != NULL && *p != NULL) {
        free(*p);
        *p = NULL;
    }
}

void proceso_mostrar(const Proceso *p, int slot) {
    if (p == NULL) {
        printf("[%d] ---> [VACÍO]\n", slot);
        return;
    }

    printf("[%d] ---> Proceso: %-5d | Procesador: %-2d | Prioridad: %-3d | Estado: %s\n",
           slot, p->id_proceso, p->procesador, p->prioridad, p->estado);
}
