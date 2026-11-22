#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "scheduler.h"
#include "archivo.h"

/*
 * Limpia el buffer de entrada estándar consumiendo caracteres hasta un salto
 * de línea o el fin de archivo.
 */
static void limpiar_buffer_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* Descartar */
    }
}

/*
 * Muestra el menú de opciones disponible para el usuario.
 */
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
 * Finaliza la ejecución del programa de forma prolija:
 * - Muestra el listado de procesos terminados registrado en archivo (consigna).
 * - Libera la memoria de todos los procesos restantes en el planificador.
 */
static void salir(void) {
    printf("\n\nCerrando simulación del Scheduler...\n");

    /* La consigna requiere que al finalizar la ejecución se liste el archivo */
    listarFile();

    scheduler_limpiar();
    printf("Memoria liberada correctamente. ¡Hasta luego!\n\n");
    exit(0);
}

int main(void) {
    /* Inicializar la semilla para los números aleatorios */
    srand((unsigned int)time(NULL));

    /* Inicializar el planificador */
    scheduler_init();

    printf("\n<<< Bienvenido/a a la simulación del Planificador de Procesos (Scheduler) >>>\n");

    while (1) {
        menuPrincipal();

        int opcion = 0;
        int leidos = scanf("%d", &opcion);

        if (leidos == EOF) {
            printf("\nFin de entrada detectado (EOF).\n");
            salir();
        }

        if (leidos != 1) {
            printf("\n[!] Entrada no válida. Por favor, ingrese un número del 1 al 5.\n");
            limpiar_buffer_stdin();
            continue;
        }

        /* Limpiar el resto de la línea */
        limpiar_buffer_stdin();

        if (opcion < 1 || opcion > 5) {
            printf("\n[!] Opción fuera de rango. Por favor elija una opción válida (1 al 5).\n");
            continue;
        }

        /* Limpiar pantalla para presentación limpia */
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
