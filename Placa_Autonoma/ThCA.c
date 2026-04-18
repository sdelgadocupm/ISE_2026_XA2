#include "Driver_I2C.h"
#include "CA.h"
#include <stdio.h>
/*----------------------------------------------------------------------------
 *      Thread Air Quality
 *---------------------------------------------------------------------------*/

osThreadId_t tid_AirQuality;

extern ARM_DRIVER_I2C  Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;
osMessageQueueId_t mid_airQueue;


int Init_ThAirQuality (void) {
  mid_airQueue = osMessageQueueNew(8, sizeof(air_sample_t), NULL);
	
  I2Cdrv->Initialize(I2C_Signal_Event);
  I2Cdrv->PowerControl (ARM_POWER_FULL);
  I2Cdrv->Control (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
  I2Cdrv->Control (ARM_I2C_BUS_CLEAR, 0);

  tid_AirQuality = osThreadNew(Th_AirQuality, NULL, NULL);
  if (tid_AirQuality == NULL) {
    return -1;
  }

  return 0;
}

void I2C_Signal_Event(uint32_t event)
{
  osThreadFlagsSet(tid_AirQuality, 0x01);
}

void CCS811_Init(void)
{
  uint8_t cmd;

  // APP_START
  cmd = 0xF4;
  I2Cdrv->MasterTransmit(CCS811_ADDR, &cmd, 1, false);
  osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);

  // MEAS_MODE: 1 lectura por segundo
  uint8_t meas_mode[2] = {0x01, 0x10};
  I2Cdrv->MasterTransmit(CCS811_ADDR, meas_mode, 2, false);
  osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
}

void Th_AirQuality (void *argument) {
  uint8_t status;
  uint8_t reg;
  uint8_t buf[8];

  // Inicializar sensor
  osDelay(100);   // pequeño delay tras encendido
  CCS811_Init();

  while (1) {
		
		//Espera orden del principa
		
		// 1. Leer STATUS
    reg = 0x00;
    I2Cdrv->MasterTransmit(CCS811_ADDR, &reg, 1, true);
    osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);

    I2Cdrv->MasterReceive(CCS811_ADDR, &status, 1, false);
    osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);

    // ?? 2. Comprobar DATA_READY (bit 3)
    if (!(status & (1 << 3))) {
        osDelay(100);
        continue;  // IMPORTANTE: saltar lectura
    }
		
    // Seleccionar registro de datos
    reg = 0x02; // ALG_RESULT_DATA
    I2Cdrv->MasterTransmit(CCS811_ADDR, &reg, 1, true);
    osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);

    // Leer 8 bytes
		
    I2Cdrv->MasterReceive(CCS811_ADDR, buf, 8, false);
    osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
		
		osThreadFlagsWait(0x02, osFlagsWaitAny, osWaitForever);
    // Conversión de datos
		air_sample_t s;
    s.eco2 = (buf[0] << 8) | buf[1];
    s.tvoc = (buf[2] << 8) | buf[3];
  
    osMessageQueuePut(mid_airQueue, &s, 0, 0);
  }
}