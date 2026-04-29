#ifndef RECEPCION_H

#define RECEPCION_H
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include "Driver_USART.h"

typedef struct {
    uint16_t temperatura;
    uint16_t eco2;
    uint16_t tvoc;
    uint8_t tipo_evento;           // 0: Activacion, 1: Desactivacion
    uint8_t rfid[4];               // UID
} MSGQUEUE_ALARM_t;

extern osMessageQueueId_t mid_AlarmQueue;

void ThRecep(void *argument);                   // thread functiond);
int Init_ThRecep (void);
void ProcesarTrama(char *buffer);
#endif