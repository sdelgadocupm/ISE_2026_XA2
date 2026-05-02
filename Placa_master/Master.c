#include "cmsis_os2.h"                          // CMSIS RTOS header file
 #include "Master.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Master_t;                        // thread id
 
void Master_Th (void *argument);                   // thread function
static  Master_Estado_t modo_master = INIT;
uint32_t tiempo = 0 ;
 
 
uint32_t tiempo_transcurrido = 0;


extern uint16_t temp;
extern uint16_t co2;
extern uint16_t tvoc;
extern uint16_t consumo;
extern uint16_t estado;
extern uint16_t modo;
extern osMessageQueueId_t mid_ComQueue;

// Variables globales de umbrales (ya inicializadas aquí)
float th_temp = 60.0f;
int th_co2 = 1000;
int th_tvoc = 500;

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





void automata (void)
{
 
  switch(modo_master)
  {
    case INIT:
      Inicializacion();
    
      break;
      
        case RECEPCION:
          printf("[MASTER] Estado: RECEPCION (esperando confirmación del SLAVE)\n");
          tiempo_transcurrido = osKernelGetTickCount() - timeout_rx;
        
            if (tiempo_transcurrido > RX_TIMEOUT_MS) {
          printf("[MASTER] ? Confirmación recibida (timeout OK)\n");
          rx_confirmado = true;
        }
             if (rx_confirmado) {
          printf("[MASTER] ? Transición a WEB\n\n");
          modo_master = WEB;
          rx_confirmado = false;
        }
             osDelay(100);
      
      break;
            case TRANSMISION:
              printf("[MASTER] Estado: TRANSMISION\n");
              if (tx_pendiente) {
              enviar_umbrales_a_slave();
              tx_pendiente = false;
        }
              
        modo_master = RECEPCION;
        printf("[MASTER] ? Transición a RECEPCION\n\n");
        break;
      
      break;
            
              case WEB:
                
              /**CREAMOS UN CICLO QUE REVISE LA WEB CADA 5 SEG**/
              
                 tiempo = osKernelGetTickCount();
              if((tiempo - last_check) > 5000)
              {
               printf("[MASTER] Estado: WEB (VERIFICACION DE DATOS )\n");
                    last_check = tiempo;
              }
                if (tx_pendiente) {
            printf("[MASTER] ? Cambios detectados, volviendo a TRANSMISION\n\n");
            modo_master = TRANSMISION;
          } else {
            printf("[MASTER] ? Esperando cambios...\n");
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
  printf("[MASTER] === Inicializando Master ===\n");
  //Inicializamos todo lo que vamos a usar, menos los hilos qeu lo haremos en el Server, en el  app_main
  printf("[MASTER] Init I2C...\n");
  Init_I2C();
  osDelay(100);
  
 // Inicializar UART
  printf("[MASTER] Init UART...\n");
  UART_Init();
  osDelay(100);
  
  
  //Inicilazaira NSTP Y RTC 
  
  
  
  printf("[MASTER] Cargando umbrales desde EEPROM...\n");
  leer_umbrales(&th_temp, &th_co2, &th_tvoc);
  
  printf("[MASTER] Umbrales confirmados: T=%.1f, CO2=%d, TVOC=%d\n", 
         th_temp, th_co2, th_tvoc);
  osDelay(100);
  
   printf("[MASTER] === Inicialización completa ===\n\n");
  
  modo_master = TRANSMISION;
  tx_pendiente = true; 
}

void enviar_umbrales_a_slave(void)
{
  //Metemos los datos al slave de los umbrales
   printf("[MASTER-TX] Enviando umbrales al SLAVE...\n");
  
    send_uart_command(1, (int)(th_temp * 10.0f));
  printf("  ? Temp: 1 %d\n", (int)(th_temp * 10.0f));
  osDelay(50);
  
  send_uart_command(2, th_co2);
  printf("  ? CO2: 2 %d\n", th_co2);
  osDelay(50);

  send_uart_command(3, th_tvoc);
  printf("  ? TVOC: 3 %d\n", th_tvoc);
  
  rx_confirmado = false;
  timeout_rx = osKernelGetTickCount();
}



static void send_uart_command(int id, int valor) {
  MSGQUEUE_OBJ_COM_t out;
  snprintf(out.Mensaje, sizeof(out.Mensaje), "%d %d\n", id, valor);
  out.TamMens = (uint8_t)strlen(out.Mensaje);
  osMessageQueuePut(mid_ComQueue, &out, 0U, 0U);
}

// MARCAR CAMBIO DE UMBRALES (desde CGI) 
void Master_confirm(void) {
  printf("[MASTER] ?? Cambio de umbrales detectado\n");
  tx_pendiente = true;
}
 

