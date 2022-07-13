# CarpinchOS

Trabajo Practico - Sistemas Operativos - UTN FRBA 2021 2C

## Objetivos del Trabajo Práctico

El objetivo del trabajo práctico consiste en desarrollar una solución que permita la simulación de un sistema distribuido, donde los grupos tendrán que planificar procesos externos no implementados por los grupos, que ejecuten peticiones al sistema de recursos. El sistema deberá, mediante esta interacción, habilitar recursos de memoria (bajo un esquema de paginación pura), recursos de entrada-salida y semáforos.

#### Mediante la realización de este trabajo se espera que el alumno:

- Adquiera conceptos prácticos del uso de las distintas herramientas de programación e interfaces (APIs) que brindan los sistemas operativos.
- Entienda aspectos del diseño de un sistema operativo.
- Afirme diversos conceptos teóricos de la materia mediante la implementación práctica de algunos de ellos.
- Se familiarice con técnicas de programación de sistemas, como el empleo de makefiles, archivos de configuración y archivos de log.
- Conozca con grado de detalle la operatoria de Linux mediante la utilización de un lenguaje de programación de relativamente bajo nivel como C.

El enunciado completo del trabajo practico se puede encontrar [aca](https://docs.google.com/document/d/1BDpr5lfzOAqmOOgcAVg6rUqvMPUfCpMSz1u1J_Vjtac/edit# "aca")

## Importante

- El sistema funciona en las [maquinas virtuales](https://www.utnso.com.ar/recursos/maquinas-virtuales/) de la catedra 
- Los modulos **Memoria** y **Swap** estan optimizados para que funcionen fuera de las maquinas virtuales de la catedra. **Kernel** puede tirar errores en otro entorno

## Como correr

Orden en el cual ejecutar los modulos:
1. **Memoria**
2. **Swap**

**Kernel** puede ser ejecutado en cualquier momento..

Una vez que este todo corriendo podemos ejecutar los procesos que queremos probar. Ejemplos en la carpeta pruebas

