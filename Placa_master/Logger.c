#include "cmsis_os2.h"
#include "Logger.h"
#include "Recepcion.h"
#include "Memoria.h"
//#include "RTC.h"
#include <stdio.h>
#include <string.h>
#include "ThCom.h"  // Para acceder a mid_ComQueue

osThreadId_t tid_Logger;

// Estructura interna para almacenar evento en progreso
static AlarmEvent_t current_alarm;
static bool alarma_en_progreso = false;
static uint16_t alarm_event_index = 0;


static int EEPROM_Write_Abs(uint16_t addr, uint8_t *data, uint32_t len) {
    uint8_t wr_buffer[2 + len];
    
    wr_buffer[0] = (addr >> 8) & 0xFF;  // MSB
    wr_buffer[1] = addr & 0xFF;         // LSB
    memcpy(&wr_buffer[2], data, len);
    
    EEPROM_Write_Event(wr_buffer, 2 + len);
    osDelay(5);
    return 0;
}

/**
 * @brief Lee datos de EEPROM en dirección absoluta (SIN usar lecturaDeValor)
 */

static int EEPROM_Read_Abs(uint16_t addr, uint8_t *data, uint32_t len) {
    uint8_t rd_addr[2];
    
    rd_addr[0] = (addr >> 8) & 0xFF;
    rd_addr[1] = addr & 0xFF;
    
    EEPROM_Read_Event(rd_addr, data, len);
    return 0;
}

static uint16_t get_alarm_event_count(void) {
    // Contar cuántos eventos válidos hay realmente
    uint8_t buffer[ALARM_EVENT_SIZE];
    uint16_t count = 0;
    
    for (uint16_t i = 0; i < MAX_ALARM_EVENTS; i++) {
        uint16_t addr = ALARM_EVENTS_PAGE_ADDR + (i * ALARM_EVENT_SIZE);
        EEPROM_Read_Abs(addr, buffer, ALARM_EVENT_SIZE);
        
        if (buffer[0] == 0xAA) {
            count++;
        } else if (buffer[0] == 0xFF) {
            break;  // Fin de eventos válidos
        }
    }
    
    return count;
}



static uint16_t recuperar_alarm_event_index(void) {
uint8_t buffer[ALARM_EVENT_SIZE];
    
    // Buscar el PRIMER slot vacío (0xFF en validación)
    for (uint16_t i = 0; i < MAX_ALARM_EVENTS; i++) {
        uint16_t addr = ALARM_EVENTS_PAGE_ADDR + (i * ALARM_EVENT_SIZE);
        EEPROM_Read_Abs(addr, buffer, ALARM_EVENT_SIZE);
        
        // Si encontramos slot vacío, ese es el siguiente índice
        if (buffer[0] != 0xAA) {
            return i;  // Retorna posición del siguiente a escribir
        }
    }
    
    // Si llegamos aquí, la EEPROM está llena
    return 0;  // Ring buffer: vuelve al inicio
}

// INICIALIZACIÓN DEL THREAD
int Init_Logger(void) {
	 
	  alarm_event_index = recuperar_alarm_event_index();
    memset(&current_alarm, 0, sizeof(AlarmEvent_t));
    alarma_en_progreso = false;
	
    tid_Logger = osThreadNew(Logger, NULL, NULL);
    if (tid_Logger == NULL) {
        return(-1);
    }
    
    return(0);
}

//Guarda un evento de alarma completo en EEPROM

static void guardar_evento_alarma(AlarmEvent_t *event) {
    if (alarm_event_index >= MAX_ALARM_EVENTS) {
        alarm_event_index = 0;
    }
    
    uint16_t addr = ALARM_EVENTS_PAGE_ADDR + (alarm_event_index * ALARM_EVENT_SIZE);
    uint8_t buffer[ALARM_EVENT_SIZE];
    
    // Empaquetar datos
		buffer[0] = 0xAA;  // VÁLIDO
    buffer[1] = (event->temperatura >> 8) & 0xFF;
    buffer[2] = event->temperatura & 0xFF;
    buffer[3] = (event->eco2 >> 8) & 0xFF;
    buffer[4] = event->eco2 & 0xFF;
    buffer[5] = (event->tvoc >> 8) & 0xFF;
    buffer[6] = event->tvoc & 0xFF;
    buffer[7] = event->hora_activacion;
    buffer[8] = event->minuto_activacion;
    buffer[9] = event->segundo_activacion;
    buffer[10] = event->hora_desac;
    buffer[11] = event->minuto_desac;
    buffer[12] = event->segundo_desac;
    buffer[13] = event->tipo_desactivacion;
    buffer[14] = event->rfid[0];
    buffer[15] = event->rfid[1];
    buffer[16] = event->rfid[2];
    buffer[17] = event->rfid[3];
    buffer[18] = 0xFF;
    
		EEPROM_Write_Abs(addr, buffer, ALARM_EVENT_SIZE);
    osDelay(10); 
    alarm_event_index++;
}

