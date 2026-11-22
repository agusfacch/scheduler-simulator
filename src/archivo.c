#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "archivo.h"

int archivo_guardar_terminado(const Proceso *p) {
    if (p == NULL) {
        return -1;
    }

    FILE *fp = fopen(ARCHIVO_TERMINADOS, "a");
    if (fp == NULL) {
        perror("Error al abrir el archivo de procesos terminados para escritura");
        return -1;
    }

    fprintf(fp, "--------------------------------------------------\n");
    fprintf(fp, "ID de Proceso : %d\n", p->id_proceso);
    if (p->procesador == 0) {
        fprintf(fp, "Procesador    : Sin procesador asignado\n");
    } else {
        fprintf(fp, "Procesador    : Núcleo CPU %d\n", p->procesador);
    }
    fprintf(fp, "Prioridad     : %d\n", p->prioridad);
    fprintf(fp, "Estado final  : %s\n", p->estado);
    fprintf(fp, "--------------------------------------------------\n");

    fclose(fp);
    return 0;
}

void listarFile(void) {
    printf("\n=======================================================\n");
    printf("        REGISTRO DE PROCESOS TERMINADOS EN ARCHIVO      \n");
    printf("=======================================================\n");

    FILE *fp = fopen(ARCHIVO_TERMINADOS, "r");
    if (fp == NULL) {
        if (errno == ENOENT) {
            printf("Aún no existe el archivo '%s' (no se registraron procesos terminados).\n", ARCHIVO_TERMINADOS);
        } else {
            perror("No se pudo abrir el archivo de terminados");
        }
        return;
    }

    int primer_caracter = fgetc(fp);
    if (primer_caracter == EOF) {
        printf("El archivo '%s' existe pero se encuentra vacío.\n", ARCHIVO_TERMINADOS);
        fclose(fp);
        return;
    }

    /* Imprimir el primer carácter leído y luego el resto */
    putchar(primer_caracter);
    int c;
    while ((c = fgetc(fp)) != EOF) {
        putchar(c);
    }

    fclose(fp);
    printf("=======================================================\n\n");
}
