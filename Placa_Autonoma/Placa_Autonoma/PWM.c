#include "PWM.h"
#include "main.h"
 
osThreadId_t tid_PWM;                        // thread id

static TIM_HandleTypeDef htim1;
static TIM_OC_InitTypeDef TIMConfig;

/* --- Timer de alarma --- */
static osTimerId_t timer_pwm_alarma;
static volatile uint8_t alarma_activa = 0;
static volatile uint8_t pwm_on = 0; // 0=apagado, 1=encendido


/* Callback del timer*/
static void cb_timer_pwm_alarma(void *argument) {
  (void)argument;

  if (!alarma_activa) {
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    pwm_on = 0;
    return;
  }

  if (pwm_on == 0) {
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);  // suena
    pwm_on = 1;
    osTimerStart(timer_pwm_alarma, 120);       // ON 120 ms
  } else {
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);   // silencio
    pwm_on = 0;
    osTimerStart(timer_pwm_alarma, 120);       // OFF 120 ms
  }
}
 

int Init_Thread_PWM (void) {
  tid_PWM = osThreadNew(Thread_PWM, NULL, NULL);
  if (tid_PWM == NULL) {
    return(-1);
  }
  return(0);
}
 
void Thread_PWM (void *argument) {
  (void)argument;

  inicializarTimer();
  inicializarPWM();

  
  timer_pwm_alarma = osTimerNew(cb_timer_pwm_alarma, osTimerOnce, NULL, NULL);

  while (1) {
    uint32_t flags = osThreadFlagsWait(ALARMA_ON | ALARMA_OFF,
                                       osFlagsWaitAny,
                                       osWaitForever);

    if (flags & ALARMA_ON) {
      alarma_activa = 1;
      pwm_on = 0;
      osTimerStart(timer_pwm_alarma, 1);   
    }

    if (flags & ALARMA_OFF) {
      alarma_activa = 0;
      osTimerStop(timer_pwm_alarma);
      HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
      pwm_on = 0;
    }
  }
}

void inicializarPWM(void){
  GPIO_InitTypeDef GPIO_InitStruct;
	
  __HAL_RCC_GPIOE_CLK_ENABLE();
	
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1; 
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOE,&GPIO_InitStruct);	
}

void inicializarTimer(void){
  __HAL_RCC_TIM1_CLK_ENABLE();
	
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 41;                 
  htim1.Init.Period = 4199;                  
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;

  TIMConfig.OCMode = TIM_OCMODE_PWM1;
  TIMConfig.Pulse = 2099;                    
  TIMConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
  TIMConfig.OCFastMode = TIM_OCFAST_DISABLE;
	
  HAL_TIM_PWM_Init(&htim1);
  HAL_TIM_PWM_ConfigChannel(&htim1, &TIMConfig, TIM_CHANNEL_1);
}