#include "main.h"
#include <mensajes/mensajes.h>

#define VALIDO 0
#define INVALIDO -1


int main(int argc, char* argv[]) {

    memoria_tests(argc, argv[1]);

    // CONECTARSE A MEMORIA
    // CAMBIAR IP Y PUERTO POR LA COFIG. 

    int fd_memoria = crear_conexion("127.0.0.1", "5003");
    printf("memoria %d\n", fd_memoria);
    char* handshake = recibir_mensaje(fd_memoria);
    printf("------------------%s\n", handshake);
    enviar_int(fd_memoria,14);
    char* tipo_asignacion = recibir_mensaje(fd_memoria);
     printf("%s\n", tipo_asignacion);
    asignacionFija = 0;

    if(strcmp(tipo_asignacion,"FIJA") == 0){
        printf("entro a if de FIJA\n");
        asignacionFija = 1;
    }
         printf("%d\n", asignacionFija);
    lista_carpinchos= list_create();
    lista_marcos= list_create();
    //marcos_libres_fija = list_create();
    marcos_libres = list_create();
    lista_pedidos = list_create();
    mutex_lista_pedidos = malloc(sizeof(sem_t));
    agrego_lista_pedidos = malloc(sizeof(sem_t));
    sem_init(mutex_lista_pedidos,0,1);
    sem_init(agrego_lista_pedidos,0,0);
  

   //solo corre si corremos el binario asi: binario test
   tests(argc, argv[1]);

   /* Este modulo funciona sin hilos. Los clientes esperan a ser atendidos en cola  */
   obtener_config();
   
   // MOSTRAR CONFIG
  /* printf("El ip del server va a ser: %s \n", configuracion.IP);   
   printf("El ip del server va a ser: %s \n", configuracion.PUERTO);   
   printf("El ip del server va a ser: %d \n", configuracion.TAMANIO_SWAP);   
   printf("El ip del server va a ser: %d \n", configuracion.TAMANIO_PAGINA);   
   printf("El ip del server va a ser: %d \n", configuracion.RETARDO_SWAP);  
   printf("Los archivos del server van a ser: %s \n", configuracion.ARCHIVOS_SWAP_list[0]);  
   printf("Los archivos del server van a ser: %s \n", configuracion.ARCHIVOS_SWAP_list[1]);
  */
    
    pthread_t tid;
    pthread_create(&tid, NULL, &agregarPedidosMemoria, fd_memoria);
    
    crearArchivos();
    if(asignacionFija){
        marcosLIbresFija();
    }
    else{
        marcosLibes();
    }

 
    while(1){
        sem_wait(agrego_lista_pedidos);
        Pedido* ped; /* = malloc(sizeof(Pedido)); */
        sem_wait(mutex_lista_pedidos);
        ped = (Pedido*) list_remove(lista_pedidos,0);
        sem_post(mutex_lista_pedidos);
        printf(" saco de lista pedido %d\n", ped->pid);
        if(strcmp(ped->nombre_pedido,"ESCRIBIR_PAGINA") == 0){
            printf("ESCRIBIR_PAGINA: pid: %d - pag: %d \n Contenido: %s ",ped->pid, ped->pagina, ped->contenido_pagina);
            int pido;
            if(asignacionFija){
                pido = remplazoPaginaFija(ped->pid, ped->pagina, ped->contenido_pagina);
            }
            else{
                pido = agregarPaginaDinamica(ped->pid, ped->pagina, ped->contenido_pagina);
            }

            printf("Envia a memoria %d\n",pido);
            enviar_int(fd_memoria, pido);
        }
        else if(strcmp(ped->nombre_pedido,"SOLICITUD_INICIO") ==0 ){
            printf("SOLICITUD_INICIO: pid: %d\n", ped->pid);
            bool puede;
            if(asignacionFija){
                // SE FIJA SI HAY ESPACIO O SI YA EXISTE ESE CARPINCHO
                Carpincho_Swamp* car = buscarCarpincho(ped->pid);
                puede = elegirMejorArchivoDOS (ped->pid) != -1 && car->pid == -1;
            }
            else{
                puede = quedaPaginasEnArchivo(ped->pid);
            }
            if (puede){
                 printf("Envia a memoria 1\n");
                enviar_int(fd_memoria,1);
                printf("envioooooo a memoria %d\n", fd_memoria);
            }
            else{ printf("Envia a memoria -1\n");

                enviar_int(fd_memoria,-1);
            }
            
        }
        else if(strcmp(ped->nombre_pedido,"INICIO") == 0){
            printf("INICIO: pid: %d\n", ped->pid);
            int inicio;
            if(asignacionFija){
                inicio = crearCarpinchoFijaDOS(ped->pid);
            }
            else{
                inicio = CrearCarpincho(ped->pid);
            }
            enviar_int(fd_memoria,inicio);

            printf("Envia a memoria %d\n",inicio);

        }
        else if(strcmp(ped->nombre_pedido,"SOLICITUD_PAGINA") == 0 ){
            // puede pedir una pagina
            printf("SOLICITUD_PAGINA: pid: %d\n", ped->pid);
            int error;
            bool puede;
            if(asignacionFija){
                // ¿COMO SERIA?
                // LLEGUO A SU NUMERO DE MARCOS DE MARCOS.
                puede = puedeAgregarPagina( ped->pid);
                error= solicitarPaginaFija(ped->pid, ped->pagina);
            }
            else{
                // ASIGNARLE UN MARCO LIBRE 
                // PASARME EL PID. 
                puede = quedaPaginasEnArchivo(ped->pid);
                error = solicitudPagina(ped->pid, ped->pagina);
                // DEVOLVER AL SOCKET CON EL OPER Y UN -1
            }
            if (puede && error == 1){
                printf("Envia a memoria 1\n");
                enviar_int(fd_memoria,1);
            }
            else{
                printf("Envia a memoria -1\n");

                enviar_int(fd_memoria,-1);
            }
        }
        else if(strcmp(ped->nombre_pedido,"BORRAR_PAGINA") == 0){
            printf("BORRAR_PAGINA: pid: %d pag: %d\n", ped->pid, ped->pagina);
            int pudo;
            if(asignacionFija){
                pudo = BorrarPaginaFija(ped->pid, ped->pagina);
            }
            else{
                pudo = borrarPagina(ped->pid, ped->pagina);
            }
            enviar_int(fd_memoria, pudo);

            printf("Envia a memoria %d\n",pudo);

        }
        else if(strcmp(ped->nombre_pedido,"BORRAR_CARPINCHO") == 0){
            printf("BORRAR_CARPINCHO: pid: %d\n", ped->pid);
            int error;
            if(asignacionFija){
              error = borrarCarpinchoFija(ped->pid);
            }
            else{
                error =borrarCarpincho(ped->pid);
            }
            enviar_int(fd_memoria, error);

            printf("Envia a memoria %d\n",error);

        }
        else if(strcmp(ped->nombre_pedido,"OBTENER_PAGINA") == 0){
            printf("OBTENER_PAGINA: pid: %d pag: %d\n", ped->pid, ped->pagina);
           char* cont_pag;
            if(asignacionFija){
                cont_pag =  buscarPaginaFija(ped->pid,ped->pagina);
            }
            else{
                cont_pag =  buscarPaginaDinamico(ped->pid,ped->pagina);
                
            }
            if ( strcmp(cont_pag,"")== 0){
                printf("Envia a memoria -1\n");
                printf("NO ENCONTRO LA PAGINA\n");
                enviar_int(fd_memoria, -1);
            }
            else{
                
                enviar_int(fd_memoria, configuracion.TAMANIO_PAGINA);
                enviar_mensaje(fd_memoria, cont_pag);
                printf("Envia a memoria: tam pagina: %d - contenido pagina: %s\n",configuracion.TAMANIO_PAGINA,cont_pag);

            }
    
        }
        
        
        sleep(configuracion.RETARDO_SWAP);
    }
    
   
    destroy_and_free(fd_memoria);
    // CONEXIONES
    // SE PUEDE BORRAR ESTO?
    pthread_join(tid, NULL);
   return 0;
}
void destroy_and_free ( int fd_memoria){
    list_destroy(lista_carpinchos);
    list_destroy(lista_marcos);

    //marcos_libres_fija = list_create();
    list_destroy(marcos_libres);
    list_destroy(lista_pedidos);
    sem_destroy(mutex_lista_pedidos);
    sem_destroy(agrego_lista_pedidos);
    close(fd_memoria);
}
void pruebas(){
    if ( asignacionFija){
        
        if(quedaEspacioEnArchivoDOS()){
           printf("hay espacio\n");
        }
        //printf("La cantidad del archivo 0 es: %d\n",cantidadBarra0File (configuracion.ARCHIVOS_SWAP_list[0]));
        //printf("La cantidad del archivo 1 es: %d\n",cantidadBarra0File (configuracion.ARCHIVOS_SWAP_list[1]));
        int error;
        crearCarpinchoFijaDOS(1);
        crearCarpinchoFijaDOS(2);
        crearCarpinchoFijaDOS(3);
        crearCarpinchoFijaDOS(4);
        crearCarpinchoFijaDOS(5);
        //mostrarCarpinchos();
        char* basura = string_repeat('n',configuracion.TAMANIO_PAGINA);
        int pudo=agregarPaginaFija(1,2,basura);
        
        basura = string_repeat('k',configuracion.TAMANIO_PAGINA);
        pudo=agregarPaginaFija(2,2,basura);
        
        for ( int i = 0; i<13;i++){
        basura = string_repeat('4',configuracion.TAMANIO_PAGINA);
        pudo=agregarPaginaFija(4,i,basura);
        }

        basura = string_repeat('A',configuracion.TAMANIO_PAGINA);
        pudo=agregarPaginaFija(4,3,basura);
        //mostrarCarpinchos();
        pudo=agregarPaginaFija(8,1,basura);

        //mostrarCarpinchos();
        char* pal = buscarPaginaFija(2,1);
        printf("pagina 1:%s\n",pal);
        
        pal = buscarPaginaFija(4,1);
        if (strcmp (pal,"")){
            printf("pagina 1:PAGINA NO ENCONTRADA\n");
        }
        pal = buscarPaginaFija(2,2);
        printf("pagina 1:%s\n",pal);
        BorrarPaginaFija(1,2);
        //int error = borrarCarpinchoFija(2);
        error = borrarCarpinchoFija(5);
        pal = buscarPaginaFija(4,3);
        printf("pagina 1:%s\n",pal);
        crearCarpinchoFijaDOS(6);
        crearCarpinchoFijaDOS(7);
        crearCarpinchoFijaDOS(8);
        crearCarpinchoFijaDOS(9);
        crearCarpinchoFijaDOS(10);
        crearCarpinchoFijaDOS(11);
        crearCarpinchoFijaDOS(12);
        crearCarpinchoFijaDOS(13);
        crearCarpinchoFijaDOS(14);
        crearCarpinchoFijaDOS(15);
        mostrarCarpinchos();
    }
    else{
        
        CrearCarpincho(1);
        CrearCarpincho(2);
        //mostrarCarpinchos();
        CrearCarpincho(4);
        char* basura = string_repeat('n',configuracion.TAMANIO_PAGINA);
        int error =agregarPaginaDinamica(2,2,basura);
        basura = string_repeat('K',configuracion.TAMANIO_PAGINA);
        error =agregarPaginaDinamica(1,5,basura);
        basura = string_repeat('G',configuracion.TAMANIO_PAGINA);
        error =agregarPaginaDinamica(1,2,basura);
        basura = string_repeat('A',configuracion.TAMANIO_PAGINA);
        error =agregarPaginaDinamica(1,3,basura);
        basura = string_repeat('4',configuracion.TAMANIO_PAGINA);
        error =agregarPaginaDinamica(4,1,basura);
        char* pal = buscarPaginaDinamico(2,2);
        
        basura = string_repeat('#',configuracion.TAMANIO_PAGINA);
        error =agregarPaginaDinamica(2,2,basura);
        printf("PAGINA: %s\n",pal);
        pal = buscarPaginaDinamico(1,2);
        printf("PAGINA: %s\n",pal);
        pal = buscarPaginaDinamico(1,3);
        printf("PAGINA: %s\n",pal);
        CrearCarpincho(3);
        mostrarCarpinchos();
        borrarCarpincho(2);
        borrarCarpincho(4);
        pal = buscarPaginaDinamico(1,2);
        printf("PAGINA: %s\n",pal);
        mostrarCarpinchos();
   

    }
}

