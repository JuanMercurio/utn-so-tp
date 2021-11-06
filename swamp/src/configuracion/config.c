#include "config.h"

t_config* config;
t_cfg configuracion;

void obtener_config(){
    config = config_create("cfg/swamp.config");
    configuracion.IP = config_get_string_value(config, "IP");
    configuracion.ARCHIVOS_SWAP = config_get_string_value(config, "ARCHIVOS_SWAP");
    configuracion.ARCHIVOS_SWAP_list = config_get_array_value(config,"ARCHIVOS_SWAP");
    configuracion.PUERTO = config_get_string_value(config, "PUERTO");
    configuracion.MARCOS_MAXIMOS = config_get_int_value(config, "MARCOS_MAXIMOS");
    configuracion.RETARDO_SWAP = config_get_int_value(config, "RETARDO_SWAP");
    configuracion.TAMANIO_PAGINA = config_get_int_value(config, "TAMANIO_PAGINA");
    configuracion.TAMANIO_SWAP = config_get_int_value(config, "TAMANIO_SWAP");
}
