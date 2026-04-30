#ifndef LEDS_H
#define LEDS_H

#include "cmsis_os2.h"                          // CMSIS RTOS header file

#include "main.h"


void Thread_LEDS (void *argument);                   // thread function
void initLeds (void);
int Init_Thread_LEDS (void);
#define ALARMA_ON_LEDS 0x04
#define ALARMA_OFF_LEDS 0x08
#endif