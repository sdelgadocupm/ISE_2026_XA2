#ifndef ThPWM_H

#define ThPWM_H

#include "main.h"

void Thread_PWM (void *argument);                   // thread function
void inicializarPWM(void);
void inicializarTimer(void);
int Init_Thread_PWM (void);
#define ALARMA_ON 0x01
#define ALARMA_OFF 0x02

#endif