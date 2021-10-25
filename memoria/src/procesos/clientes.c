#include "clientes.h"

#include <matelib/matelib.h>
#include <conexiones/conexiones.h>
#include <utils/utils.h>
#include <sys/socket.h>
#include <pthread.h>

void atender_proceso(void* arg){
    int cliente = *(int*)arg;
    free(arg);

    handshake(cliente, "memoria");
    ejecutar_proceso(cliente);
}



void ejecutar_proceso(int cliente) {

    while(1) //TODO: feo while
    {
        int operacion = recibir_operacion(cliente);
        switch (operacion)
        {
        case NEW_INSTANCE:
            // iniciar_proceso(cliente);
            break;
        
        case MEMALLOC:
            break;

        case MEMFREE:
            break;

        case MEMREAD:
            break;

        case MEMWRITE:
            break;

        default:
            break;
        }
    }
}

// void iniciar_proceso(int cliente){
//     // checkear si puede iniciar
//     // si no puede ser iniciado: pthread_exit(0)
//     int pid = suma_atomica(&id_memoria);
//     mate_instance* meta_proceso =  recibir_instancia(cliente);
//     // mate_instance.pid = pid;

// }