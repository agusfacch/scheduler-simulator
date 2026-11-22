# Simulador de Planificador de Procesos (CPU Scheduler)

**Trabajo Práctico Final — Taller de Programación II (2025)**  
*Instituto Politécnico Superior — Analista Universitario en Sistemas*

---

## 📋 Descripción del Proyecto

Este proyecto es una simulación en lenguaje C de un **Planificador de Procesos (CPU Scheduler)** de un sistema operativo sobre una arquitectura con **2 núcleos de procesamiento (CPU 1 y CPU 2)** y capacidad para hasta **10 procesos simultáneos** en memoria principal.

El planificador administra el ciclo de vida de los procesos, asigna tiempos de procesador según un esquema de **prioridades**, evita la inanición y el sesgo de inserción mediante un **algoritmo de búsqueda circular**, y persiste el historial de procesos finalizados en un archivo de texto (`terminados.txt`).

---

## ⚙️ Especificaciones Técnicas y Ciclo de Vida

### 1. Estructura del Proceso
```c
typedef struct proceso {
    int procesador;                 /* Núcleo asignado: 0 = sin procesador, 1 o 2 */
    int id_proceso;                 /* Identificador único [500, 5000] */
    int prioridad;                  /* Prioridad: menor valor = mayor prioridad */
    char estado[ESTADO_MAX_LEN];    /* Estado actual del proceso */
} Proceso;
```

### 2. Transición de Estados
La evolución de cada proceso sigue estrictamente el ciclo establecido en la consigna:
$$\text{Nuevo} \longrightarrow \text{Listo} \longrightarrow \text{Esperando} \longrightarrow \text{Corriendo} \longrightarrow \text{Terminado}$$

- **Nuevo**: Cuando recién se crea e ingresa a la cola.
- **Listo**: En espera de que un núcleo de CPU esté libre.
- **Esperando**: Cuando se le asigna un procesador libre (CPU 1 o 2) en función de su prioridad.
- **Corriendo**: En ejecución activa dentro del procesador asignado. Solo puede haber hasta 2 procesos en este estado simultáneamente.
- **Terminado**: Al concluir su ejecución. Se retira del scheduler, libera su slot en la cola y se guarda su registro en `terminados.txt`.

### 3. Búsqueda Circular (Sin Sesgo a la Izquierda)
Para evitar la sobrecarga de los primeros índices del vector (`[0]`, `[1]`), el algoritmo de inserción utiliza un cursor persistente:
- Inicia la búsqueda de un slot libre desde la posición inmediata siguiente a la última inserción.
- Si alcanza el final del vector (`TAM_SCHEDULER - 1`), recomienza circularmente desde el índice `[0]`.
- Garantiza un aprovechamiento equitativo de todo el buffer.

---

## 🗂️ Estructura del Proyecto

El código fuente ha sido modularizado siguiendo las mejores prácticas de desarrollo en C:

```
.
├── Makefile                     # Automatización de compilación y limpieza
├── README.md                    # Documentación detallada del proyecto
├── Scheduler - Final 2025.pdf   # Consigna original de la cátedra
├── scheduler.c                  # Versión consolidada autocontenida
├── terminados.txt               # Registro histórico de procesos finalizados
├── include/                     # Cabeceras (.h)
│   ├── archivo.h                # Prototipos de persistencia e I/O
│   ├── proceso.h                # Definición de estructuras y prototipos de proceso
│   └── scheduler.h              # Prototipos del planificador y ciclo de vida
└── src/                         # Código fuente modular (.c)
    ├── archivo.c                # Manejo de archivos y lectura segura
    ├── main.c                   # Interfaz de consola y control de flujo
    ├── proceso.c                # Operaciones sobre la entidad Proceso
    └── scheduler.c              # Lógica de la cola circular y asignación de CPU
```

---

## 🛠️ Compilación y Ejecución

### Opción 1: Compilación Modular con `make` (Recomendada)

El proyecto incluye un `Makefile` configurado con flags de advertencia estrictos (`-Wall -Wextra -pedantic -std=c99`):

```bash
# Compilar el proyecto modular
make

# Ejecutar la simulación
make run

# Limpiar archivos objeto y binarios generados
make clean
```

### Opción 2: Compilación de la Versión Modular con `gcc` directo

```bash
gcc -Wall -Wextra -pedantic -std=c99 -Iinclude src/*.c -o scheduler
./scheduler
```

### Opción 3: Compilación de la Versión Consolidada (`scheduler.c`)

Si se requiere entregar o compilar un único archivo fuente `.c`:

```bash
gcc -Wall -Wextra -pedantic -std=c99 scheduler.c -o scheduler
./scheduler
```

---

## 🛡️ Mejoras de Robustez y Calidad Implementadas

1. **Gestión Segura de Memoria**:
   - Validación inmediata de retornos de `malloc()`.
   - Función `proceso_destruir()` que libera memoria y anula punteros (`NULL`) para prevenir *dangling pointers*.
   - Función `scheduler_limpiar()` ejecutada al salir del programa para garantizar 0 fugas de memoria (*zero memory leaks*).

2. **Control Robusto de Entrada/Salida**:
   - Detección y manejo de fin de archivo (`EOF` / `Ctrl+D`) en el menú interactivo para evitar bucles infinitos.
   - Vaciado seguro del búfer `stdin` ante ingresos no numéricos o caracteres inesperados.
   - Validación segura de apertura de archivos (`fopen`) con reporte informativo mediante `perror` y verificación de `errno`.
   - Se eliminó el riesgo de invocar `fclose(NULL)`.

3. **Garantía de Identificadores Únicos**:
   - Generación de identificadores de proceso aleatorios en el rango $[500, 5000]$ verificando que no colisionen con procesos activos en cola.

4. **Persistencia Garantizada**:
   - De acuerdo a la consigna, los procesos terminados se archivan ordenadamente con sus datos (ID, procesador utilizado, prioridad y estado) y se listan por pantalla al finalizar el programa.
