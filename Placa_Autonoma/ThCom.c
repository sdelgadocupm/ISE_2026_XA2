#include "cmsis_os2.h"                          // CMSIS RTOS header file

#include <stdio.h>
#include <string.h>
#include "ThCom.h"


/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThCom;                        // thread id
 

 
extern ARM_DRIVER_USART Driver_USART3; 

static osStatus_t osStatusQueue;

//COLA DE MENSAJES A ENVIAR POR LA USART

osMessageQueueId_t mid_ComQueue;
MSGQUEUE_OBJ_COM_t msgCom;



//CREACIÓN DE LA CALLBACK BLOQUEANTE

void myUSART_callback(uint32_t event){
	
	if(event & ARM_USART_EVENT_SEND_COMPLETE){
		osThreadFlagsSet(tid_ThCom, 0x01);	
	}
}	

//INICIALIZACIÓN DEL HILO	

int Init_ThCom (void) {
 
  tid_ThCom = osThreadNew(ThCom, NULL, NULL);
  if (tid_ThCom == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThCom (void *argument) {
	static ARM_DRIVER_USART * USARTdrv = &Driver_USART3;
	
 	USARTdrv->Initialize(myUSART_callback);
	USARTdrv->PowerControl(ARM_POWER_FULL);
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS|
										ARM_USART_DATA_BITS_8|
										ARM_USART_PARITY_NONE|
										ARM_USART_STOP_BITS_1|
										ARM_USART_FLOW_CONTROL_NONE, 9600); // LO ÚLTIMO ES EL BAUD RATE
	
	USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
	USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
	
  while (1) {
		osStatusQueue = osMessageQueueGet(mid_ComQueue, &msgCom, 0U, osWaitForever);
		USARTdrv->Send(msgCom.Mensaje, msgCom.TamMens);  
		osThreadFlagsWait(0x01, osFlagsWaitAny , osWaitForever);
    osThreadYield();                            // suspend thread
  }
}
