#ifndef PRINCIPAL_H
#define PRINCIPAL_H

#include "main.h"
#include "cmsis_os2.h"
#include "CA.h"
#include "RTC.h"
#include "RFID.h"
#include "LEDS.h"
#include "PWM.h"

#define TEMP_UMBRAL 50
#define FLAG_DISARM_RFID 0X01
typedef enum
{
BAJO_CONSUMO,PREACTIVACION,ALARMA
} state_t;

#endif
