#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "Memoria.h"
#include <stdio.h>  // Para printf
#include "Thread.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread;                        // thread id


 
 
 
 uint16_t temp = 2;
 uint16_t ca = 3;
 uint16_t consumo = 4;
 
int Init_Thread (void) {
 
  tid_Thread = osThreadNew(Thread, NULL, NULL);
  if (tid_Thread == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread (void *argument) {
 Init_I2C();
  while (1) {
    printf("\n[GUARDANDO] Temp=%u, CA=%u, Consumo=%u\r\n", temp, ca, consumo);
     fflush(stdout);
     guardar_medidad();// Insert thread code here...
    osDelay(500); // Esperar a que escriba en EEPROM
    debug_memoria();
    osThreadYield();                            // suspend thread
  }
}

void guardar_medidad(void)
{
  guardar_calidad_aire(ca);
  guardar_consumo(consumo);
  guardar_temperatura(temp);
  guardar_hora(12,30,25);
  osDelay(100);
  
}

void debug_memoria(void)
{
    uint16_t t, c,ca;
  uint8_t  h, m, s;
  
  leer_medidas(&t, &ca, &c, &h, &m, &s);
  
   printf("\r\n===== EEPROM DEBUG =====\r\n");
  printf("Temperatura: %u\r\n", t);
  printf("Calidad Aire: %u\r\n", ca);
  printf("Consumo: %u\r\n", c);
  printf("Hora: %02u:%02u:%02u\r\n", h, m, s);
  printf("========================\r\n");
  
}