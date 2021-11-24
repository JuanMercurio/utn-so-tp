#include "algoritmos.h"

#include <stdlib.h>
#include <utils/utils.h>

int LRU_C = 0;
int LRU_TLB = 0;
int FIFO_TLB = 0;

t_clock p_clock;


t_victima lru_dinamico(int pid, tab_pags* tabla)
{
    pag_t* pagina;
    int LRU_min = LRU_C, n_pagina, pid_victima;

    for(int i=0; i<list_size(tablas.lista); i++)
    {
        tab_pags* t = list_get(tablas.lista, i);
        for(int j=0; j<list_size(t->tabla_pag); j++)
        {
            pag_t* registro = list_get(t->tabla_pag, j);
            if(registro->LRU < LRU_min){
                LRU_min     = registro->LRU;
                pagina      = registro;
                n_pagina    = j;
                pid_victima = t->pid;
                
            }
        }
    }

    pagina->presente = 0;

    t_victima victima;    
    victima.marco      = pagina->marco;
    victima.modificado = pagina->modificado;
    victima.pid        = pid_victima;
    victima.pagina     = n_pagina;

    return victima;
}

t_victima lru_fijo(int pid, tab_pags* tabla){
    
    return lru_en_pag_table(tabla);
}

t_victima lru_en_pag_table(tab_pags* tabla){

    int LRU_min = LRU_C, n_pagina;
    pag_t* pagina; 

    for(int i=0; i<list_size(tabla->tabla_pag); i++){

        pag_t* registro = list_get(tabla->tabla_pag, i);
        if(registro->LRU < LRU_min){
            LRU_min  = registro->LRU;
            pagina   = registro;
            n_pagina = i;
            
        }
    }
    
    pagina->presente = 0;

    t_victima victima;    
    victima.marco      = pagina->marco;
    victima.modificado = pagina->modificado;
    victima.pid        = tabla->pid;
    victima.pagina     = n_pagina;

    return victima;
}

t_victima clock_fijo(int pid, tab_pags* tabla)
{
    if(tabla->p_clock == -1) tabla->p_clock = 0;
    
    int pagina = -1;
    while(pagina != -1)
    {
       pagina = clock_buscar_00(tabla);
       if(pagina != -1) break;

       pagina = clock_buscar_01(tabla);
    }

    pag_t* reg = list_get(tabla->tabla_pag, pagina);
    reg->presente = 0;

    t_victima victima;    
    victima.marco      = reg->marco;
    victima.modificado = reg->modificado;
    victima.pagina     = pagina;
    victima.pid        = pid;

    return victima;
}

t_victima clock_dinamico(int pid, tab_pags* tabla)
{
    int posicion = clock_buscar_puntero();
    int cantidad_procesos = list_size(tablas.lista);
    tab_pags* t; 
    int pagina = -1;
    while(pagina != -1)
    {
        t = list_get(tablas.lista, posicion);

        pagina = clock_buscar_00(t);
        if(pagina != -1) break;

        pagina = clock_buscar_01(t);
        if(pagina != -1) break;

        if(posicion++ == cantidad_procesos) 
            posicion = 0;
        else
            posicion++;

        t->p_clock = -1;
    }

    pag_t* p = list_get(t->tabla_pag, pagina);

    t_victima victima;

    victima.marco      =  p->marco;
    victima.modificado =  p->modificado;
    victima.pagina     =  pagina;
    victima.pid        =  t->pid;

    return victima;
}

int clock_buscar_00(tab_pags* tabla)
{
    int i;

    if(tabla->p_clock == -1) 
        i = 0;
    else
        i = tabla->p_clock;

    int iteracion = 0;
    int tamanio = list_size(tabla->tabla_pag);

    while(iteracion != tamanio)
    {
        if(i == list_size(tabla->tabla_pag)) i = 0;
        
        pag_t* reg = list_get(tabla->tabla_pag, tabla->p_clock);

        if(reg->presente != 1) continue;

        if(reg->modificado == 0 && reg->algoritmo == 0) 
        {
            if(i++ == tamanio)
                tabla->p_clock = 0;
            else
            tabla->p_clock = i++;
            
            return i;
        }

       if(i++ == tamanio) 
            i = 0;
        else
            i++;
        
        iteracion++;
    }

    return -1;
}

int clock_buscar_01(tab_pags* tabla)
{
    int i;

    if(tabla->p_clock == -1) 
        i = 0;
    else
        i = tabla->p_clock;

    int iteracion = 0;
    int tamanio = list_size(tabla->tabla_pag);

    while(iteracion != tamanio)
    {
        if(i == list_size(tabla->tabla_pag)) i = 0;
        
        pag_t* reg = list_get(tabla->tabla_pag, tabla->p_clock);

        if(reg->presente != 1) continue;

        /* codigo horrible  */
        if(reg->modificado == 1 && reg->algoritmo == 0) 
        {
            if(i++ == tamanio)
                tabla->p_clock = 0;
            else
                tabla->p_clock = i++;
                
            return i;
        }
        else
            reg->algoritmo = 0;


        if(i++ == tamanio) 
            i = 0;
        else
            i++;
        
        iteracion++;
    }

    return -1;
}

int clock_buscar_puntero()
{
    int cantidad_procesos = list_size(tablas.lista);

    for(int i=0; i < cantidad_procesos; i++)
    {
        tab_pags* tabla = list_get(tablas.lista, i);
        if(tabla->p_clock != -1) return i;
    }

    return -1;
}

void page_use_lru(int pagina, tab_pags* tabla)
{
   pag_t* p = list_get(tabla->tabla_pag, pagina);
   p->presente = 1;
   p->algoritmo = suma_atomica(&LRU_C);
}

void page_use_clock_modificado(int pagina, tab_pags* tabla)
{
   pag_t* p = list_get(tabla->tabla_pag, pagina);
   p->presente = 1;
   p->algoritmo = 1;
}