// GENERAL
void crearArchivos(){
    
    int i = 0;
    while(configuracion.ARCHIVOS_SWAP_list[i]){
        // intenta abrirlo apra escritura, si no existe lo crea
        
        /*FILE* archivo = fopen (configuracion.ARCHIVOS_SWAP_list[i],"w+");

        /*for ( int j = 0; j< configuracion.TAMANIO_SWAP;j++){
            fputs("\0",archivo);
        }
        fclose(archivo);*/

        int particion = open(configuracion.ARCHIVOS_SWAP_list[i], O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
        struct stat infoParticion;
        stat(configuracion.ARCHIVOS_SWAP_list[i], &infoParticion);
        int sizeParticion = infoParticion.st_size;
        if(sizeParticion == 0){
        sizeParticion = configuracion.TAMANIO_SWAP;
        ftruncate(particion, sizeParticion);
        }
        //printf("El valor del archivo es %d\n",sizeParticion);
        char* particionPointer = mmap(NULL, sizeParticion, PROT_READ|PROT_WRITE, MAP_SHARED, particion, 0);
        //printf("FILE IN MEMORY: %s\n",particionPointer);
         
         //printf("cantidad de /0 es %d\n",cantidadBarra0File (configuracion.ARCHIVOS_SWAP_list[i]));
        /*int file =  open(configuracion.ARCHIVOS_SWAP_list[i], O_RDWR,S_IRUSR|S_IWUSR, O_APPEND,O_CREAT);
        for ( int j = 0; j< configuracion.TAMANIO_SWAP;j++){
            write(file,"\0",1);
        } 
        //int trun = truncate (configuracion.ARCHIVOS_SWAP_list[i],configuracion.TAMANIO_SWAP);
        
        // abre el archivo en modo escritura
         //int file = open (configuracion.ARCHIVOS_SWAP_list[i] , O_RDWR, S_IRUSR|S_IWUSR);
         
        // le trunca (coloca) un espacio fijo. puede que tenga datos y lo borre
        int trun2 = ftruncate(file,configuracion.TAMANIO_SWAP);
        printf("El valor del truncate es %d\n",trun2);
        printf("El truncated es %s\n",ftruncate(file,configuracion.TAMANIO_SWAP));
        char * file_in_memory = mmap(NULL,configuracion.TAMANIO_SWAP,PROT_READ |PROT_WRITE ,MAP_SHARED, file,0);
         printf("FILE IN MEMORY: %s\n",file_in_memory);
         printf("CANT FILE IN MEMORY: %d\n",string_length(file_in_memory));

        

        //printf("El valor del truncate 1 es %d\n",trun);
        FILE *fp =fopen(configuracion.ARCHIVOS_SWAP_list[i],"r");
        struct stat statbuf;
        int size = fstat(fp,&statbuf);
        int tamano = statbuf.st_size;
        fclose(fp);
        
        printf("El valor del archivo es %ld\n",statbuf.st_size);
         //printf("el valor de i es %d.\n",i);*/
         close(particion);
        i++;
       
    }
     //printf("FIN aapertura archivo y truncate\n");
        
}
int elegirMejorArchivo(){
    // busca el archivo con el que mas espacio tenga. 
    int i = 0;
    int menor = cantidadBarra0File (configuracion.ARCHIVOS_SWAP_list[i]);
    int indiceMenor = 0;
   
    //int menor2 = cantidadCaracteresFile (configuracion.ARCHIVOS_SWAP_list[1]);
    //printf("valor menor %d\n",menor);
    //printf("valor menor2 %d\n",menor2);
    while(configuracion.ARCHIVOS_SWAP_list[i]){
       //printf("el valor de i es %d.\n",i);
        if (i == 0){
           menor = cantidadBarra0File (configuracion.ARCHIVOS_SWAP_list[i]);
        }
        else if (menor < cantidadBarra0File (configuracion.ARCHIVOS_SWAP_list[i])){
            menor = cantidadBarra0File (configuracion.ARCHIVOS_SWAP_list[i]);
            indiceMenor = i;
        }
        // FUNCION STAT NO USADA
        /*int file = open (configuracion.ARCHIVOS_SWAP_list[i] , 1);
        struct stat statbuf;
        int size = fstat(file,&statbuf);
        close(file);
        int tamano = statbuf.st_size; 
        printf("El valor de truncate es %d\n",tamano);
        */
        
	    //char** list = string_split(bufer, "\n");
         /* if (ptr == NULL) {
                printf("es null \n");
            }*/
        //printf("El valor del archivo es %s\n",devuelve);
        i++;
    }
    //printf("el indice menor es %d.\n",indiceMenor);
    return indiceMenor;
}
Carpincho_Swamp* buscarCarpincho(int pidd){
    //printf("BUSCANDO NUMERO CARPINCHO. %d\n",pidd);
    int max = list_size(lista_carpinchos);
    Carpincho_Swamp* car = malloc(sizeof(Carpincho_Swamp));
    //printf("Cantidad de carpinchos en lista es %d\n",max);
    for (int i = 0;i<max;i++){
    
        car = (Carpincho_Swamp*) list_get(lista_carpinchos,i);
        /*printf("ITERACION %d\n",i);
        printf("EL pid es %d \n",car->pid);
        printf("EL num archivo es %d \n",car->numeroArchivo);*/
        if(car->pid == pidd){
            return car;
        }
    }
    car = malloc(sizeof(Carpincho_Swamp));
    car->pid = -1;
    return car;
}
int cantidadCaracteresFile (char* path){
    FILE* archivo = fopen (path , "r");
    
    int i = 0;
    while(configuracion.ARCHIVOS_SWAP_list[i]){
        // intenta abrirlo apra escritura, si no existe lo crea
        
        if (fopen (configuracion.ARCHIVOS_SWAP_list[i],"r") == -1){
            printf("EL archivo no existe. \n");
        }
        FILE* archivo = fopen (configuracion.ARCHIVOS_SWAP_list[i],"r+");

       
    char bufer[configuracion.TAMANIO_SWAP];
    char* ptr = fgets(bufer, configuracion.TAMANIO_SWAP, archivo);
    int cant = string_length(ptr);
    //printf("El valor del archivo es %s\n",bufer);
    //printf("La cantidad es %d\n",cant);
    fclose(archivo);
    return cant;
    }
}
int cantidadBarra0File (char* path){
    int  file =  open(path, O_RDWR, S_IRUSR|S_IWUSR);
    char * file_in_memory = mmap(NULL,configuracion.TAMANIO_SWAP,PROT_READ |PROT_WRITE ,MAP_SHARED, file,0);
    int k = 0;
    for(int i = 0; i< configuracion.TAMANIO_SWAP;i++ ){
        //printf("entro\n");
        //printf("%c",file_in_memory[i]);
        if (file_in_memory[i]== '\0'){
            //printf("entroHOLI\n");
            k++;
        }
    }
    //printf("El valor del archivo es %s\n",bufer);
    //printf("La cantidad es %d\n",cant);
    close(file);
    return k;
}
bool quedaEspacioEnArchivo(){
    int i = 0;
    while(configuracion.ARCHIVOS_SWAP_list[i]){
        if( cantidadCaracteresFile (configuracion.ARCHIVOS_SWAP_list[i])< configuracion.TAMANIO_SWAP-1){
            return true;
        }
        i++;
    }
    return false;
}
bool quedaEspacioEnArchivoDOS(){
    int i = 0;

    while(configuracion.ARCHIVOS_SWAP_list[i]){
        // PUEDE SALTAR ERROR O QUE SEA TAMAÑO PAGINA -1
        if( cantidadBarra0File (configuracion.ARCHIVOS_SWAP_list[i]) > configuracion.TAMANIO_PAGINA){
            return true;
        }
        i++;
    }
    return false;
}


//DINAMICA
int CrearCarpincho( int pidd){
    printf("Creando carpincho %d\n",pidd);
    Carpincho_Swamp* car = buscarCarpincho(pidd);
    if (elegirMejorArchivoDOS (pidd) != -1 && car->pid == -1 ){
        int mejorArchivo = elegirMejorArchivoDOS();
        Carpincho_Swamp* carpincho;
        carpincho = malloc(sizeof(Carpincho_Swamp));
        carpincho->pid = pidd;
        carpincho->numeroArchivo = mejorArchivo;
        carpincho->paginas= list_create();
        list_add(lista_carpinchos,carpincho);
        return 1;
    }
    return -1;
}
// sirve para modificar una pagina tambien. 
int agregarPaginaDinamica(int pid, int pagina, char* contenido){
    //TAMBIEN MODIFICA
    printf("Agregando pagina: pid: %d pag: %d\n",pid,pagina);
    //printf("ENTRO contenido: %s\n",contenido);
    Carpincho_Swamp* car = buscarCarpincho(pid);
    if (car->pid != -1){
        int base = buscarMarcoLibre(car->numeroArchivo);
        //printf("base:%d\n",base);
        Marcos_x_pagina* mar_pag = malloc(sizeof(Marcos_x_pagina));
        mar_pag->base = base;
        mar_pag->pagina = pagina;
        list_add(car->paginas,mar_pag);
        //printf("Agrego a la lista\n");
        int  file =  open(configuracion.ARCHIVOS_SWAP_list[car->numeroArchivo], O_RDWR, S_IRUSR|S_IWUSR);
        int j = 0;
        /*lseek (file , base, 0);
        write(file,contenido,configuracion.TAMANIO_PAGINA);*/
        char * file_in_memory = mmap(NULL,configuracion.TAMANIO_SWAP,PROT_READ |PROT_WRITE ,MAP_SHARED, file,0);
        for (int i = base; i < base+configuracion.TAMANIO_PAGINA ;i++){
            
            file_in_memory[i]=contenido[j];
            //printf("%d/%c - ",i,file_in_memory[i]);
            j++;
        }
        //printf("\n");
        //munmap(file_in_memory,configuracion.TAMANIO_SWAP);
        // printf("FILE IN MEMORY: %s\n",file_in_memory);
        //printf("CANT FILE IN MEMORY: %d\n",string_length(file_in_memory));
        close(file);
    }
    else{
        printf("CARPINCHO NO ENCONTRADO\n");
    }
}

char* buscarPaginaDinamico(int pid, int pagina){
    Carpincho_Swamp* car = buscarCarpincho(pid);
    int max= list_size(car->paginas);
    Marcos_x_pagina* mar_x_pag;
    //printf("EL max es %d\n",max);
    for(int i = 0; i<max;i++){
        mar_x_pag = malloc (sizeof(Marcos_x_pagina));
        mar_x_pag = list_get(car->paginas,i);
        //printf("La pagina es: %d\n",mar_x_pag->pagina);
        //printf("La base es: %d \n",mar_x_pag->base);
        if( mar_x_pag->pagina == pagina){
            //printf("ES ESTA\n");
            i = max;
        }
    }
    //printf("La cantidad del archivo es: %d\n",cantidadCaracteresFile(configuracion.ARCHIVOS_SWAP_list[car->numeroArchivo]));
    int  file =  open(configuracion.ARCHIVOS_SWAP_list[car->numeroArchivo], O_RDWR, S_IRUSR|S_IWUSR);
    struct stat* estadisticas;
    int tamano = stat (file,estadisticas);
    //printf("El valor el tamaño es %d\n",estadisticas.st_size);
    char * file_in_memory = mmap(NULL,configuracion.TAMANIO_SWAP,PROT_READ |PROT_WRITE ,MAP_SHARED, file,0);
    //printf("FILE IN MEMORY: %s\n",file_in_memory);
    //printf("CANT FILE IN MEMORY: %d\n",string_length(file_in_memory));
    char* pagina_devolver = string_substring(file_in_memory, mar_x_pag->base,configuracion.TAMANIO_PAGINA);
    //char * file_in_memory2 = mmap(NULL,configuracion.TAMANIO_SWAP,PROT_READ |PROT_WRITE ,MAP_SHARED, file,0);
    close(file);
    //printf("se encontro: %s\n",pagina_devolver);
    return pagina_devolver;

}
void marcosLibes(){
    int i = 0;
    while(configuracion.ARCHIVOS_SWAP_list[i]){    
        for(int j = 0; j<configuracion.TAMANIO_SWAP; j = j+ configuracion.TAMANIO_PAGINA)
        {
            //printf("j-%d",j);
            un_marco_libre* mar = malloc (sizeof(un_marco_libre));
            //printf("%d - ",j);
            mar->numero_archivo = i;
            mar->base = j;
            list_add(marcos_libres,mar);
        }
        //printf("l\n");
        i++;
    }
}
int buscarMarcoLibre(int num_archivo){
    printf("BUSAR MARCO LIBRE ----\n");
    int max = list_size(marcos_libres);
    printf("EL max es %d\n",max);
    for(int i = 0; i<max;i++){
        un_marco_libre* mar = malloc (sizeof(un_marco_libre));
        mar = list_get(marcos_libres,i);
        printf("marco_pagina %d - numero_archivo:%d - base:%d \n",i,mar->numero_archivo,mar->base);
        if(mar->numero_archivo == num_archivo){
            list_remove(marcos_libres,i);
            max = max-1;
            return mar->base;
        }
    }
    return -1;
}
int borrarCarpincho(int pid){
    Carpincho_Swamp* car = buscarCarpincho(pid);
    int max= list_size(car->paginas);
    Marcos_x_pagina* mar_x_pag;
    if(  car->pid != -1 ){
        int  file =  open(configuracion.ARCHIVOS_SWAP_list[car->numeroArchivo], O_RDWR, S_IRUSR|S_IWUSR);
        char * file_in_memory = mmap(NULL,configuracion.TAMANIO_SWAP,PROT_READ |PROT_WRITE ,MAP_SHARED, file,0);
        for(int i = 0; i<max;i++){
            mar_x_pag = malloc (sizeof(Marcos_x_pagina));
            mar_x_pag = list_get(car->paginas,i);
            for(int j =mar_x_pag->base; j< mar_x_pag->base+configuracion.TAMANIO_PAGINA;j++){
                file_in_memory[j]='\0';
            }
            list_remove(car->paginas,i);
            un_marco_libre* mar = malloc (sizeof(un_marco_libre));
            mar->numero_archivo = car->numeroArchivo;
            mar->base = mar_x_pag->base;
            list_add(marcos_libres,mar);
        }
        close(file);

        int max2 = list_size(lista_carpinchos);
        Carpincho_Swamp* car2 = malloc(sizeof(Carpincho_Swamp));
        //printf("Cantidad de carpinchos en lista es %d\n",max);
        for (int i = 0;i<max2;i++){
        
            car2 = (Carpincho_Swamp*) list_get(lista_carpinchos,i);

            if(car2->pid == car->pid){
            list_remove(lista_carpinchos,i);
            i=max2;
            }
        }
        return 1;
    }
    return -1;
}
bool quedaPaginasEnArchivo(int pid){
    Carpincho_Swamp* car = buscarCarpincho(pid);

    int max = list_size(marcos_libres);
    for(int i = 0; i<max;i++){
        un_marco_libre* mar = malloc (sizeof(un_marco_libre));
        mar = list_get(marcos_libres,i);
        if(mar->numero_archivo == car->numeroArchivo){
            return true;
        }
    }
    return false;
}
int solicitudPagina(int pid, int pagina){
     Carpincho_Swamp* car = buscarCarpincho(pid);
    if (car->pid != -1){
        
        int base = buscarMarcoLibre(car->numeroArchivo);
        if(base != -1){
            Marcos_x_pagina* mar_pag = malloc(sizeof(Marcos_x_pagina));
            mar_pag->base = base;
            mar_pag->pagina = pagina;
            list_add(car->paginas,mar_pag);
            return 1;
        }
        //printf("base:%d\n",base);
       
    }
    return -1;
}
int borrarPagina(int pid, int pagina){
    // ver tema si encontro la pagina...
    Carpincho_Swamp* car = buscarCarpincho(pid);
    int max= list_size(car->paginas);
    Marcos_x_pagina* mar_x_pag;
    if (car->pid != -1){
        int  file =  open(configuracion.ARCHIVOS_SWAP_list[car->numeroArchivo], O_RDWR, S_IRUSR|S_IWUSR);
        char * file_in_memory = mmap(NULL,configuracion.TAMANIO_SWAP,PROT_READ |PROT_WRITE ,MAP_SHARED, file,0);
        int encontro = 0;
        for(int i = 0; i<max;i++){
            
            mar_x_pag = malloc (sizeof(Marcos_x_pagina));
            mar_x_pag = list_get(car->paginas,i);
            if( mar_x_pag->pagina == pagina){
                for(int j =mar_x_pag->base; j< mar_x_pag->base+configuracion.TAMANIO_PAGINA;j++){
                    file_in_memory[j]='\0';
                }
                list_remove(car->paginas,i);
                un_marco_libre* mar = malloc (sizeof(un_marco_libre));
                mar->numero_archivo = car->numeroArchivo;
                mar->base = mar_x_pag->base;
                list_add(marcos_libres,mar);
                encontro = 1;
                i = max;
            }
        
        }
        close(file);
        if (encontro != 0){
            return 1;
        }
        
    }
    return -1;
}

// ASIGNACION FIJA
int crearCarpinchoFijaDOS(int pidd){
    printf("Creando carpincho %d\n",pidd);
    Carpincho_Swamp* car = buscarCarpincho(pidd);
    Marcos_x_pagina* mar_x_pag;
    if (elegirMejorArchivoDOS (pidd) != -1 && car->pid == -1 ){
        //printf("ENTRO %d\n",pidd);
        int mejorArchivo = elegirMejorArchivoDOS();
        int file =  open(configuracion.ARCHIVOS_SWAP_list[mejorArchivo], O_RDWR,S_IRUSR|S_IWUSR, O_APPEND);
        Carpincho_Swamp* carpincho;
        carpincho = malloc(sizeof(Carpincho_Swamp));
        carpincho->pid = pidd;
        carpincho->numeroArchivo = mejorArchivo;
        carpincho->base = elegirBaseCarpincho(mejorArchivo);
        carpincho->paginas = list_create();
        for( int i = 0; i< configuracion.MARCOS_MAXIMOS; i++ ){
            Marcos_x_pagina* mar_pag = malloc(sizeof(Marcos_x_pagina));
            mar_pag->base = carpincho->base+ configuracion.TAMANIO_PAGINA*i;
            mar_pag->pagina = -1;
            mar_pag->marco = i;
            list_add(carpincho->paginas,mar_pag);
        }
        
        list_add(lista_carpinchos,carpincho);
        close(file);
        return 1;
    }
    else{
        return -1;
        printf("CARPINCHO YA EXISTENTE o no hay paginas\n");
    }
}
int elegirMejorArchivoDOS(){
    int i = 0;
    int mayorBloque = 0;
    int contador;
    int archivo_mayor = -1;
    while(configuracion.ARCHIVOS_SWAP_list[i]){
        contador = 0;
        un_marco_libre* mar = malloc (sizeof(un_marco_libre));
        int max = list_size(marcos_libres);
        if (max != 0){
            for (int j = 0;j<max;j++){
                mar = ( un_marco_libre*) list_get(marcos_libres,j);
                
                if(mar->numero_archivo == i){
                    //printf("%d-",mar->numero_archivo);
                    contador++;
                }
            }
            //printf("\n");
            if (mayorBloque == 0){
                mayorBloque = contador;
                archivo_mayor = i;
            }
            else if(contador>mayorBloque){
                archivo_mayor = i;
            }
        }
        
        i++;
    }
    return archivo_mayor;
}
void marcosLIbresFija(){
    int i = 0;
     while(configuracion.ARCHIVOS_SWAP_list[i]){    
        for(int j = 0; j<configuracion.TAMANIO_SWAP-configuracion.TAMANIO_PAGINA*configuracion.MARCOS_MAXIMOS; j = j+ configuracion.TAMANIO_PAGINA*configuracion.MARCOS_MAXIMOS)
            {
                un_marco_libre* mar = malloc (sizeof(un_marco_libre));
                //printf("%d - ",j);
                mar->numero_archivo = i;
                mar->base = j;
                list_add(marcos_libres,mar);
                

            }
             //printf("\n");
         i++;
     }
     //printf("Marcos libres %d\n",list_size(marcos_libres_fija));

}
int elegirBaseCarpincho(int num_archivo){
    //printf("ENtro elegir base\n");
    un_marco_libre* mar = malloc (sizeof(un_marco_libre));
    int max = list_size(marcos_libres);
    //printf("max:%d\n",max);
    for(int j = 0; j< max; j++)
    {
        mar = ( un_marco_libre*) list_get(marcos_libres,j);
        //printf("num archivo: %d, num base: %d\n",mar->numero_archivo,mar->base);
        if (mar->numero_archivo == num_archivo){
            //printf("base%d\n",mar->base);
            int devolver = mar->base;
            list_remove(marcos_libres, j);
            return devolver;
        }
        
    }
}
void mostrarCarpinchos(){
    printf("--------\nMostrando carpincho\n");
    int max = list_size(lista_carpinchos);
    Carpincho_Swamp* car;
    printf("CANT CARPINCHO %d\n",max);
    for (int i = 0;i<max;i++){
    
        car = (Carpincho_Swamp*) list_get(lista_carpinchos,i);
        printf("ITERACION %d\n",i);
        printf("EL pid es %d \n",car->pid);
        printf("EL num archivo es %d \n",car->numeroArchivo);
        printf("La BASE es %d \n",car->base);
        printf("La cantidad de paginas %d\n",list_size(car->paginas));

    }
}
int agregarPaginaFija(int pid, int pagina, char* contenido){
    Carpincho_Swamp* car = buscarCarpincho(pid);
    //printf("llego aca\n");
    if(  car->pid != -1 ){
        // printf("llego aca2\n");
        Marcos_x_pagina* mar_x_pag;
        mar_x_pag = buscarMarcoXPagina(car, pagina);
        //printf("marco x pagina: %d\n",mar_x_pag->pagina);
        if (mar_x_pag->pagina == -1 ){
           // printf("entro\n");
            int max= list_size(car->paginas);
            for(int i = 0; i<max;i++){
                //printf("entro i: %d\n",i);
                mar_x_pag = malloc (sizeof(Marcos_x_pagina));
                mar_x_pag = list_get(car->paginas,i);

                if( mar_x_pag->pagina == -1){
                    mar_x_pag->pagina = pagina;
                   
                    i = max;
                }
            
            }
        }
        //printf("llego aca4\n");
        int  file =  open(configuracion.ARCHIVOS_SWAP_list[car->numeroArchivo], O_RDWR, S_IRUSR|S_IWUSR);
        //printf("llego aca5\n");
        char * file_in_memory = mmap(NULL,configuracion.TAMANIO_SWAP,PROT_READ |PROT_WRITE ,MAP_SHARED, file,0);        
        int j = 0;
        //printf("llego aca6\n");
        for (int i = mar_x_pag->base; i < mar_x_pag->base + configuracion.TAMANIO_PAGINA ;i++){
            //printf("r-%d\n",i);
            file_in_memory[i]=contenido[j];
            j++;
        }
        //printf("llego aca7\n");
        //printf("\n");
        //munmap(file_in_memory,cantidadCaracteresFile(configuracion.ARCHIVOS_SWAP_list[car->numeroArchivo]));
        
        close(file);

       
    }
    //p
        /*mar_pag->base = ;
        mar_pag->pagina = pagina;
        list_add(car->paginas,mar_pag);*/
    
    else{
        return -1;
    }
}
Marcos_x_pagina* buscarMarcoXPagina( Carpincho_Swamp* car, int pagina){
    int max= list_size(car->paginas);
    Marcos_x_pagina* mar_x_pag;
    for(int i = 0; i<max;i++){
        mar_x_pag = malloc (sizeof(Marcos_x_pagina));
        mar_x_pag = list_get(car->paginas,i);

        if( mar_x_pag->pagina == pagina){
              return mar_x_pag;  
           }
            
    }
    mar_x_pag = malloc (sizeof(Marcos_x_pagina));
    mar_x_pag->pagina = -1;
    return mar_x_pag;

}

int remplazoPaginaFija(int pid, int pagina, char*contenido){

    Carpincho_Swamp* car = buscarCarpincho(pid);
    if(  car->pid != -1 ){
        int  file =  open(configuracion.ARCHIVOS_SWAP_list[car->numeroArchivo], O_RDWR, S_IRUSR|S_IWUSR);
        //printf(" base %d carp: %d.\n",base,pid);
        char * file_in_memory = mmap(NULL,configuracion.TAMANIO_SWAP,PROT_READ |PROT_WRITE ,MAP_SHARED, file,0);
       // supone que las paginas cuentan desde 0. por eso desde la pagina.. hasta pagina +1..
        int j = 0;
       
        for (int i = car->base+(pagina)*configuracion.TAMANIO_PAGINA; i < car->base+(pagina+1)*configuracion.TAMANIO_PAGINA ;i++){
            //printf("r-%d\n",i);
            file_in_memory[i]=contenido[j];
            j++;
        }
        //printf("\n");
        //munmap(file_in_memory,cantidadCaracteresFile(configuracion.ARCHIVOS_SWAP_list[car->numeroArchivo]));
        
        close(file);
       
        return 1;
    }
    else{
        printf("NO SE ENCONTRO LA PAGINA\n");
        return -1;
    }
    
}

char* buscarPaginaFija(int pid, int pagina){
    Carpincho_Swamp* car = buscarCarpincho(pid);
    Marcos_x_pagina* mar_x_pag = buscarMarcoXPagina( car,pagina);
    if(  car->pid != -1 && mar_x_pag->pagina != -1 ){
        
        int  file =  open(configuracion.ARCHIVOS_SWAP_list[car->numeroArchivo], O_RDWR, S_IRUSR|S_IWUSR);
        
         //printf(" base %d carp: %d.\n",base,pid);
        char * file_in_memory = mmap(NULL,configuracion.TAMANIO_SWAP,PROT_READ |PROT_WRITE ,MAP_SHARED, file,0);
       // supone que las paginas cuentan desde 0. por eso desde la pagina.. hasta pagina +1..
       
        char* pagina_devolver = string_substring(file_in_memory, mar_x_pag->base,configuracion.TAMANIO_PAGINA);
        
       /* for (int i = base + pagina*configuracion.TAMANIO_PAGINA; i < tope ;i++){
            //printf("%d\n",j);
          string_append 
          
        }*/
        //printf("\n");
        //munmap(file_in_memory,cantidadCaracteresFile(configuracion.ARCHIVOS_SWAP_list[car->numeroArchivo]));
        
        close(file);
        //printf("Devuelve: %s.\n",pagina_devolver);
        //printf("CANT CARACTERES: %d.\n",string_length(pagina_devolver));
        return pagina_devolver;
    }
    printf("pagina no encontrada\n");
    return "";
    
}
int BorrarPaginaFija(int pid, int pagina){
    Carpincho_Swamp* car = buscarCarpincho(pid);
      Marcos_x_pagina* mar_x_pag = buscarMarcoXPagina( car,pagina);
    if(  car->pid != -1 && mar_x_pag->pagina != -1 ){
        int  file =  open(configuracion.ARCHIVOS_SWAP_list[car->numeroArchivo], O_RDWR, S_IRUSR|S_IWUSR);
        
         //printf(" base %d carp: %d.\n",base,pid);
        char * file_in_memory = mmap(NULL,configuracion.TAMANIO_SWAP,PROT_READ |PROT_WRITE ,MAP_SHARED, file,0);
       // supone que las paginas cuentan desde 0. por eso desde la pagina.. hasta pagina +1..
       for( int i = mar_x_pag->base; i < mar_x_pag->base+configuracion.TAMANIO_PAGINA;i++){
            file_in_memory[i] ='\0'; 
       }
        //char* pagina_devolver = string_substring(file_in_memory, car->base + pagina*configuracion.TAMANIO_PAGINA,configuracion.TAMANIO_PAGINA);
        
       /* for (int i = base + pagina*configuracion.TAMANIO_PAGINA; i < tope ;i++){
            //printf("%d\n",j);
          string_append 
          
        }*/
        //printf("\n");
        //munmap(file_in_memory,cantidadCaracteresFile(configuracion.ARCHIVOS_SWAP_list[car->numeroArchivo]));
        
        close(file);
        //printf("Devuelve: %s.\n",pagina_devolver);
        //printf("CANT CARACTERES: %d.\n",string_length(pagina_devolver));
        //return pagina_devolver;
    }
    else{
        printf("Pagina no encontrada o pagina no encontrada\n");
    }
    
}
int borrarCarpinchoFija(int pid){
    Carpincho_Swamp* car = buscarCarpincho(pid);
    if(  car->pid != -1 ){
        int  file =  open(configuracion.ARCHIVOS_SWAP_list[car->numeroArchivo], O_RDWR, S_IRUSR|S_IWUSR);
      
        char * file_in_memory = mmap(NULL,configuracion.TAMANIO_SWAP,PROT_READ |PROT_WRITE ,MAP_SHARED, file,0);
        for (int i = car->base; i < car->base+configuracion.MARCOS_MAXIMOS*configuracion.TAMANIO_PAGINA;i++){
            //printf("r-%d\n",i);
            file_in_memory[i]='\0';
           
        }
        un_marco_libre* mar = malloc (sizeof(un_marco_libre));
        mar->numero_archivo = car->numeroArchivo;
        mar->base = car->base;
        list_add(marcos_libres,mar);
        printf("entro1\n");
        sacarCarpinchoDeLista(car->pid);
        free(car);

        close(file);
        return 1;
    }
    return -1;
}
void sacarCarpinchoDeLista( int pidd){
    int max = list_size(lista_carpinchos);
    Carpincho_Swamp* car = malloc(sizeof(Carpincho_Swamp));
    //printf("Cantidad de carpinchos en lista es %d\n",max);
  
    for (int i = 0;i<max;i++){
        
        car = (Carpincho_Swamp*) list_get(lista_carpinchos,i);
        if(car->pid == pidd){
            list_remove(lista_carpinchos, i);
            max = max-1;
            i = max;
        }
    }
  
}
bool puedeAgregarPagina( int pid){
    Carpincho_Swamp* car = buscarCarpincho(pid);
    int max= list_size(car->paginas);
    Marcos_x_pagina* mar_x_pag;
    for(int i = 0; i<max;i++){
        mar_x_pag = malloc (sizeof(Marcos_x_pagina));
        mar_x_pag = list_get(car->paginas,i);

        if( mar_x_pag->pagina == -1){
              return true;  
           }
            
    }
    return false;
}
int solicitarPaginaFija(int pid, int pagina){
      Carpincho_Swamp* car = buscarCarpincho(pid);
    //printf("llego aca\n");
    if(  car->pid != -1 ){
        // printf("llego aca2\n");
        Marcos_x_pagina* mar_x_pag;
        mar_x_pag = buscarMarcoXPagina(car, pagina);
        //printf("marco x pagina: %d\n",mar_x_pag->pagina);
        if (mar_x_pag->pagina == -1 ){
           // printf("entro\n");
            int max= list_size(car->paginas);
            for(int i = 0; i<max;i++){
                //printf("entro i: %d\n",i);
                mar_x_pag = malloc (sizeof(Marcos_x_pagina));
                mar_x_pag = list_get(car->paginas,i);

                if( mar_x_pag->pagina == -1){
                    mar_x_pag->pagina = pagina;
                   
                    i = max;
                }
            
            }
        }
        return 1;
    }
    return -1;
}

// CONEXIONES
void terminar_programa(){
   config_destroy(config);
}
void iniciar_swamp(){
   server = iniciar_servidor("127.0.0.1", "5003");
}
void atender_clientes(){ 

   // while feo
   while(1){ 
      int cliente = aceptar_cliente(server);
      //printf("Se conecto alguien en el socket: %d\n", cliente);
       int operacion = recibir_operacion(cliente);
      int estado;
      switch (operacion)
      {
      case SOLICITUD_INICIO:
      // estado = funcion que determina si puede iniciar un nuevo proceso (0 si puede, -1 si no puede)
         estado = 0;
         resolver_estado(estado, cliente);
         break;


      //faltan las demas operaciones
      default:
         break;
      }
   }
}

int resolver_estado(int estado, int cliente){

   if (estado == INVALIDO)
   {
      enviar_int(cliente, INVALIDO);
      return INVALIDO;
   }

   if (estado == VALIDO)
   {
      enviar_int(cliente, VALIDO);
      return recibir_int(cliente);
   }

}
void agregarPedidosMemoria (int cliente){
    int i = 0;
    while(1){
        memoria_operacion(cliente);
        
        /*Pedido* ped = malloc(sizeof(Pedido));
        ped->nombre_pedido = "crear";
        ped->pid = i;
        sem_wait(mutex_lista_pedidos);
        list_add(lista_pedidos,ped);
        sem_post(mutex_lista_pedidos);
        sem_post(agrego_lista_pedidos);
        sleep(1);
        i++;
        /* char* nombre_pedido;
            int pid;
            int pagina;
            char* contenido_pagina;
        */
    }
    
}


void recibir_asignacion(int cliente){
    int op = recibir_operacion(cliente);
    if(op != INICIO_CONFIG)return ;
    int size = recibir_int(cliente);

}

// funcion para testear con memoria rapido
void memoria_tests(int argc, char* argv)
{
    if(argc == 1 || strcmp(argv, "memoria") != 0) return;
    iniciar_swamp();
    atender_clientes();
    abort();
}


void memoria_operacion(int cliente){

    int codop = recibir_operacion(cliente);
    printf("codigo de operacdion %d\n", codop);
    //int tamanio = recibir_int(cliente);
    Pedido* ped = malloc(sizeof(Pedido));
    int tamanio_pid,pid,tamanio_pagina,pagina,tamanio2;
   switch(codop){
        case ESCRIBIR_PAGINA:
            tamanio_pid = recibir_int(cliente);
            pid = recibir_int(cliente);
            tamanio_pagina = recibir_int(cliente);
            pagina = recibir_int(cliente);
            tamanio2;
            void* buffer = recibir_buffer_t(tamanio2, cliente);
            ped->nombre_pedido = malloc(sizeof("ESCRIBIR_PAGINA"));
            ped->nombre_pedido = "ESCRIBIR_PAGINA";
            ped->pid = pid;
            ped->pagina = pagina;
            ped->contenido_pagina = buffer;
            ped->oper = codop;
            sem_wait(mutex_lista_pedidos);
            list_add(lista_pedidos,ped);
            sem_post(mutex_lista_pedidos);
            sem_post(agrego_lista_pedidos);
            break;

        case SOLICITUD_INICIO:
         printf("entro a solicitud inicio\n");
            //tamanio_pid = recibir_int(cliente);
            pid = recibir_int(cliente);
            // queda lugar
            ped->nombre_pedido = "SOLICITUD_INICIO";
            ped->pid = pid;
            ped->oper = codop;
            printf("creo struct de pid %d\n", pid);
            sem_wait(mutex_lista_pedidos);
            list_add(lista_pedidos,ped);
            sem_post(mutex_lista_pedidos);
            sem_post(agrego_lista_pedidos);
            printf("agrego a lista\n");
            break;

        case INICIO:
            tamanio_pid = recibir_int(cliente);
            pid = recibir_int(cliente);
            ped->nombre_pedido = "INICIO";
            ped->pid = pid;
            ped->oper = codop;
            sem_wait(mutex_lista_pedidos);
            list_add(lista_pedidos,ped);
            sem_post(mutex_lista_pedidos);
            sem_post(agrego_lista_pedidos);
            break;

        case SOLICITUD_PAGINA:
            tamanio_pid = recibir_int(cliente);
            pid = recibir_int(cliente);
            tamanio_pagina = recibir_int(cliente);
            pagina = recibir_int(cliente);
            ped->nombre_pedido = "SOLICITUD_PAGINA";
            ped->pid = pid;
            ped->pagina = pagina;
            ped->oper = codop;
            sem_wait(mutex_lista_pedidos);
            list_add(lista_pedidos,ped);
            sem_post(mutex_lista_pedidos);
            sem_post(agrego_lista_pedidos);
            break;

        case BORRAR_PAGINA:
            tamanio_pid = recibir_int(cliente);
            pid = recibir_int(cliente);
            tamanio_pagina = recibir_int(cliente);
            pagina = recibir_int(cliente);
            ped->nombre_pedido = "BORRAR_PAGINA";
            ped->pid = pid;
            ped->pagina = pagina;
            ped->oper = codop;
            sem_wait(mutex_lista_pedidos);
            list_add(lista_pedidos,ped);
            sem_post(mutex_lista_pedidos);
            sem_post(agrego_lista_pedidos);
            break;
         case OBTENER_PAGINA:
            tamanio_pid = recibir_int(cliente);
            pid = recibir_int(cliente);
            tamanio_pagina = recibir_int(cliente);
            pagina = recibir_int(cliente);
            ped->nombre_pedido = "OBTENER_PAGINA";
            ped->pid = pid;
            ped->pagina = pagina;
            ped->oper = codop;
            sem_wait(mutex_lista_pedidos);
            list_add(lista_pedidos,ped);
            sem_post(mutex_lista_pedidos);
            sem_post(agrego_lista_pedidos);
            break;

        case BORRAR_CARPINCHO:
            tamanio_pid = recibir_int(cliente);
            pid = recibir_int(cliente);
            ped->nombre_pedido = "BORRAR_CARPINCHO";
            ped->pid = pid;
            ped->oper = codop;
            sem_wait(mutex_lista_pedidos);
            list_add(lista_pedidos,ped);
            sem_post(mutex_lista_pedidos);
            sem_post(agrego_lista_pedidos);
            break;
        default:
            printf("NO ES DE SWAP %d.\n",codop);
            break;
    }

}