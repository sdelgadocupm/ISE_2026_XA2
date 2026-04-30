#ifndef __UART_H
#define __UART_H

#include "Driver_USART.h"
#include "cmsis_os2.h"
#include "string.h"
#include "stdio.h"

extern ARM_DRIVER_USART Driver_USART3;

typedef struct
{
  	char Mensaje[128];   //DE TAMAÑO 128
	uint8_t TamMens;		 // EL TAMAÑO DEL MENSAJE	
}MSGQUEUE_OBJ_UART_TX_t;

static osStatus_t osStatusQueue;

 static ARM_DRIVER_USART * USARTdrv = &Driver_USART3;

void Init_UART(void);

void UART_TX (void *argument);                   // thread function


#endif