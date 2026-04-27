#ifndef PRINCIPAL_H
#define PRINCIPAL_H

#include "main.h"
#include "cmsis_os2.h"
#include "CA.h"
#include "RTC.h"
#include "RFID.h"
#include "LEDS.h"
#include "PWM.h"
#include "ThCom.h"
#include "adc.h"
#include "Recepcion.h"

//Flag de desactivación
#define FLAG_DISARM_RFID 0X01

//DEFINICIÓN DE ID DE MENSAJES
#define TEMPERATURA 1
#define CO2 2
#define TVOC 3
#define MODO 4
#define ESTADO 5
#define RFID 6
#define CONSUMO 7


//ESTADOS
#define STOP 0
#define RUN 1

//MODOS
#define BAJO_CONS 0
#define PREACT 1
#define ALAR 2


typedef enum{
BAJO_CONSUMO,PREACTIVACION,ALARMA
} state_t;

#endif
