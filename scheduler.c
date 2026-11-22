/**
 * ============================================================================
 * TRABAJO FINAL - TALLER DE PROGRAMACIÓN II (2025)
 * Instituto Politécnico Superior - Dto. de Informática
 * Simulación de un Planificador de Procesos de CPU (Scheduler)
 * ============================================================================
 *
 * Características:
 * - Cola circular de tamaño 10 para evitar sesgo hacia la izquierda del buffer.
 * - Simulación de multiprogramación en un equipo con 2 procesadores (núcleos).
 * - Ciclo de vida de procesos:
 *     Nuevo -> Listo -> Esperando (asignado a CPU 1 o 2) -> Corriendo -> Terminado
 * - Planificación basada en prioridades para procesos en estado "Listo".
 * - Persistencia de procesos terminados en archivo "terminados.txt".
 * - Manejo robusto de errores de memoria (malloc) y archivos (I/O).
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define TAM_SCHEDULER       10
#define NUM_CPUS            2
#define ESTADO_MAX_LEN      16
#define ARCHIVO_TERMINADOS  "terminados.txt"

#define ID_PROCESO_MIN      500
#define ID_PROCESO_MAX      5000

#define ESTADO_NUEVO        "Nuevo"
#define ESTADO_LISTO        "Listo"
#define ESTADO_ESPERANDO    "Esperando"
#define ESTADO_CORRIENDO    "Corriendo"
#define ESTADO_TERMINADO    "Terminado"

/* Estructura del Proceso requerida por la consigna */
typedef struct proceso {
    int procesador;                 /* Núcleo asignado: 0 = sin asignar, 1 o 2 */
    int id_proceso;                 /* Identificador único [500, 5000] */
    int prioridad;                  /* Prioridad: a menor número, mayor prioridad */
    char estado[ESTADO_MAX_LEN];    /* Estado actual del proceso */
} Proceso;

/* Vector donde residen los procesos activos en memoria */
static Proceso *scheduling[TAM_SCHEDULER] = { NULL };

/* Contador global para asignación de prioridades consecutivas */
static int next_priority = 0;

/*
 * Cursor de inserción circular: marca el índice a partir del cual se busca
 * el próximo espacio libre para evitar sobrecargar los primeros índices.
 */
static int cursor = 0;

/* Prototipos de funciones requeridos por la consigna */
void asignaEstado(Proceso *p);
void ingresaProceso(void);
int terminaProceso(int indice);
void recorreCola(void);
void mostrarScheduler(void);
void listarFile(void);

/* Funciones auxiliares */
static void menuPrincipal(void);
static void salir(void);
static void limpiarBufferStdin(void);
static int idEstaDuplicado(int id);
static int generarIdUnico(void);
static void liberarScheduler(void);
static int contarProcesosActivos(void);

/* ============================================================================
 * FUNCIÓN PRINCIPAL (MAIN)
 * ============================================================================ */
int main(void) {
    srand((unsigned int)time(NULL));

    printf("<<< Bienvenido/a a la simulación del Planificador de Procesos (Scheduler) >>>\n");

    while (1) {
        menuPrincipal();

        int opcion = 0;
        int leidos = scanf("%d", &opcion);

        if (leidos == EOF) {
            printf("\nFin de entrada detectado. Saliendo...\n");
            salir();
        }

        if (leidos != 1) {
            printf("\n[!] Entrada no válida. Por favor ingrese un número del 1 al 5.\n");
            limpiarBufferStdin();
            continue;
        }

        limpiarBufferStdin();

        if (opcion < 1 || opcion > 5) {
            printf("\n[!] Opción incorrecta. Elija una opción entre 1 y 5.\n");
            continue;
        }

        /* Limpieza de pantalla para interfaz limpia */
        printf("\033[2J\033[H");

        switch (opcion) {
            case 1:
                ingresaProceso();
                printf("\n");
                mostrarScheduler();
                break;

            case 2:
                recorreCola();
                break;

            case 3:
                mostrarScheduler();
                break;

            case 4:
                listarFile();
                break;

            case 5:
                salir();
                break;

            default:
                break;
        }
    }

    return 0;
}

/* ============================================================================
 * IMPLEMENTACIÓN DE FUNCIONES DE LA CONSIGNA
 * ============================================================================ */

/* Menú principal de la consola */
static void menuPrincipal(void) {
    printf("\n============================================\n");
    printf("          SIMULADOR DE SCHEDULER            \n");
    printf("        Taller de Programación II           \n");
    printf("============================================\n");
    printf("1. Ingresar proceso\n");
    printf("2. Recorrer cola (avanzar ciclo de estados)\n");
    printf("3. Mostrar estado del Scheduler\n");
    printf("4. Mostrar historial de procesos terminados\n");
    printf("5. Salir del programa\n");
    printf("--------------------------------------------\n");
    printf("Seleccione una opción [1-5]: ");
}

/*
 * Asigna el siguiente estado según el orden del ciclo de vida:
 * Nuevo -> Listo -> Esperando -> Corriendo -> Terminado.
 * (El paso Listo -> Esperando es mediado por la disponibilidad de CPU).
 */
