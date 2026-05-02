#ifndef MASTER_H
#define MASTER_H

#include "main.h"
#include "Logger.h"
#include "ThCom.h"
#include "Recepcion.h"
#include "Memoria.h"
#include "UARTManager.h"
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef enum
{
  INIT,
  TRANSMISION,
  RECEPCION,
  WEB,
}Master_Estado_t;


void Inicializacion(void);
void automata (void);
void enviar_umbrales_a_slave(void);
static void send_uart_command(int id, int valor) ;
void Master_confirm(void);

static bool tx_pendiente = false;        
static bool rx_confirmado = false;       
static uint32_t timeout_rx = 0;          
#define RX_TIMEOUT_MS 5000               

 extern float th_temp ;
 extern int   th_co2 ;
 extern int   th_tvoc;

static uint32_t last_check = 0;









#endif