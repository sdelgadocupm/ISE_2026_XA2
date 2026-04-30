#include "cmsis_os2.h"                          // CMSIS RTOS header file

#include <stdio.h>
#include <string.h>
#include "Recepcion.h"
#include "UARTManager.h"
#include "Logger.h"


/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThRecep;                        // thread id
 

static osStatus_t osStatusQueue;

MSGQUEUE_ALARM_t msg;

osMessageQueueId_t mid_AlarmQueue;

uint16_t temp;
uint16_t co2;
uint16_t tvoc;
uint8_t RFID[4];
uint16_t consumo; 
uint16_t estado; 
uint16_t modo; 
uint16_t modo_ant; 


//INICIALIZACIÓN DEL HILO	

int Init_ThRecep (void) {
	
  mid_AlarmQueue = osMessageQueueNew(10, sizeof(MSGQUEUE_ALARM_t), NULL);
    if (mid_AlarmQueue == NULL) {
        return(-1);
    }
		
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
            case 1: // Valor de Temperatura
							printf("[RX] ? Temp es %d\n", valor);
              temp = (uint16_t)valor;
						  msg.temperatura = temp;
              break;
            case 2: // Valor de eCO2
							printf("[RX] ? CO2 es %d\n", valor);
              co2 = (uint16_t)valor;
							msg.eco2 = co2;
              break;
            case 3: // Valor de tvoc
							printf("[RX] ? TVOC es %d\n", valor);
              tvoc = (uint16_t)valor;
							msg.tvoc = tvoc;
              break;
						case 4: // Modo
							printf("[RX] ? Modo es %d\n", valor);
							modo = (uint16_t)valor;
                if(modo == 2){
									msg.tipo_evento = 0;
									osMessageQueuePut(mid_AlarmQueue, &msg, 0U, 0U);
								}else if(modo == 1 && modo_ant == 2){
									msg.tipo_evento = 1;
									osMessageQueuePut(mid_AlarmQueue, &msg, 0U, 0U);
								}
								modo_ant = modo;
              break;
						case 5: // Estado
								printf("[RX] ? Estado es %d \n", valor);
								estado = (uint16_t)valor;
                break;
						case 6: // Valor RFID
								printf("[RX] ? RFID es %d\n");
								RFID[0] = (valor >> 24) & 0xFF;
                RFID[1] = (valor >> 16) & 0xFF;
                RFID[2] = (valor >> 8) & 0xFF;
                RFID[3] = valor & 0xFF;
                
                // Copiar al mensaje
                msg.rfid[0] = RFID[0];
                msg.rfid[1] = RFID[1];
                msg.rfid[2] = RFID[2];
                msg.rfid[3] = RFID[3];
                break;
						case 7: // Consumo
								printf("[RX] ? Consumo es %d\n");
								consumo = (uint16_t)valor;
                break;
            case 8: //mostrar historial
                 printf("[RX] Comando: Ver historial\n");
                enviar_historial_eventos();
              break;
						default:
							printf("[RX] ? Unknown command ID: %d\n", id);
                // ID desconocido
                break;
            
        }
    }
}
