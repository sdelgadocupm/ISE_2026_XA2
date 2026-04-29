#ifndef UART_MANAGER_H
#define UART_MANAGER_H

#include "cmsis_os2.h"
#include "Driver_USART.h"

void UART_Init(void);
void UART_SetThreadIds(osThreadId_t tx_id, osThreadId_t rx_id);
int UART_Send(uint8_t *data, uint32_t len);
int UART_Receive(uint8_t *data, uint32_t len);
ARM_DRIVER_USART* UART_GetDriver(void);

#endif