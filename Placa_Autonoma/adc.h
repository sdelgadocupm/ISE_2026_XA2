#include "stm32f4xx_hal.h"
#ifndef __ADC_H
	void ADC1_pins_F429ZI_config(void);
	int ADC_Init_Single_Conversion(ADC_HandleTypeDef *, ADC_TypeDef  *);
	uint16_t ADC_getTemp(ADC_HandleTypeDef *hadc);
	uint16_t ADC_getConsumo(ADC_HandleTypeDef *hadc);
#endif
