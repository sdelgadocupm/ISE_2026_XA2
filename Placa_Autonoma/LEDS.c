#include "LEDS.h"
#include "PWM.h"
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_LEDS;                        // thread id

static osTimerId_t ledBlinkTimer;
static volatile uint8_t alarma_activa = 0;
 
static void LedBlinkCb(void *argument) {
  (void)argument;
  if (alarma_activa) {
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);   // parpadeo rojo
  }
}
 
int Init_Thread_LEDS (void) {
 
  tid_LEDS = osThreadNew(Thread_LEDS, NULL, NULL);
  if (tid_LEDS == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_LEDS (void *argument) {
  (void)argument;
  
 ledBlinkTimer = osTimerNew(LedBlinkCb, osTimerPeriodic, NULL, NULL);
  if (ledBlinkTimer != NULL) {
    osTimerStart(ledBlinkTimer, 300);
  }
  while (1) {
    
    
 uint32_t flags = osThreadFlagsWait(ALARMA_ON_LEDS | ALARMA_OFF_LEDS,
                                       osFlagsWaitAny,
                                       osWaitForever);

    if (flags & ALARMA_ON_LEDS) {
      alarma_activa = 1;
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11 | GPIO_PIN_12, GPIO_PIN_SET); // base ON
    }

    if (flags & ALARMA_OFF_LEDS) {
      alarma_activa = 0;
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13 | GPIO_PIN_12, GPIO_PIN_SET);
       HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
     
    osThreadYield();                            // suspend thread
  }
}
}