void asignaEstado(Proceso *p) {
    if (p == NULL) return;

    if (strcmp(p->estado, ESTADO_NUEVO) == 0) {
        strncpy(p->estado, ESTADO_LISTO, ESTADO_MAX_LEN - 1);
    } else if (strcmp(p->estado, ESTADO_ESPERANDO) == 0) {
        strncpy(p->estado, ESTADO_CORRIENDO, ESTADO_MAX_LEN - 1);
    } else if (strcmp(p->estado, ESTADO_CORRIENDO) == 0) {
        strncpy(p->estado, ESTADO_TERMINADO, ESTADO_MAX_LEN - 1);
    }
    p->estado[ESTADO_MAX_LEN - 1] = '\0';
}

/*
 * Ingresa un proceso al Scheduler en el primer espacio libre que encuentre,
 * aplicando el algoritmo de búsqueda circular a partir del cursor.
 */
void ingresaProceso(void) {
    for (int count = 0; count < TAM_SCHEDULER; count++) {
        int i = (cursor + count) % TAM_SCHEDULER;

        if (scheduling[i] == NULL) {
            Proceso *nuevo = (Proceso *)malloc(sizeof(Proceso));
            if (nuevo == NULL) {
                perror("[!] Error fatal: no se pudo asignar memoria para el proceso");
                return;
            }

            next_priority++;
            nuevo->procesador = 0;
            nuevo->id_proceso = generarIdUnico();
            nuevo->prioridad = next_priority;
            strncpy(nuevo->estado, ESTADO_NUEVO, ESTADO_MAX_LEN - 1);
            nuevo->estado[ESTADO_MAX_LEN - 1] = '\0';

            scheduling[i] = nuevo;
            cursor = (i + 1) % TAM_SCHEDULER; /* Avanza el cursor circular */

            printf("\n[+] Proceso ingresado exitosamente en slot [%d]:\n", i);
            printf("    ID: %d | Prioridad: %d | Estado: %s | CPU: %d\n",
                   nuevo->id_proceso, nuevo->prioridad, nuevo->estado, nuevo->procesador);
            return;
        }
    }

    printf("\n[!] ¡No hay espacios disponibles en el Scheduler! (Capacidad máxima: %d procesos)\n", TAM_SCHEDULER);
}

/*
 * Quita el proceso de la cola liberando su memoria, guardándolo en el archivo
 * persistente y retornando el índice del lugar liberado.
 */
int terminaProceso(int indice) {
    if (indice < 0 || indice >= TAM_SCHEDULER || scheduling[indice] == NULL) {
        return -1;
    }

    FILE *fp = fopen(ARCHIVO_TERMINADOS, "a");
    if (fp == NULL) {
        perror("[!] Error al abrir el archivo de terminados para registro");
    } else {
        fprintf(fp, "--------------------------------------------------\n");
        fprintf(fp, "ID de Proceso : %d\n", scheduling[indice]->id_proceso);
        if (scheduling[indice]->procesador == 0) {
            fprintf(fp, "Procesador    : Sin procesador asignado\n");
        } else {
            fprintf(fp, "Procesador    : Núcleo CPU %d\n", scheduling[indice]->procesador);
        }
        fprintf(fp, "Prioridad     : %d\n", scheduling[indice]->prioridad);
        fprintf(fp, "Estado final  : %s\n", scheduling[indice]->estado);
        fprintf(fp, "--------------------------------------------------\n");
        fclose(fp);
    }

    free(scheduling[indice]);
    scheduling[indice] = NULL;

    return indice;
}

/*
 * Recorre todos los procesos de la cola haciendo el cambio de estado de los mismos,
 * archivando los finalizados y asignando procesadores según prioridad.
 */
