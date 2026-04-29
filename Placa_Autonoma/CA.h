#ifndef AIRQUALITY_H
#define AIRQUALITY_H

#include "main.h"
#include "cmsis_os2.h"


// Funciones
void I2C_Signal_Event(uint32_t event);
void Th_AirQuality (void *argument);
int Init_ThAirQuality (void);
void CCS811_Init(void);

// Variables globales
typedef struct {
  uint16_t  eco2;
  uint16_t  tvoc;
} air_sample_t;

// Cola RTOS
extern osThreadId_t tid_AirQuality;
extern osMessageQueueId_t mid_airQueue;

// Dirección I2C
#define CCS811_ADDR 0x5A

#endif
