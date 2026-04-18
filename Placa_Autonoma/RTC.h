#ifndef __RTC_H
#define __RTC_H

/*Includes*/

#include "main.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os2.h"
/*Funciones*/

void RTC_CalendarConfig(void);
void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate);
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc);                  
void RTC_SetAlarm(void);
void RTC_init(void);
/*Definiciones*/
#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */

extern RTC_HandleTypeDef RtcHandle;

/* Buffers used for displaying Time and Date */
extern uint8_t aShowTime[50];
extern uint8_t aShowDate[50];

#endif
