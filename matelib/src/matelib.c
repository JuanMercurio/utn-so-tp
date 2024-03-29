#include "matelib.h"

//------------------General Functions---------------------/

t_log* logger;

/* 
  - recibe un mensaje (handshake)
  - envia NEW_INSTANCE
  - recibe un pid
  - recibe confirmacion 
        - "OK"    --> true
        - != "OK" --> false

 */
int mate_init(mate_instance *lib_ref, char *config) //AGREGAR LOG
{
  printf("MATE INIT LLEGUE\n");
  t_config *configuracion = config_create(config);

  char *IP = config_get_string_value(configuracion, "IP");
  char *PUERTO = config_get_string_value(configuracion, "PUERTO");
  char *ARCHIVO_LOG = config_get_string_value(configuracion, "ARCHIVO_LOG");

  logger = log_create(ARCHIVO_LOG, "Carpincho", 0, LOG_LEVEL_INFO);
  log_info(logger, "Log creado");

  mate_inner_structure *mate_ref = malloc(sizeof(mate_inner_structure));
  lib_ref->group_info = mate_ref;

  mate_ref->conexion = crear_conexion(IP, PUERTO);

  char *respuesta = recibir_mensaje(mate_ref->conexion);
  conexion_set_server(mate_ref, respuesta, logger);

  log_info(logger, "\nNUEVO CARPINCHO\n");
  enviar_int(mate_ref->conexion, NEW_INSTANCE);
  mate_ref->pid  = recibir_int(mate_ref->conexion);

  char* estado = recibir_mensaje(mate_ref->conexion); // el procesos queda bloqueado aca hasta recibir un OK del procesador

  if (strcmp(estado, "OK") == 0)
  {
    log_info(logger, "Carpincho creado correctamente con el pid: %d", mate_ref->pid);
    return 0;
  }
  else
  {
    log_info(logger, "Error al crear el carpimcho");
    return -1;
  }
}

int mate_close(mate_instance *lib_ref)
{
  log_info(logger, "MATE_CLOSE carpincho %d",((mate_inner_structure*)lib_ref->group_info)->pid );
  enviar_int(((mate_inner_structure*)lib_ref->group_info)->conexion, MATE_CLOSE);
  log_info(logger, "MATE_CLOSE: mensaje enviado a kernel");
  free(lib_ref->group_info);
  log_info(logger, "Carpincho eliminado");
    return 0;
 }

//-----------------Semaphore Functions---------------------/

