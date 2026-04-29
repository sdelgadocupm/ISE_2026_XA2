#ifndef ThCom_H

#define ThCom_H
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include "Driver_USART.h"

typedef struct{
	char Mensaje[64];   //DE TAMAÑO 64
	uint8_t TamMens;		 // EL TAMAÑO DEL MENSAJE	
}MSGQUEUE_OBJ_COM_t;

extern osMessageQueueId_t mid_ComQueue;   //Id de la cola

void ThCom(void *argument);                   // thread functiond);
int Init_ThCom (void);

#endif