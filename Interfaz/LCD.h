#ifndef __LCD_H
#define __LCD_H

/*Includes*/
#include "Driver_SPI.h"  
#include "main.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os2.h"
#include "stdint.h"

/*Funciones*/
void GPIO_LCD_Init(void);
void LCD_reset(void);
void LCD_init(void);
static void delay(uint32_t n_microsegundos);
void LCD_wr_data(unsigned char data);
void LCD_wr_cmd(unsigned char data);
void LCD_update(void);
void LCD_Clean(void);
void symbolToLocalBuffer_L1(uint8_t symbol);
void symbolToLocalBuffer_L2(uint8_t symbol);
void symbolToLocalBuffer(uint8_t line, uint8_t symbol);
void Pintar_Arriba(void);
void Pintar_Abajo(void);
void Pintar_Centro(void);
void Pintar_Derecha(void);
void Pintar_Izquierda(void);


void Thread2 (void *argument);                   // thread function
 extern int Init_Thread2 (void);

typedef struct{
	char Buf[128]; 			//Buffer que almacena los datos del mensaje
	uint8_t Idx;					//Índice
}MSGQUEUE_OBJ_t_LCD;
void Pintar(MSGQUEUE_OBJ_t_LCD msg);
extern osMessageQueueId_t mid_MsgQueue_LCD; 
#endif
