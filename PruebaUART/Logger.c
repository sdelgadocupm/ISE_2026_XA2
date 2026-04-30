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

// Direcciones EEPROM
#define ALARM_EVENTS_PAGE_ADDR    0x2000
#define ALARM_EVENT_SIZE         16
#define MAX_ALARM_EVENTS         20

static uint8_t alarm_event_index = 0;

// INICIALIZACIÓN DEL THREAD
int Init_Logger(void) {
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
    buffer[0] = (event->temperatura >> 8) & 0xFF;
    buffer[1] = event->temperatura & 0xFF;
    buffer[2] = (event->eco2 >> 8) & 0xFF;
    buffer[3] = event->eco2 & 0xFF;
    buffer[4] = (event->tvoc >> 8) & 0xFF;
    buffer[5] = event->tvoc & 0xFF;
    buffer[6] = event->horas;
    buffer[7] = event->minutos;
    buffer[8] = event->segundos;
    buffer[9] = event->tipo_desactivacion;
    buffer[10] = event->rfid[0];
    buffer[11] = event->rfid[1];
    buffer[12] = event->rfid[2];
    buffer[13] = event->rfid[3];
    buffer[14] = 0xFF;
    buffer[15] = 0xFF;
    
    registroDevalor(ALARM_EVENTS_PAGE_ADDR, buffer, ALARM_EVENT_SIZE, &alarm_event_index);
    
     guardar_temperatura(event->temperatura);
    guardar_hora(event->horas, event->minutos, event->segundos);
    guardar_calidad_aire(event->eco2);
    guardar_consumo(event->tvoc);
}

/**
 * @brief Lee un evento de la EEPROM
 */
static void leer_evento_alarma_master(uint8_t index, AlarmEvent_t *event) {
    if (index >= alarm_event_index) {
        return;
    }
    
    uint8_t buffer[ALARM_EVENT_SIZE];
    uint8_t temp_index = index;
    
    // Usar función de Memoria.c
    lecturaDeValor(ALARM_EVENTS_PAGE_ADDR, buffer, ALARM_EVENT_SIZE, &temp_index);
    
    // Desempaquetar
    event->temperatura = (buffer[0] << 8) | buffer[1];
    event->eco2 = (buffer[2] << 8) | buffer[3];
    event->tvoc = (buffer[4] << 8) | buffer[5];
    event->horas = buffer[6];
    event->minutos = buffer[7];
    event->segundos = buffer[8];
    event->tipo_desactivacion = buffer[9];
    event->rfid[0] = buffer[10];
    event->rfid[1] = buffer[11];
    event->rfid[2] = buffer[12];
    event->rfid[3] = buffer[13];
}

//Genera cadena historial

uint32_t generar_cadena(char *out, uint32_t max_len) {
    if (!out || max_len == 0) {
        return 0;
    }
    
    uint32_t total_len = 0;
    AlarmEvent_t event;
    
    for (uint8_t i = 0; i < alarm_event_index; i++) {
        leer_evento_alarma_master(i, &event);
        
        const char *tipo = (event.tipo_desactivacion == 0) ? "RFID" : 
                          (event.tipo_desactivacion == 1) ? "REMOTO" : "PENDIENTE";
        char rfid_str[12] = {0};
        
        if (event.tipo_desactivacion == 0) {
            snprintf(rfid_str, sizeof(rfid_str), "%02X%02X%02X%02X",
         event.rfid[0], event.rfid[1], event.rfid[2], event.rfid[3]);
        } else if (event.tipo_desactivacion == 1) {
            snprintf(rfid_str, sizeof(rfid_str), "N/A");
        } else {
            snprintf(rfid_str, sizeof(rfid_str), "PENDIENTE");
        }
        
        int n = snprintf(out + total_len, max_len - total_len,
                        "%u;%u;%u;%02u:%02u:%02u;%s;%s\n",
                        event.temperatura, event.eco2, event.tvoc,
                        event.horas, event.minutos, event.segundos,
                        tipo, rfid_str);
        
        if (n < 0 || (total_len + n) >= max_len) {
            break;
        }
        
        total_len += n;
    }
    
    return total_len;
}


/**
 * @brief Thread principal
 */
void Logger(void *argument) {
    MSGQUEUE_ALARM_t msg;
    //RTC_TimeTypeDef rtc_time;

    
    while (1) {
        osStatus_t status = osMessageQueueGet(mid_AlarmQueue, &msg, NULL, osWaitForever);
        
        if (status == osOK) {
            if (msg.tipo_evento == ALARM_ACTIVATION) {
                // ACTIVACIÓN
                memset(current_alarm.rfid, 0, 4);
							
                //RTC_GetTime(&rtc_time);
                
                current_alarm.temperatura = msg.temperatura;
                current_alarm.eco2 = msg.eco2;
                current_alarm.tvoc = msg.tvoc;
                //current_alarm.horas = rtc_time.Hours;
                //current_alarm.minutos = rtc_time.Minutes;
                //current_alarm.segundos = rtc_time.Seconds;
                current_alarm.tipo_desactivacion = 0xFF;
                memcpy(current_alarm.rfid, msg.rfid, 4);
                
                alarma_en_progreso = true;
                
            } else if (msg.tipo_evento == ALARM_DEACTIVATION && alarma_en_progreso) {
                // DESACTIVACIÓN
								if(msg.rfid[0] == 0 && msg.rfid[1] == 0 && 
                   msg.rfid[2] == 0 && msg.rfid[3] == 0){
									current_alarm.tipo_desactivacion = 1;
								}else{
									memcpy(current_alarm.rfid, msg.rfid, 4);
									current_alarm.tipo_desactivacion = 0;
								}
                
                guardar_evento_alarma(&current_alarm);
                alarma_en_progreso = false;
            }
        }
        
        osThreadYield();
    }
}
//Para poder comprobar
void enviar_historial_eventos(void) {
    char buffer[512] = {0};
    uint32_t len = generar_cadena(buffer, sizeof(buffer));
    
    // Prepara el mensaje para enviar por UART
    MSGQUEUE_OBJ_COM_t msgCom;
    
    snprintf(msgCom.Mensaje, sizeof(msgCom.Mensaje), 
             "\n=== HISTORIAL ===\nEventos: %u\n%s===END===\n", 
             alarm_event_index, buffer);
    
    msgCom.TamMens = strlen(msgCom.Mensaje);
    
    // Envía a la cola de transmisión
    osMessageQueuePut(mid_ComQueue, &msgCom, 0U, 0U);
}