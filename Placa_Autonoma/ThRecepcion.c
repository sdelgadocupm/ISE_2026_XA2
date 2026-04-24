#include "cmsis_os2.h"                          // CMSIS RTOS header file

#include <stdio.h>
#include <string.h>
#include "Recepcion.h"


/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThRecep;                        // thread id
 
extern ARM_DRIVER_USART Driver_USART3; 

static osStatus_t osStatusQueue;


extern uint16_t umbral_temp;
extern uint16_t umbral_co2;
extern uint16_t umbral_tvoc;
extern uint8_t remoto; 

//CREACIÓN DE LA CALLBACK BLOQUEANTE

static void myUSART_callback(uint32_t event){
	
	if(event & ARM_USART_EVENT_RECEIVE_COMPLETE){
		osThreadFlagsSet(tid_ThRecep, 0x01);	
	}
}	

//INICIALIZACIÓN DEL HILO	

int Init_ThRecep (void) {
 
  tid_ThRecep = osThreadNew(ThRecep, NULL, NULL);
  if (tid_ThRecep == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThRecep (void *argument) {
	static ARM_DRIVER_USART * USARTdrv = &Driver_USART3;
	
	uint8_t byte_entrante;
  uint8_t indice = 0;
  static char buffer_local[64];
	
 	USARTdrv->Initialize(myUSART_callback);
	USARTdrv->PowerControl(ARM_POWER_FULL);
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS|
										ARM_USART_DATA_BITS_8|
										ARM_USART_PARITY_NONE|
										ARM_USART_STOP_BITS_1|
										ARM_USART_FLOW_CONTROL_NONE, 9600); // LO ÚLTIMO ES EL BAUD RATE
	

	USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
	
  while (1) {
		
		USARTdrv->Receive(&byte_entrante, 1);  
		osThreadFlagsWait(0x01, osFlagsWaitAny , osWaitForever);
		
		
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
}

void ProcesarTrama(char *buffer) {
    int id, valor;
    // sscanf busca el primer entero, salta el espacio y busca el segundo
    if (sscanf(buffer, "%d %d", &id, &valor) == 2) {
        switch(id) {
            case 1: // Valor de Umbral de Temperatura
                umbral_temp = (uint16_t)valor;
                break;
            case 2: // Valor del umbral de eCO2
                umbral_co2 = (uint16_t)valor;
                break;
            case 3: // Valor del umbral de tvoc
                umbral_tvoc = (uint16_t)valor;
                break;
						case 4: // Activación remota de alarm
                remoto = 1;
                break;
						default:
                // ID desconocido
                break;
            
        }
    }
}
