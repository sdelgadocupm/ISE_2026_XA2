#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#ifndef __ADC_H
#define __ADC_H
	void ADC1_pins_F429ZI_config(void);
	int ADC_Init_Single_Conversion(ADC_HandleTypeDef *, ADC_TypeDef  *);
	uint16_t ADC_getTemp(ADC_HandleTypeDef *hadc);
	uint16_t ADC_getConsumo(ADC_HandleTypeDef *hadc);
  
  void Thread_ADC (void *argument);
  
  uint32_t ADCs_getVoltage(ADC_HandleTypeDef * , uint32_t );
  
  extern osMessageQueueId_t mid_MsgQueueADC;
  
  typedef struct
  {
    uint16_t temp_ADC;
    uint16_t cons_ADC;
  }MSG_ADC_VALORES_t;
  
  
  void media(uint16_t tem, uint16_t cons,int *num );
  
  
  
#endif
