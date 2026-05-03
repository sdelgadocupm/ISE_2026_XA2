#ifndef ThPWM_H

#define ThPWM_H

#include "main.h"
#include "cmsis_os2.h"   

void Thread_PWM (void *argument);                   // thread function
void inicializarPWM(void);
void inicializarTimer(void);
int Init_Thread_PWM (void);

extern osThreadId_t tid_PWM; 

#define ALARMA_ON 0x01
#define ALARMA_OFF 0x02

#endif