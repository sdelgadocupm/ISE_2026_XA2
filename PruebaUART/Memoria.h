#ifndef Memoria_H

#define Memoria_H
#include "main.h"
#include "cmsis_os2.h"
#include <stdbool.h>


#define EEPROM_I2C_ADDR 0x50 //Direccion de la memoria
#define EEPROM_PAGE_SIZE      64u
#define MAX_ENTRIES_DEFAULT   10u

#define TEMPERATURE_PAGE_ADDR      0x0000  // Página 0
#define HORA_PAGE_ADDR      0x0040  // Página 1
#define CA_PAGE_ADDR      0x0080  // Página 2
#define CONSUMO_PAGE_ADDR      0x00C0  // Página 3

extern bool completo;

void Init_I2C(void);


void guardar_hora(uint8_t hora,uint8_t minutos,uint8_t segundos );
void guardar_temperatura(uint16_t medida);
//void guardar_humedad(void);
void guardar_calidad_aire(uint16_t medida);
void guardar_consumo(uint16_t medida);


void leer_hora(void);
void leer_temperatura(void);
//void leer_humedad(void);
void leer_calidad_aire(void);
void leer_consumo(void);

uint32_t generar_cadena_medidas(char *out, uint32_t max_len);
#endif