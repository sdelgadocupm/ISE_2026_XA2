#include "UART_RX.h"                          // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      Thread 1 'TID_UART_TX': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t TID_USART_RX;                        // thread id
 
extern ARM_DRIVER_USART Driver_USART3;

MEDIDAS_t meas_global;
 
void UART_RX (void *argument) {
    Init_UART();
  	uint8_t byte_entrante;
  uint8_t indice = 0;
  static char buffer_local[64];
  uint8_t buf_rx[2];
  while (1) {
    
      USARTdrv->Receive(&byte_entrante, 1);  
    osThreadFlagsWait(0x01,osFlagsWaitAny , osWaitForever);
    
    if (byte_entrante == '\n' || byte_entrante == '\r') {
			if (indice > 0) {
        buffer_local[indice] = '\0'; // Cerramos el string
        ProcesarTrama(buffer_local);  // Analizamos Trama
        indice = 0; // Reset para la próxima vez
       }
     }else {
       // Vamos guardando en el buffer si hay espacio
       if (indice < 63) {
         buffer_local[indice++] = byte_entrante;
			}
		}
  }
    
    
    osThreadYield();                            // suspend thread
  }


//CREACIÓN DE LA CALLBACK BLOQUEANTE

void myUSART_callback(uint32_t event){
	
	if(event & ARM_USART_EVENT_SEND_COMPLETE){
		osThreadFlagsSet(TID_USART_RX, 0x01);	
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
	

	USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
}

void ProcesarTrama (char *buffer)
{
  int id, valor;
  
    // sscanf busca el primer entero, salta el espacio y busca el segundo
    if (sscanf(buffer, "%d %d", &id, &valor) == 2) {
        switch(id) {
            case 1: // Valor de Umbral de Temperatura
                
                break;
            case 2: // Valor del umbral de eCO2
                
                break;
            case 3: // Valor del umbral de tvoc
                
                break;
						case 4: // Activación remota de alarm
                
                break;
						default:
                // ID desconocido
                break;
}
 }
}