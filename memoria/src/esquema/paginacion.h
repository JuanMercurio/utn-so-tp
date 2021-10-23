#ifndef _ESQUEMA_PAGINACION_
#define _ESQUEMA_PAGINACION_

#include <commons/collections/list.h>

void* ram;

/* Lista con todas las tablas de paginas de los procesos */
t_list* tablas;

/* Bitmap de marcos */
int* marcos; 

/* Esructura para tablas */
typedef struct tabla_paginas{
    int pid;
    t_list* tabla_pag;
    t_list* allocs;
}tabla_paginas;

/* Registros de las tablas de paginas */
typedef struct t_pag{
    int marco;
    int en_memoria;
}t_pag;

/*
    @NAME:  iniciar_paginacion
    @DESC:  inicia todas las estructuras necesarias 
            para utilizar paginacion
 */
void iniciar_paginacion();

/*
    @NAME:  init_ram
    @DESC:  aloca la memoria principal
 */
void init_ram();

/*
    @NAME:  init_bitmap_frames
    @DESC:  iniciar el bitmap de frames de la memoria fisica
 */
void init_bitmap_frames();

#endif