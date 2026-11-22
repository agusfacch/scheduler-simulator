#ifndef ARCHIVO_H
#define ARCHIVO_H

#include "proceso.h"

#define ARCHIVO_TERMINADOS "terminados.txt"

/*
 * Registra un proceso finalizado en el archivo de texto.
 * Retorna 0 en éxito, -1 si ocurrió un error al abrir/escribir el archivo.
 */
int archivo_guardar_terminado(const Proceso *p);

/*
 * Lista en consola todos los procesos registrados en el archivo.
 * Requerido por la consigna académica.
 */
void listarFile(void);

#endif /* ARCHIVO_H */