/**
 * @brief Lee un evento de la EEPROM
 */
 void leer_evento_alarma_master(uint16_t index, AlarmEvent_t *event) {
    if (index >= MAX_ALARM_EVENTS) {
				memset(event, 0, sizeof(AlarmEvent_t));
        return;
    }
		
    uint16_t addr = ALARM_EVENTS_PAGE_ADDR + (index * ALARM_EVENT_SIZE);
    uint8_t buffer[ALARM_EVENT_SIZE];

    
    // Usar función de Memoria.c
    EEPROM_Read_Abs(addr, buffer, ALARM_EVENT_SIZE);
    
		if (buffer[0] != 0xAA) {
        memset(event, 0, sizeof(AlarmEvent_t));
        return;
    }

		// Desempaquetar
		event->validacion = buffer[0];
    event->temperatura = ((uint16_t)buffer[1] << 8) | buffer[2];
    event->eco2 = ((uint16_t)buffer[3] << 8) | buffer[4];
    event->tvoc = ((uint16_t)buffer[5] << 8) | buffer[6];
    event->hora_activacion = buffer[7];
    event->minuto_activacion = buffer[8];
    event->segundo_activacion = buffer[9];
    event->hora_desac = buffer[10];
    event->minuto_desac = buffer[11];
    event->segundo_desac = buffer[12];
    event->tipo_desactivacion = buffer[13];
    event->rfid[0] = buffer[14];
    event->rfid[1] = buffer[15];
    event->rfid[2] = buffer[16];
    event->rfid[3] = buffer[17];
}
/**
 * @brief Thread principal
 */
void Logger(void *argument) {
    MSGQUEUE_ALARM_t msg;
    
    while (1) {
        osStatus_t status = osMessageQueueGet(mid_AlarmQueue, &msg, NULL, osWaitForever);
        
        if (status == osOK) {
						RTC_TimeTypeDef sTime;
            RTC_DateTypeDef sDate;
            HAL_RTC_GetTime(&RtcHandle, &sTime, RTC_FORMAT_BIN);
            HAL_RTC_GetDate(&RtcHandle, &sDate, RTC_FORMAT_BIN);
            if (msg.tipo_evento == 0) {
                // ACTIVACIÓN
                printf("[Logger] ??  ALARMA ACTIVADA\n");
                printf("  Temp: %u (0.1°C), eCO2: %u ppm, TVOC: %u ppb\n", 
                       msg.temperatura, msg.eco2, msg.tvoc);
                printf("  RFID: %02X%02X%02X%02X\n", 
                       msg.rfid[0], msg.rfid[1], msg.rfid[2], msg.rfid[3]);
                
                memset(current_alarm.rfid, 0, 4);
                
								current_alarm.hora_activacion = sTime.Hours;
                current_alarm.minuto_activacion = sTime.Minutes;
                current_alarm.segundo_activacion = sTime.Seconds;
							
                current_alarm.temperatura = msg.temperatura;
                current_alarm.eco2 = msg.eco2;
                current_alarm.tvoc = msg.tvoc;
                current_alarm.tipo_desactivacion = 0xFF;
                memcpy(current_alarm.rfid, msg.rfid, 4);
                
                alarma_en_progreso = true;
                
            } else if (msg.tipo_evento == 1 && alarma_en_progreso) {
                // DESACTIVACIÓN
								current_alarm.hora_desac = sTime.Hours;
                current_alarm.minuto_desac = sTime.Minutes;
                current_alarm.segundo_desac = sTime.Seconds;
							
                if(msg.rfid[0] == 0 && msg.rfid[1] == 0 && 
                   msg.rfid[2] == 0 && msg.rfid[3] == 0){
                    current_alarm.tipo_desactivacion = 1;
                }else{
                    memcpy(current_alarm.rfid, msg.rfid, 4);
                    current_alarm.tipo_desactivacion = 0;
                }
                
                guardar_evento_alarma(&current_alarm);
                alarma_en_progreso = false;
								
//								uint8_t buffer[ALARM_EVENT_SIZE];
//                uint16_t eventos_validos = 0;
//								for (uint16_t i = 0; i < MAX_ALARM_EVENTS; i++) {
//                    uint16_t addr = ALARM_EVENTS_PAGE_ADDR + (i * ALARM_EVENT_SIZE);
//                    EEPROM_Read_Abs(addr, buffer, ALARM_EVENT_SIZE);
//                    
//                    if (buffer[0] == 0xAA) {
//                        eventos_validos++;
//                        uint16_t temp = ((uint16_t)buffer[1] << 8) | buffer[2];
//                        uint16_t co2 = ((uint16_t)buffer[3] << 8) | buffer[4];
//                        uint16_t tvoc = ((uint16_t)buffer[5] << 8) | buffer[6];
//                        uint8_t h_act = buffer[7];
//                        uint8_t m_act = buffer[8];
//                        uint8_t s_act = buffer[9];
//                        uint8_t h_desac = buffer[10];
//                        uint8_t m_desac = buffer[11];
//                        uint8_t s_desac = buffer[12];
//                        uint8_t tipo_desac = buffer[13];
//                        uint8_t rfid0 = buffer[14];
//                        uint8_t rfid1 = buffer[15];
//                        uint8_t rfid2 = buffer[16];
//                        uint8_t rfid3 = buffer[17];
//                        
//                        const char *tipo = (tipo_desac == 0) ? "RFID" : (tipo_desac == 1) ? "REMOTO" : "PENDING";
//                        
//                        printf("[%2u] %02u:%02u:%02u -> %02u:%02u:%02u | T=%u CO2=%u TVOC=%u | %s %02X%02X%02X%02X\n",
//                               i, h_act, m_act, s_act, h_desac, m_desac, s_desac, 
//                               temp, co2, tvoc, tipo, rfid0, rfid1, rfid2, rfid3);
//                    } else if (buffer[0] == 0xFF) {
//                        break;
//                    }
//                }
//                
//                printf("Total: %u eventos\n", eventos_validos);
//                printf("==================================\n\n");
						}
        }
        
        osThreadYield();
    }
}

uint16_t Logger_GetWriteIndex(void) {
  return alarm_event_index;
}


int Logger_ReadEvent(uint16_t index, AlarmEvent_t *event) {
  
  leer_evento_alarma_master(index, event);

  // si no es válido, leer_evento_alarma_master hace memset 0, borramos 
  return (event->validacion == 0xAA) ? 1 : 0;
}
