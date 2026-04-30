#include "UART.h"                          // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      Thread 1 'TID_UART_TX': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t TID_USART_TX;                        // thread id
 
; 

osMessageQueueId_t mid_TX_USARTQueue;
MSGQUEUE_OBJ_UART_TX_t msg_TX;


 
void UART_TX (void *argument) {
    Init_UART();
  while (1) {
    osStatusQueue = osMessageQueueGet(TID_USART_TX,&msg_TX,0U,osWaitForever);
      USARTdrv->Send(msg_TX.Mensaje,msg_TX.TamMens);
      osThreadFlagsWait(0x01,osFlagsWaitAny , osWaitForever);
    osThreadYield();                            // suspend thread
  }
}

//CREACIÓN DE LA CALLBACK BLOQUEANTE

void myUSART_callback(uint32_t event){
	
	if(event & ARM_USART_EVENT_SEND_COMPLETE){
		osThreadFlagsSet(TID_USART_TX, 0x01);	
	}
}	


void Init_UART(void)
{
 
  
  USARTdrv->Initialize(myUSART_callback);
	USARTdrv->PowerControl(ARM_POWER_FULL);
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS|
										ARM_USART_DATA_BITS_8|
										ARM_USART_PARITY_NONE|
										ARM_USART_STOP_BITS_1|
										ARM_USART_FLOW_CONTROL_NONE, 9600);
	
	USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
	USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
  
}