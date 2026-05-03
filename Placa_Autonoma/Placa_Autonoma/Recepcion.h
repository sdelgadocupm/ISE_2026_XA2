#ifndef RECEPCION_H

#define RECEPCION_H
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include "Driver_USART.h"


void ThRecep(void *argument);                   // thread functiond);
int Init_ThRecep (void);
void ProcesarTrama(char *buffer);
#endif