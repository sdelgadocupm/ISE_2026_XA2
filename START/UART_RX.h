#ifndef __UART_RX_H
#define __UART_RX_H

#include "Driver_USART.h"
#include "cmsis_os2.h"
#include "string.h"
#include "stdio.h"
#include "Memoria.h"
#include "Master.h"
extern ARM_DRIVER_USART Driver_USART3;



static osStatus_t osStatusQueue;

typedef struct
{
  float temp;
  int  calidad_aire;
  float consumo;
  
}MEDIDAS_t;

 static ARM_DRIVER_USART * USARTdrv = &Driver_USART3;
void ProcesarTrama (char *buffer);
void Init_UART(void);

void UART_RX (void *argument);                   // thread function


#endif