int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value)
{

  log_info(logger, "SEM_INIT");

  if (conectado_a_memoria(lib_ref))
  {
    log_info(logger, "Conectado con memoria. No se puede inicializar semáforo.");
    return -1;
  }

  //((mate_inner_structure *)lib_ref->group_info)->sem_instance = malloc(sizeof(sem_t));
  // sem_init(((mate_inner_structure *)lib_ref->group_info)->sem_instance, 0, value);
log_info(logger, "Conectado con memoria. No se puede inicializar semáforo.");
  enviar_sem_init(sem, (int)value, ((mate_inner_structure *)lib_ref->group_info)->conexion, INIT_SEMAFORO); // codear fiuncion; pasar valor
  log_info(logger, "envio sem init al kernel.");
  int respuesta = recibir_int(((mate_inner_structure *)lib_ref->group_info)->conexion);               // espera respuesta para continuar ejecutando condigo???
log_info(logger, "recibio algo del kernel por sem init %d .", respuesta);
return respuesta;
}

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem)
{
  log_info(logger, "SEM_WAIT al semáforo %s de carpincho %d", sem,((mate_inner_structure *)lib_ref->group_info)->pid );

  if (conectado_a_memoria(lib_ref))
  {
    log_info(logger, "Conectado con memoria. No se puede hacer SEM WAIT.");
    return -1;
  }

  enviar_mensaje_y_cod_op(sem, ((mate_inner_structure *)lib_ref->group_info)->conexion, SEM_WAIT);
  int respuesta = recibir_int(((mate_inner_structure *)lib_ref->group_info)->conexion);

  if (respuesta != -1)
  {
    if(respuesta ==0)
    recibir_mensaje(((mate_inner_structure *)lib_ref->group_info)->conexion);
    log_info(logger, "Se realizó el WAIT correctamente");
    return 0;
  }
  else
  {
    log_info(logger, "Error al realizar el WAIT");
    return -1;
  }
}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem)
{
  log_info(logger, "SEM_POST al semáforo %s del carpincho %d", sem, ((mate_inner_structure *)lib_ref->group_info)->pid);

  if (conectado_a_memoria(lib_ref))
  {
    log_info(logger, "Conectado con memoria. No se puede hacer SEM POST.");
    return -1;
  }

  enviar_mensaje_y_cod_op(sem, ((mate_inner_structure *)lib_ref->group_info)->conexion, SEM_POST);
  int respuesta = recibir_int(((mate_inner_structure *)lib_ref->group_info)->conexion);
  if (respuesta == 0)
  {
    log_info(logger, "Se realizó el POST correctamente a %s", sem);
  }
  else
  {
    log_info(logger, "Error al realizar el POST a %s", sem);
  }
  return respuesta;
}

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem)
{
  log_info(logger, "SEM_DESTROY al semáforo %s", sem);

  if (conectado_a_memoria(lib_ref))
  {
    log_info(logger, "Conectado con memoria. No se puede hacer SEM DESTROY.");
    return -1;
  }

  enviar_mensaje_y_cod_op(sem, ((mate_inner_structure *)lib_ref->group_info)->conexion, SEM_DESTROY);
  int respuesta = recibir_operacion(((mate_inner_structure *)lib_ref->group_info)->conexion);
 log_info(logger, "respuesta SEM DDESTROY %d\n", respuesta);

  if (respuesta == 0)
  {
    log_info(logger, "Se realizó el DESTROY correctamente");
    return 0;
  }
  else
  {
    log_info(logger, "Error al realizar el DESTROY");
    return -1;
  }
}
//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg)
{
  log_info(logger, "CALL_IO a %s carpincho %d", io , ((mate_inner_structure*)lib_ref->group_info)->pid);
  log_info(logger,"---:-%s", msg);
  
  if (conectado_a_memoria(lib_ref))
  {
    log_info(logger, "Conectado con memoria. No se puede hacer CALL_IO.");
    return -1;
  }

  enviar_mensaje_y_cod_op(io, ((mate_inner_structure *)lib_ref->group_info)->conexion, IO);
 int respuesta = recibir_operacion(((mate_inner_structure *)lib_ref->group_info)->conexion);
log_info(logger, "RESPUESA DE IO RECIBIDA %d ", respuesta);
  if (respuesta == 0)
  {log_info(logger, "MATE IO ESPERANDO OK DEL PROCESADOR PARA SEGUIR carpincho %d\n\n", ((mate_inner_structure *)lib_ref->group_info)->pid);
    char *espuesta = recibir_mensaje(((mate_inner_structure *)lib_ref->group_info)->conexion);
    log_info(logger,"\n_____llego del procesador un %s para el caprincho %d\n\n", espuesta, ((mate_inner_structure *)lib_ref->group_info)->pid);
    log_info(logger, "Se realizó el CALL IO correctamente");
  }
  else
  {
    log_info(logger, "Error al realizar el CALL IO");
  }
  return respuesta;
}

//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size)
{
  log_info(logger, "MEM_ALLOC de carpincho %d de tamaño %d",((mate_inner_structure *)lib_ref->group_info)->pid, size);

  //((mate_inner_structure *)lib_ref->group_info)->memory = malloc(size);

  enviar_cod_op_e_int(((mate_inner_structure *)lib_ref->group_info)->conexion, MEMALLOC, size);

  int32_t respuesta = (int32_t)recibir_operacion(((mate_inner_structure *)lib_ref->group_info)->conexion);

  if ((int)respuesta == -1)
  {
    log_info(logger, "Error al realizar el MEM_ALLOC");
    return -1;
  }
  else
  {
    log_info(logger, "Se realizó el MEM_ALLOC correctamente. La dirección lógica es %d. Carpincho %d", respuesta,((mate_inner_structure *)lib_ref->group_info)->pid);
    return respuesta;
  }
}

