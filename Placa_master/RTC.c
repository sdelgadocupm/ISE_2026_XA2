#include "RTC.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
RTC_HandleTypeDef RtcHandle;
uint8_t aShowTime[50] = {0};
uint8_t aShowDate[50] = {0};

void RTC_CalendarConfig(void){
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;

  /*##-1- Configure the Date #################################################*/
  /* Set Date: Tuesday February 18th 2014 */
  sdatestructure.Year = 0x14;
  sdatestructure.Month = RTC_MONTH_FEBRUARY;
  sdatestructure.Date = 0x18;
  sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;
  

  /*##-2- Configure the Time #################################################*/
  /* Set Time: 02:00:00 */
  stimestructure.Hours = 0x02;
  stimestructure.Minutes = 0x00;
  stimestructure.Seconds = 0x00;
  stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

	
	HAL_RTC_SetDate(&RtcHandle, &sdatestructure, RTC_FORMAT_BCD);
  HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BCD);

	
  /*##-3- Writes a data in a RTC Backup data Register1 #######################*/
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, 0x32F2);
}

void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate){
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;

  /* Get the RTC current Time */
  HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);
  /* Display time Format : hh:mm:ss */
  sprintf((char *)showtime, "%2d:%2d:%2d", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
  /* Display date Format : mm-dd-yy */
  sprintf((char *)showdate, "%2d-%2d-%2d", sdatestructureget.Month, sdatestructureget.Date, 2000 + sdatestructureget.Year);
}

void RTC_SetAlarm(void){
  RTC_TimeTypeDef stime;
  RTC_AlarmTypeDef sAlarm;
  
  HAL_RTC_GetTime(&RtcHandle,&stime, RTC_FORMAT_BIN);
  
  sAlarm.AlarmTime.Hours = stime.Hours;
  sAlarm.AlarmTime.Minutes = (stime.Minutes+1)%60;
  sAlarm.AlarmTime.Seconds = 0;

  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
  sAlarm.AlarmSubSecondMask= RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay= 1;
  sAlarm.Alarm = RTC_ALARM_A;
  
  HAL_RTC_SetAlarm_IT(&RtcHandle,&sAlarm,RTC_FORMAT_BIN); 


}

void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc) {
	
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    // Habilitar el reloj del Power Controller (PWR)
    __HAL_RCC_PWR_CLK_ENABLE();

    // Habilitar el acceso al dominio de Backup (RTC)
    HAL_PWR_EnableBkUpAccess();

    //Configurar el oscilador LSE
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        
    }

    // Habilitar el reloj del periférico RTC
  __HAL_RCC_RTC_ENABLE();
} 


void RTC_Init(void) {
    RtcHandle.Instance = RTC;
    RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
    RtcHandle.Init.AsynchPrediv = 127;
    RtcHandle.Init.SynchPrediv = 255;
    RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
    HAL_RTC_Init(&RtcHandle);

    // Si no tiene la marca de seguridad, lo configuramos por defecto
    if (HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR1) != 0x32F2) {
        RTC_CalendarConfig();
    }
}