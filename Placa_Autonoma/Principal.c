#include "Principal.h"                          // CMSIS RTOS header file

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Principal;                        // thread id
 
void Principal (void *argument);                   // thread function
 
 
 volatile state_t modo; 
 static uint32_t temp = 0;
 uint8_t alarma = 0;
int Init_Principal (void) {
	
	modo = BAJO_CONSUMO;
	
	//Inicialización RFID
	Init_ThRFID();
	
	//Inicialización Sens
	Init_ThAirQuality();
	
	//Inicialización RTC
	RTC_init();
	
	//Inicialización del hilo
  tid_Principal = osThreadNew(Principal, NULL, NULL);
	
  if (tid_Principal == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Principal (void *argument) {
 
	
	while (1) {
    switch(modo)
    {
      
      case BAJO_CONSUMO:
				  RTC_SetWakeUpEvery60s();
				//Enviar estado al servidor
			
				//INDICAR POR UART QUE SE VA A ENTRAR EN STOP
			
				// MODO DE BAJO CONSUMO STOP
				HAL_SuspendTick();  // si usas SysTick HAL; con RTOS puede variar
				HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
				HAL_ResumeTick();

				SystemClock_Config();
			
				//INDICAR POR UART QUE SE SALE DE STOP
				 
				//MEDIR LA TEMPERATURA
				//temp = getTemp();
				if(temp >= TEMP_UMBRAL)
						modo = PREACTIVACION;
				else
						modo = BAJO_CONSUMO;
      break;
			
      case PREACTIVACION:
				alarma = 0;
				//Enviar estado al servidor

				for(int i = 0; i < 10; i++){
					//Señalizar Medida
					osThreadFlagsSet(tid_AirQuality,0x02);
					
					//Obtener Medida
					air_sample_t s;
					osStatus_t status = osMessageQueueGet(mid_airQueue, &s, NULL, 2000);
					
					if (status == osOK && s.eco2 != 0xFFFF) {
						printf("CO2: %d ", s.eco2);
						printf("TVOC: %d\n", s.tvoc);
						if (s.eco2 > ECO2_UMBRAL || s.tvoc > TVOC_UMBRAL) {
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
			
        //ACTIVAR LEDS Y PWM
				osThreadFlagsSet(tid_LEDS, ALARMA_ON_LEDS);
				osThreadFlagsSet(tid_PWM,  ALARMA_ON);
				//DESACTIVACIÓN ALARMA FALTA DESAC REMOTA
			
				osThreadFlagsWait(FLAG_DISARM_RFID,osFlagsWaitAny,osWaitForever);
				
				//Apagar Alarma
			  osThreadFlagsSet(tid_PWM,  ALARMA_OFF);
				osThreadFlagsSet(tid_LEDS, ALARMA_OFF_LEDS);
			
				alarma = 0;
				modo = BAJO_CONSUMO;
				
      break;
    }
  }
}
