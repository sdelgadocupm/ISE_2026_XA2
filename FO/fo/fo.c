#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "fo.h"
#include "Driver_USART.h"
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThfoRX;                        // thread id
osThreadId_t tid_ThfoTX;                        // thread id
 
void ThfoRX (void *argument);                   // thread function
void ThfoTX (void *argument);                   // thread function

uint8_t nuevo_comando=1;


#define MSGQUEUE_OBJECTS 16                     // number of Message Queue Objects
 
typedef struct {                                // object data type
  uint8_t data[8];
} MSGQUEUE_OBJ_COM_t;
 
osMessageQueueId_t mid_MsgQueue_fo_TX;                // message queue id
osMessageQueueId_t mid_MsgQueue_fo_RX;                // message queue id


/* USART Driver */
extern ARM_DRIVER_USART Driver_USART3;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART3;

void myUSART_callback_FO(uint32_t event);
void initUSART(void);
 
int Init_Thfo (void) {
	
	mid_MsgQueue_fo_TX = osMessageQueueNew(MSGQUEUE_OBJECTS, sizeof(MSGQUEUE_OBJ_COM_t), NULL);
  if (mid_MsgQueue_fo_TX == NULL) {
    ; // Message Queue object not created, handle failure
  }
	mid_MsgQueue_fo_RX = osMessageQueueNew(MSGQUEUE_OBJECTS, sizeof(MSGQUEUE_OBJ_COM_t), NULL);
  if (mid_MsgQueue_fo_RX == NULL) {
    ; // Message Queue object not created, handle failure
  }
 
	initUSART();
	
  tid_ThfoRX = osThreadNew(ThfoRX, NULL, NULL);
  if (tid_ThfoRX == NULL) {
    return(-1);
  }

  tid_ThfoTX = osThreadNew(ThfoTX, NULL, NULL);
  if (tid_ThfoTX == NULL) {
    return(-1);
  }
	
  return(0);
}
 
void ThfoRX (void *argument) {
	
	
	uint8_t i;
	MSGQUEUE_OBJ_COM_t msg_rx;

	char cmd;
 
  while (1) {
		if(nuevo_comando){
			for(i=0;i<8;i++){
				msg_rx.data[i]=0x00;
			}
			nuevo_comando=0;
		}
		
		USARTdrv->Receive(&cmd, 1);
		osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
		if(cmd==0x7E){
			msg_rx.data[0]=cmd;
			i=1;
			do{
				USARTdrv->Receive(&cmd, 1);
				osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
				msg_rx.data[i]=cmd;
				i++;
			}while(cmd!=0xFE);
			osMessageQueuePut(mid_MsgQueue_fo_RX, &msg_rx, NULL, 0U);
			
		}

    osThreadYield();                            // suspend thread
  }
}

void ThfoTX (void *argument) {

	MSGQUEUE_OBJ_COM_t msg_tx;
	uint8_t i;
			
	while(1){
		for(i=0;i<8;i++){
			msg_tx.data[i]=0x00;
		}
		osMessageQueueGet(mid_MsgQueue_fo_TX, &msg_tx, NULL, osWaitForever);
		
		for(i=0;i<37;i++){
			USARTdrv->Send(&msg_tx.data[i], 1);
			osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
		}
		nuevo_comando=1;

		
		
	}

}

void myUSART_callback_FO(uint32_t event)
{
  uint32_t mask1;
	uint32_t mask2;
  mask1 = ARM_USART_EVENT_RECEIVE_COMPLETE  |
         ARM_USART_EVENT_TRANSFER_COMPLETE |
         ARM_USART_EVENT_SEND_COMPLETE     |
         ARM_USART_EVENT_TX_COMPLETE       ;
	mask2 = ARM_USART_EVENT_SEND_COMPLETE     |
         ARM_USART_EVENT_TX_COMPLETE       ;
  if (event & mask1) {
    /* Success: Wakeup Thread */
    osThreadFlagsSet(tid_ThfoRX, 0x01);
  }
	if (event & mask2) {
    /* Success: Wakeup Thread */
    osThreadFlagsSet(tid_ThfoTX, 0x01);
  }
  if (event & ARM_USART_EVENT_RX_TIMEOUT) {
    /* Error: Call debugger or replace with custom error handling */
  }
  if (event & (ARM_USART_EVENT_RX_OVERFLOW | ARM_USART_EVENT_TX_UNDERFLOW)) {
		/* Error: Call debugger or replace with custom error handling */
  }
}

void initUSART(void){
	
	/*Initialize the USART driver */
	USARTdrv->Initialize(myUSART_callback_FO);
	/*Power up the USART peripheral */
	USARTdrv->PowerControl(ARM_POWER_FULL);
	/*Configure the USART to 4800 Bits/sec */
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
										ARM_USART_DATA_BITS_8 |
										ARM_USART_PARITY_NONE |
										ARM_USART_STOP_BITS_1 |
										ARM_USART_FLOW_CONTROL_NONE, 115200);
	 
	/* Enable Receiver and Transmitter lines */
	USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
	USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
	
	
}


