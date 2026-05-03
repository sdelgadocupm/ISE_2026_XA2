#ifndef LOGGER_H
#define LOGGER_H

#include "cmsis_os2.h"
#include "RTC.h"
#include <stdint.h>
#include <stdbool.h>

// Estructura para almacenar un evento de alarma
typedef struct {
		uint8_t validacion;
    uint16_t temperatura;           
    uint16_t eco2;                  
    uint16_t tvoc;                  
    uint8_t hora_activacion;        
    uint8_t minuto_activacion;     
    uint8_t segundo_activacion;    
	
    uint8_t hora_desac;
    uint8_t minuto_desac;
    uint8_t segundo_desac;
    uint8_t tipo_desactivacion;    
    uint8_t rfid[4];	// 0: RFID, 1: Remoto, 0xFF: Pendiente
	
		uint8_t reserved;
} AlarmEvent_t;

// IDs de eventos

extern osThreadId_t tid_Logger;

void Logger(void *argument);
int Init_Logger(void);
uint32_t generar_cadena(char *out, uint32_t max_len);
void enviar_historial_eventos(void);

void leer_evento_alarma_master(uint16_t index, AlarmEvent_t *event);
//Devuelve 1 si el evento ya existe y es valido si no devuleve un 0 
int Logger_ReadEvent(uint16_t index, AlarmEvent_t *event);

//Devuelve el indice para poder escribir
uint16_t Logger_GetWriteIndex(void);

#endif