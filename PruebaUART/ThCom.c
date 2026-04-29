#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include <stdio.h>
#include <string.h>
#include "ThCom.h"
#include "UARTManager.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThCom;                        // thread id
 

 
extern ARM_DRIVER_USART Driver_USART3; 

static osStatus_t osStatusQueue;

//COLA DE MENSAJES A ENVIAR POR LA USART
osMessageQueueId_t mid_ComQueue;
MSGQUEUE_OBJ_COM_t msgCom;


//INICIALIZACI�N DEL HILO	

int Init_ThCom (void) {
 
  tid_ThCom = osThreadNew(ThCom, NULL, NULL);
  if (tid_ThCom == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThCom (void *argument) {
  while (1) {
		osStatusQueue = osMessageQueueGet(mid_ComQueue, &msgCom, 0U, osWaitForever);
		UART_Send(msgCom.Mensaje, msgCom.TamMens);  
		printf("Enviando: %s (bytes: %d)\n", msgCom.Mensaje, msgCom.TamMens);
		osThreadFlagsWait(0x01, osFlagsWaitAny , osWaitForever);
    osThreadYield();                            
  }
}
