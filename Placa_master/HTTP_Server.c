/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include <stdio.h>

#include "main.h"
#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "Master.h"
#include "Board_Buttons.h"              // ::Board Support:Buttons
//#include "Board_ADC.h"                  // ::Board Support:A/D Converter
//#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD


// Declaración externa del manejador RTC
extern RTC_HandleTypeDef RtcHandle;
extern void RTC_SetAlarm(void);


// Main stack size must be multiple of 8 Bytes
#define APP_MAIN_STK_SZ (1024U)
uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};


extern uint8_t  get_button     (void);
extern void     netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len);


__NO_RETURN void app_main (void *arg);

extern void UART_TX (void *arg);
extern void UART_RX (void *arg);





/* Thread IDs */
osThreadId_t TID_MASTER;
extern osThreadId_t TID_UART_TX;
extern osThreadId_t TID_UART_RX;
                           
   
/* Read digital inputs */
uint8_t get_button (void) {
  return ((uint8_t)Buttons_GetState ());
}

/* IP address change notification */
void netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len) {

  (void)if_num;
  (void)val;
  (void)len;

  if (option == NET_DHCP_OPTION_IP_ADDRESS) {
    /* IP address change, trigger LCD update */
    
  }
}

/*******HILO DEL MASTER*******/
__NO_RETURN void Master (void *arg)
{
  (void)arg;
  while(1){
  automata_Master();
  }
}

/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
__NO_RETURN void app_main (void *arg) {
  (void)arg;

  netInitialize ();

  TID_MASTER = osThreadNew(Master,NULL,NULL);
  
  TID_UART_RX = osThreadNew(UART_RX,NULL,NULL);
  TID_UART_TX = osThreadNew(UART_TX,NULL,NULL);
 
  osThreadExit();
}