int mate_memfree(mate_instance *lib_ref, mate_pointer addr)
{
  log_info(logger, "MEM_FREE a la dirección %d", addr);
  enviar_cod_op_e_int(((mate_inner_structure *)lib_ref->group_info)->conexion, MEMFREE, addr);
  int respuesta = recibir_int(((mate_inner_structure *)lib_ref->group_info)->conexion);
  printf("Esperando respuesta memfree\n");
  if (respuesta == -5)
  {
    printf("Error al realizar el MEM_FREE\n");
    log_info(logger, "Error al realizar el MEM_FREE");
    return MATE_FREE_FAULT;
  }
  else
  {
    printf("Se realizó el MEM_FREE correctamente\n");
    log_info(logger, "Se realizó el MEM_FREE correctamente");
    return respuesta;
  }
}

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size) // revisar que retorna
{
  mate_inner_structure* info = (mate_inner_structure*)lib_ref->group_info;

  log_info(logger, "MEM_READ desde %d el size %d", origin, size);

  enviar_int(info->conexion, MEMREAD);
  enviar_int(info->conexion, origin);
  enviar_int(info->conexion, size);

  int estado = recibir_int(info->conexion);
  if (estado == -1){
    dest = NULL;
    log_info(logger, "Error al realizar el MEM_READ");
    return MATE_READ_FAULT;
  }

  int *tam = malloc(sizeof(int));
  void* buffer = recibir_buffer_t(tam, info->conexion);
  memcpy(dest, buffer, *tam);

  printf("El valor de dest es %s\n", (char*)dest);

  log_info(logger, "Se realizó el MEM_READ correctamente. El contenido es: %s", (char *)dest);
  free(buffer);
  free(tam);

  return 0;
}

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size)
{
  printf("Envie el el tamanio %d\n", size);
  mate_inner_structure* info = (mate_inner_structure *)lib_ref->group_info;
  log_info(logger, "MEM_WRITE el mensaje %s en %d", (char *)origin, dest);

  enviar_int(info->conexion, MEMWRITE);
  enviar_int(info->conexion, size);
  enviar_buffer(info->conexion, origin, size);
  enviar_int(((mate_inner_structure *)lib_ref->group_info)->conexion, dest);
  int recibido = recibir_operacion(((mate_inner_structure *)lib_ref->group_info)->conexion);

  if (recibido == -7)
  {
    log_info(logger, "Error al realizar MEM_WRITE");

    return MATE_WRITE_FAULT;
  }
  else
  {
    log_info(logger, "Se realizó el MEM_WRITE correctamente");
    return recibido;
  }
}

/*
ESTAS FUNCIONES DE MEMEMORIA DEBEN TENER LA ESPERA DE UNA CONFIRMACION POR PARTE DEL MODULO PARA SEGUIR EJECUTADO
INDEPENDIENTEMENTE DEL RETURN PARA EVITA QUE EL PROCEOS HAGA OTRA PETICION Y AUN NO HAYA TERMINADO DE HACER LO ANTERIOR
 */
//--------------------Auxiliar Functions-----------------//

// funcion al pedo pero sirve para lectura
int obtener_int(mate_instance *lib_ref, cod_int COD)
{
  void *buffer = leer_bloque(lib_ref->group_info, sizeof(int) * COD, sizeof(int));
  return *(int *)buffer;
}

// funcion al pedo pero sirve para lectura
void *leer_bloque(void *bloque, int offset, int tamanio)
{
  void *buffer = malloc(tamanio);
  memcpy(buffer, bloque + offset, tamanio);
  return buffer;
}

void *crear_mate_inner(int pid, int conexion, int server)
{

  void *bloque = malloc(sizeof(int) * 3 + sizeof(sem_t)); // verificar tamanio para semaforos
  int offset = 0;

  memcpy(bloque + offset, &pid, sizeof(int));
  offset += sizeof(int);
  memcpy(bloque + offset, &conexion, sizeof(int));
  offset += sizeof(int);
  memcpy(bloque + offset, &server, sizeof(int));
  offset += sizeof(int);

  return bloque;
}

mate_pointer recibir_dl(int socket)
{
  return recibir_int32(socket);
}

mate_pointer recibir_int32(int socket)
{
  void *buffer = malloc(sizeof(mate_pointer));
  recv(socket, buffer, sizeof(mate_pointer), 0);
  mate_pointer dl = *(mate_pointer *)buffer;
  free(buffer);
  return dl;
}

bool conectado_a_memoria(mate_instance *lib_ref)
{
  return !((mate_inner_structure *)lib_ref->group_info)->con_kernel;
}



void conexion_set_server(mate_inner_structure *mate_ref, char* respuesta, t_log* logger)
{ 
  if (strcmp(respuesta, "KERNEL") == 0)
    mate_ref->con_kernel = true;
  else
    mate_ref->con_kernel = false;

  char mensaje[50];
  sprintf(mensaje, "Conectado a %s", respuesta);
}
