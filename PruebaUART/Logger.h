#ifndef LOGGER_H
#define LOGGER_H

#include "cmsis_os2.h"
#include <stdint.h>
#include <stdbool.h>

// Estructura para almacenar un evento de alarma
typedef struct {
	  uint8_t horas;
    uint8_t minutos;
    uint8_t segundos;
    uint16_t temperatura;
    uint16_t eco2;
    uint16_t tvoc;
    uint8_t horas_desac;
    uint8_t minutos_desac;
    uint8_t segundos_desac;
    uint8_t tipo_desactivacion;    
    uint8_t rfid[4];	// 0: RFID, 1: Remoto, 0xFF: Pendiente
} AlarmEvent_t;

// IDs de eventos
#define ALARM_ACTIVATION   0
#define ALARM_DEACTIVATION 1

extern osThreadId_t tid_Logger;

void Logger(void *argument);
int Init_Logger(void);
uint32_t generar_cadena(char *out, uint32_t max_len);

#endif