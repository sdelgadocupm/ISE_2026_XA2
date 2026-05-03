#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "RTC.h"
#include <time.h>
#include "rl_net.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

osThreadId_t tid_ThSNTP;                        // thread id

void ThSNTP (void *argument);                   // thread function

void sntp_client_cb (uint32_t seconds, uint32_t seconds_fraction);
const NET_ADDR ntp_server = {NET_ADDR_IP4,0,216,239,35,0};
int Init_ThSNTP (void) {

  tid_ThSNTP = osThreadNew(ThSNTP, NULL, NULL);

  if (tid_ThSNTP == NULL) {
    return(-1);
  }
  return(0);
}

 
void ThSNTP (void *argument) {

    osDelay(5000); // Espera inicial de 5s tras el arranque
  while (1) {

    netSNTPc_GetTime((NET_ADDR*)&ntp_server, sntp_client_cb);
    osDelay(100); 
    osDelay(179900); 
  }
 }

void sntp_client_cb (uint32_t seconds, uint32_t seconds_fraction) {

    struct tm ts;
    uint32_t UTC_seconds = seconds+3600;
  
    ts = *localtime(&UTC_seconds);

    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sTime.Hours = ts.tm_hour;
    sTime.Minutes = ts.tm_min;
    sTime.Seconds = ts.tm_sec;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    if (HAL_RTC_SetTime(&RtcHandle, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
        return;
    }
    sDate.WeekDay = ts.tm_wday + 1;
    sDate.Month = ts.tm_mon + 1;      
    sDate.Date = ts.tm_mday;
    sDate.Year = ts.tm_year - 100;

    if (HAL_RTC_SetDate(&RtcHandle, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
        return;
    }
}
