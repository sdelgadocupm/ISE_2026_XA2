#ifndef Thread_TEST_H
#define Thread_TEST_H

#include "cmsis_os2.h"                          // CMSIS RTOS header file

#include "main.h"


void Thread (void *argument);                   // thread function
int Init_Thread (void) ;

#endif