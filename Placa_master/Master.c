#include "cmsis_os2.h"                          // CMSIS RTOS header file
 #include "Master.h"
/*----------------------------------------------------------------------------
 *      MASTER
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Master_t;                        // thread id
 
void Master_Th (void *argument);                   // thread function

static  Master_Estado_t modo_master = INIT;
uint32_t tiempo = 0 ;
 
 
uint32_t tiempo_transcurrido = 0;

extern int Init_ThSNTP();

extern uint16_t temp;
extern uint16_t co2;
extern uint16_t tvoc;
extern uint16_t consumo;
extern uint16_t estado;
extern uint16_t modo;

extern osMessageQueueId_t mid_ComQueue;

// Variables globales de umbrales (ya inicializadas aquí)
uint16_t th_temp = 60;
uint16_t th_co2 = 1000;
uint16_t th_tvoc = 500;

/* ============================================================================
 * Inicialización
 * ========================================================================= */

int Init_Master(void) {
 
  tid_Master_t = osThreadNew(Master_Th, NULL, NULL);
  if (tid_Master_t == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Master_Th (void *argument) {
 
  while (1) {
     automata(); // Insert thread code here...
    osThreadYield();                            // suspend thread
  }
}

/* ============================================================================
 * Máquina de estados
 * ========================================================================= */

void automata (void){
 
  switch(modo_master)
  {
    case INIT:
      Inicializacion();
    
      break;
      
        case RECEPCION:
					// Calcular tiempo transcurrido desde envío
          tiempo_transcurrido = osKernelGetTickCount() - timeout_rx;
        
					// Esperar timeout de 5 segundos
          if (tiempo_transcurrido > RX_TIMEOUT_MS) {
          
						rx_confirmado = true;
					}
          if (rx_confirmado) {
						modo_master = WEB;
						rx_confirmado = false;
					}
             osDelay(100);
      
      break;
            case TRANSMISION:
							
							// Solo enviar si hay cambios pendientes
              if (tx_pendiente) {
								
								// Esperar a que SLAVE esté en RUN
								if (estado == 1) {
									printf("[MASTER] Slave en RUN, enviando umbrales\n");
									enviar_umbrales_a_slave();
									tx_pendiente = false;

									modo_master = RECEPCION;	

								} else {
									// SLAVE aún no en RUN, esperar
										osDelay(10);  
								}
						} else {
							// Sin cambios pendientes
								modo_master = RECEPCION;
						}
        break;
      
      break;
            
            case WEB:
                
              //CREAMOS UN CICLO QUE REVISE LA WEB CADA 5 SEG
              
							tiempo = osKernelGetTickCount();
              if((tiempo - last_check) > 5000){
                    last_check = tiempo;
              }
							
              if (tx_pendiente) {
								modo_master = TRANSMISION;
							} else {
							//printf("[MASTER] ? Esperando cambios...\n");
							}
						osDelay(500);
          
					break;
          default:
						printf("[MASTER] Estado desconocido\n");
						osDelay(100);
					break;
  }
  
}

void Inicializacion(void)
{
  //Inicializamos todo lo que vamos a usar, menos los hilos qeu lo haremos en el Server, en el  app_main
  Init_I2C();
	
  osDelay(100);
	
  mid_ComQueue = osMessageQueueNew(10, sizeof(MSGQUEUE_OBJ_COM_t), NULL);
 // Inicializar UART
  UART_Init();
	Init_ThCom();
	Init_ThRecep();

	Init_Logger();
	
	UART_SetThreadIds(tid_ThCom, tid_ThRecep);
  osDelay(100);
  
  
  //Inicilazar NSTP Y RTC 
  RTC_Init();
  Init_ThSNTP();
  
  printf("[MASTER] Cargando umbrales desde EEPROM...\n");
  leer_umbrales(&th_temp, &th_co2, &th_tvoc);
  
  osDelay(100);
  
  modo_master = TRANSMISION;
  tx_pendiente = true; 
}

void enviar_umbrales_a_slave(void)
{
  //Metemos los datos al slave de los umbrales
   printf("[MASTER-TX] Enviando umbrales al SLAVE...\n");
  
  send_uart_command(1, th_temp );
  printf("  ? Temp: 1 %d\n",th_temp );
  osDelay(150);
  
  send_uart_command(2, th_co2);
  printf("  ? CO2: 2 %d\n", th_co2);
  osDelay(150);

  send_uart_command(3, th_tvoc);
  printf("  ? TVOC: 3 %d\n", th_tvoc);
  osDelay(150);
  rx_confirmado = false;
  timeout_rx = osKernelGetTickCount();
}



static void send_uart_command(uint16_t id, uint16_t valor) {
  MSGQUEUE_OBJ_COM_t out;
  out.TamMens = sprintf(out.Mensaje, "\n%d %d\r\n", id, valor);   
  osMessageQueuePut(mid_ComQueue, &out, 0U, 100U);
}

// MARCAR CAMBIO DE UMBRALES (desde CGI) 
void Master_confirm(void) {
  tx_pendiente = true;
}
 