void recorreCola(void) {
    if (contarProcesosActivos() == 0) {
        printf("\n[!] El Scheduler está vacío. ¡Nada para recorrer!\n\n");
        return;
    }

    printf("\n---> Estado del Scheduler antes de la transición:\n");
    mostrarScheduler();

    /* Paso 1: Transición de ciclo de estados */
    for (int i = 0; i < TAM_SCHEDULER; i++) {
        if (scheduling[i] != NULL) {
            asignaEstado(scheduling[i]);
        }
    }

    printf("\n---> Transición de ciclo aplicada:\n");
    mostrarScheduler();

    /* Paso 2: Liberar y registrar en archivo los procesos en "Terminado" */
    int terminados_en_ciclo = 0;
    for (int i = 0; i < TAM_SCHEDULER; i++) {
        if (scheduling[i] != NULL && strcmp(scheduling[i]->estado, ESTADO_TERMINADO) == 0) {
            int id_liberado = scheduling[i]->id_proceso;
            int slot = terminaProceso(i);
            printf("[-] Proceso ID %d en slot [%d] finalizó y fue archivado.\n", id_liberado, slot);
            terminados_en_ciclo++;
        }
    }

    if (terminados_en_ciclo > 0) {
        printf("\n---> Estado tras finalizar procesos concluidos:\n");
        mostrarScheduler();
    }

    /* Paso 3: Asignar procesadores libres (1 y 2) por prioridad a procesos en "Listo" */
    while (1) {
        int cpu1_ocupada = 0;
        int cpu2_ocupada = 0;

        for (int i = 0; i < TAM_SCHEDULER; i++) {
            if (scheduling[i] != NULL) {
                if (scheduling[i]->procesador == 1) cpu1_ocupada = 1;
                else if (scheduling[i]->procesador == 2) cpu2_ocupada = 1;
            }
        }

        if (cpu1_ocupada && cpu2_ocupada) {
            break; /* Ambos núcleos están en uso */
        }

        Proceso *candidato = NULL;
        for (int i = 0; i < TAM_SCHEDULER; i++) {
            if (scheduling[i] != NULL && strcmp(scheduling[i]->estado, ESTADO_LISTO) == 0) {
                if (candidato == NULL || scheduling[i]->prioridad < candidato->prioridad) {
                    candidato = scheduling[i];
                }
            }
        }

        if (candidato == NULL) {
            break; /* No hay más procesos esperando CPU en "Listo" */
        }

        strncpy(candidato->estado, ESTADO_ESPERANDO, ESTADO_MAX_LEN - 1);
        candidato->estado[ESTADO_MAX_LEN - 1] = '\0';
        candidato->procesador = (!cpu1_ocupada) ? 1 : 2;

        printf("[*] Proceso ID %d (Prioridad %d) pasó a 'Esperando' asignado al Núcleo CPU %d.\n",
               candidato->id_proceso, candidato->prioridad, candidato->procesador);
    }

    printf("\n---> Estado final del Scheduler tras completar el ciclo:\n");
    mostrarScheduler();
}

/* Lista los procesos de la cola */
void mostrarScheduler(void) {
    int activos = 0;
    printf("===============================================================================\n");
    printf(" ESTADO ACTUAL DEL SCHEDULER  |  Próximo índice de inserción (cursor): [%d]    \n", cursor);
    printf("===============================================================================\n");

    for (int i = 0; i < TAM_SCHEDULER; i++) {
        if (scheduling[i] != NULL) {
            activos++;
            printf("[%d] ---> Proceso: %-5d | Procesador: %-2d | Prioridad: %-3d | Estado: %s\n",
                   i, scheduling[i]->id_proceso, scheduling[i]->procesador, scheduling[i]->prioridad, scheduling[i]->estado);
        } else {
            printf("[%d] ---> [VACÍO]\n", i);
        }
    }

    printf("-------------------------------------------------------------------------------\n");
    printf("Total de procesos activos en cola: %d / %d\n", activos, TAM_SCHEDULER);
    printf("===============================================================================\n");
}

/* Lista los procesos registrados en el archivo de texto */
void listarFile(void) {
    printf("\n=======================================================\n");
    printf("        REGISTRO DE PROCESOS TERMINADOS EN ARCHIVO      \n");
    printf("=======================================================\n");

    FILE *fp = fopen(ARCHIVO_TERMINADOS, "r");
    if (fp == NULL) {
        if (errno == ENOENT) {
            printf("Aún no existe el archivo '%s' (no hay procesos terminados registrados).\n", ARCHIVO_TERMINADOS);
        } else {
            perror("[!] No se pudo abrir el archivo de terminados");
        }
        return;
    }

    int c = fgetc(fp);
    if (c == EOF) {
        printf("El archivo '%s' se encuentra vacío.\n", ARCHIVO_TERMINADOS);
        fclose(fp);
        return;
    }

    putchar(c);
    while ((c = fgetc(fp)) != EOF) {
        putchar(c);
    }

    fclose(fp);
    printf("=======================================================\n\n");
}

/* ============================================================================
 * FUNCIONES AUXILIARES
 * ============================================================================ */

static void salir(void) {
    printf("\n\nCerrando simulación del Scheduler...\n");

    /* Requisito de la consigna: listar archivo al finalizar */
    listarFile();

    liberarScheduler();
    printf("Memoria dinámica liberada correctamente. ¡Hasta luego!\n\n");
    exit(0);
}

static void limpiarBufferStdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* Descartar */
    }
}

static int idEstaDuplicado(int id) {
    for (int i = 0; i < TAM_SCHEDULER; i++) {
        if (scheduling[i] != NULL && scheduling[i]->id_proceso == id) {
            return 1;
        }
    }
    return 0;
}

static int generarIdUnico(void) {
    int intentos = 0;
    int id;
    int rango = (ID_PROCESO_MAX - ID_PROCESO_MIN + 1);

    do {
        id = ID_PROCESO_MIN + (rand() % rango);
        intentos++;
    } while (idEstaDuplicado(id) && intentos < 100);

    return id;
}

static void liberarScheduler(void) {
    for (int i = 0; i < TAM_SCHEDULER; i++) {
        if (scheduling[i] != NULL) {
            free(scheduling[i]);
            scheduling[i] = NULL;
        }
    }
}

static int contarProcesosActivos(void) {
    int c = 0;
    for (int i = 0; i < TAM_SCHEDULER; i++) {
        if (scheduling[i] != NULL) c++;
    }
    return c;
}
