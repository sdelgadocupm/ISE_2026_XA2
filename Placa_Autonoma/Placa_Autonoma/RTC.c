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

extern osThreadId_t TID_Led;  


/*----------------------------------------------------------------------------
 *       CONFIGURACIÓN INICIAL DEL RTC
 *---------------------------------------------------------------------------*/

void RTC_init(void){
	
	HAL_RTC_MspInit(&RtcHandle);
	
	RtcHandle.Instance = RTC; 
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  __HAL_RTC_RESET_HANDLE_STATE(&RtcHandle);
	HAL_RTC_Init(&RtcHandle);
  /*##-2- Check if Data stored in BackUp register1: No Need to reconfigure RTC#*/
  /* Read the Back Up Register 1 Data */
  if (HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR1) != 0x32F2)
  {
    /* Configure RTC Calendar */
    RTC_CalendarConfig();
  }
  else
  {
		if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
    {
      /* Turn on LED2: Power on reset occurred */
   
    }
    /* Check if Pin Reset flag is set */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
    {
      /* Turn on LED1: External reset occurred */

    }
    /* Clear source Reset Flag */
    __HAL_RCC_CLEAR_RESET_FLAGS();
  }
	
}


/*----------------------------------------------------------------------------
 *      FUNCIÓN QUE DESPIERTA AL SISTEMA CADA 10 SEGUNDOS
 *---------------------------------------------------------------------------*/

void RTC_SetWakeUpEvery10s(void){
	
  // 1) Desactivar por si estaba activo
  HAL_RTCEx_DeactivateWakeUpTimer(&RtcHandle);

  // 2) Limpiar flags
  __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RtcHandle, RTC_FLAG_WUTF);
  __HAL_RTC_EXTI_CLEAR_FLAG(RTC_EXTI_LINE_WAKEUPTIMER_EVENT);

  // 3) Programar 10 segundos

  if (HAL_RTCEx_SetWakeUpTimer_IT(&RtcHandle,10,  // segundos
		RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK) {
  }

  // 4) Habilitar la interrupción del Wake-up
  HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
}

void RTC_WKUP_IRQHandler(void){
	//Handler que causa que se despierte el sistema
  HAL_RTCEx_WakeUpTimerIRQHandler(&RtcHandle);
}
