#include "Principal.h"                          // CMSIS RTOS header file

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Principal;                        // thread id
 
void Principal (void *argument);                   // thread function
 extern uint8_t ultimo_uid[4];
 static MSGQUEUE_OBJ_COM_t msg_com;
 volatile state_t modo; 
 static uint16_t temp = 100;
 
 //Umbrales globales
 uint16_t umbral_temp = 50;
 uint16_t umbral_co2 = 1200;
 uint16_t umbral_tvoc = 400;
 
 //Alarmas
 uint8_t alarma = 0;
 extern uint8_t remoto; 
int Init_Principal (void) {
	
	modo = BAJO_CONSUMO;
	
  //Inicialización LEDS
  Init_Thread_LEDS();
  
  //Inicialización PWM
  Init_Thread_PWM();
  
	//Inicialización RFID
	Init_ThRFID();
	
	//Inicialización Sens
	Init_ThAirQuality();
	
	//Inicialización RTC
	RTC_init();
	
	//Inicialización ThCom
	Init_ThCom();
	
	//Inicialización del hilo
  tid_Principal = osThreadNew(Principal, NULL, NULL);
	
  if (tid_Principal == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Principal (void *argument) {
	mid_ComQueue = osMessageQueueNew(10, sizeof(MSGQUEUE_OBJ_COM_t), NULL);
	
	while (1) {
    switch(modo)
    {
      
      case BAJO_CONSUMO:
				RTC_SetWakeUpEvery60s();
				//Enviar estado al servidor
				msg_com.TamMens = sprintf(msg_com.Mensaje, "%d %d\n", MODO, BAJO_CONS);
				osMessageQueuePut(mid_ComQueue, &msg_com, 0U, 0U);
			
				//INDICAR POR UART QUE SE VA A ENTRAR EN STOP
				msg_com.TamMens = sprintf(msg_com.Mensaje, "%d %d\n", ESTADO, STOP);
				osMessageQueuePut(mid_ComQueue, &msg_com, 0U, 0U);
			
				osDelay(100);
			
				// MODO DE BAJO CONSUMO STOP
				HAL_SuspendTick();  // si usas SysTick HAL; con RTOS puede variar
				HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
				HAL_ResumeTick();

				SystemClock_Config();
			
				//INDICAR POR UART QUE SE SALE DE STOP
				
				//MEDIR LA TEMPERATURA
				//temp = getTemp();
				
				msg_com.TamMens = sprintf(msg_com.Mensaje, "%d %d\n", TEMPERATURA, temp);
				osMessageQueuePut(mid_ComQueue, &msg_com, 0U, 0U);
				
				if(temp >= umbral_temp)
						modo = PREACTIVACION;
				else
						modo = BAJO_CONSUMO;
      break;
			
      case PREACTIVACION:
				alarma = 0;
				//Enviar estado al servidor
				msg_com.TamMens = sprintf(msg_com.Mensaje, "%d %d\n", ESTADO, RUN);
				osMessageQueuePut(mid_ComQueue, &msg_com, 0U, 0U);
			
				msg_com.TamMens = sprintf(msg_com.Mensaje, "%d %d\n", MODO, PREACT);
				osMessageQueuePut(mid_ComQueue, &msg_com, 0U, 0U);
				
				for(int i = 0; i < 10; i++){
					//Señalizar Medida
					osThreadFlagsSet(tid_AirQuality,0x02);


					//Obtener Medida
					air_sample_t s;
					osStatus_t status = osMessageQueueGet(mid_airQueue, &s, NULL, 2000);
					
					if (status == osOK && s.eco2 != 0xFFFF) {
						msg_com.TamMens = sprintf(msg_com.Mensaje, "%d %d\n", CO2, s.eco2);
						osMessageQueuePut(mid_ComQueue, &msg_com, 0U, 0U);
						
						msg_com.TamMens = sprintf(msg_com.Mensaje, "%d %d\n", TVOC, s.tvoc);
						osMessageQueuePut(mid_ComQueue, &msg_com, 0U, 0U);
						
						if (s.eco2 > umbral_co2 || s.tvoc > umbral_tvoc) {
							alarma = 1;
							break;
						}
					}
					osDelay(1000); //Siguiente muestra
				}
			modo = alarma ? ALARMA : BAJO_CONSUMO;
				
      break;
    
      case ALARMA:
				//Enviar información relevante al servidor (estado, medidas)
				msg_com.TamMens = sprintf(msg_com.Mensaje, "%d %d\n", MODO, ALAR);
				osMessageQueuePut(mid_ComQueue, &msg_com, 0U, 0U);
			
        //ACTIVAR LEDS Y PWM
				osThreadFlagsSet(tid_LEDS, ALARMA_ON_LEDS);
				osThreadFlagsSet(tid_PWM,  ALARMA_ON);
				//DESACTIVACIÓN ALARMA FALTA DESAC REMOTA
			
				osThreadFlagsWait(FLAG_DISARM_RFID,osFlagsWaitAny,osWaitForever);
			
				msg_com.TamMens = sprintf(msg_com.Mensaje, "%d %02X%02X%02X%02X\n", RFID, 
                      ultimo_uid[0], ultimo_uid[1], 
                      ultimo_uid[2], ultimo_uid[3]);
				//Apagar Alarma
			  osThreadFlagsSet(tid_PWM,  ALARMA_OFF);
				osThreadFlagsSet(tid_LEDS, ALARMA_OFF_LEDS);
				
				remoto = 0;
				alarma = 0;
				modo = BAJO_CONSUMO;
				
      break;
    }
  }
}
