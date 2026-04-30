
#ifndef MASTER_H
#define MASTER_H

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "Memoria.h"
typedef enum{
  INI,
  RECIBIR,
  ESCRIBIR,
  ESPERA,
}Modos_master_t;

 extern osThreadId_t TID_MASTER;


void automata_master (void);

#endif