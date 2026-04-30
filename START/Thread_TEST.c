#include "cmsis_os2.h"                          // CMSIS RTOS header file
 #include "LEDS.h"
 #include "PWM.h"
 #include "Thread_TEST.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread;                        // thread id
 

 extern osThreadId_t tid_LEDS;
 extern osThreadId_t tid_PWM;
 
int Init_Thread (void) {
 
  tid_Thread = osThreadNew(Thread, NULL, NULL);
  if (tid_Thread == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread (void *argument) {
 
  while (1) {
    osThreadFlagsSet(tid_LEDS,ALARMA_ON_LEDS); // Insert thread code here...
    osThreadFlagsSet(tid_PWM,ALARMA_ON);
    osDelay(5000);
    osThreadFlagsSet(tid_LEDS,ALARMA_OFF_LEDS);
    osThreadFlagsSet(tid_PWM,ALARMA_OFF);
    osDelay(5000);
    
    
    osThreadYield();                            // suspend thread
  }
}
