#include "cmsis_os2.h"                          // CMSIS RTOS header file

#include <stdio.h>
#include <string.h>
#include "Recepcion.h"
#include "UARTManager.h"


/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThRecep;                        // thread id
 

static osStatus_t osStatusQueue;


extern uint16_t umbral_temp;
extern uint16_t umbral_co2;
extern uint16_t umbral_tvoc;
extern uint8_t remoto; 




//INICIALIZACIÓN DEL HILO	

int Init_ThRecep (void) {
 
  tid_ThRecep = osThreadNew(ThRecep, NULL, NULL);
  if (tid_ThRecep == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThRecep (void *argument) {
	
	uint8_t byte_entrante;
  uint8_t indice = 0;
  static char buffer_local[64];
	
	
  while (1) {
		
		UART_Receive(&byte_entrante, 1); 
		osThreadFlagsWait(0x02, osFlagsWaitAny , osWaitForever);
		
		
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
							printf("[RX] ? Setting temp threshold to %d\n", valor);
                umbral_temp = (uint16_t)valor;
                break;
            case 2: // Valor del umbral de eCO2
							printf("[RX] ? Setting CO2 threshold to %d\n", valor);
                umbral_co2 = (uint16_t)valor;
                break;
            case 3: // Valor del umbral de tvoc
							printf("[RX] ? Setting TVOC threshold to %d\n", valor);
                umbral_tvoc = (uint16_t)valor;
                break;
						case 4: // Activación remota de alarm
							printf("[RX] ? Remote alarm triggered\n");
                remoto = 1;
                break;
						default:
							printf("[RX] ? Unknown command ID: %d\n", id);
                // ID desconocido
                break;
            
        }
    }
}
