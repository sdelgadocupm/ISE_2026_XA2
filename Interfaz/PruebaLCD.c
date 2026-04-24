#include "LCD.h"
#include "Arial12x12.h"


//ADAPTAR A TU PRACTICA

extern GPIO_InitTypeDef GPIO_InitStruct;

unsigned char buffer [512];


extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;//variable del SPI

TIM_HandleTypeDef htim7; 

uint16_t positionL1=0;
uint16_t positionL2=0;

//////////////////////////////////////////////RESET Y DELAY//////////////////////////////////////////////////
/* Funci?n de retardo*/
static void delay(uint32_t n_microsegundos){
  
  htim7.Instance=TIM7;
  htim7.Init.Prescaler=83;
  htim7.Init.Period = n_microsegundos-1;
  
  __HAL_RCC_TIM7_CLK_ENABLE();
  
  HAL_TIM_Base_Init(&htim7);
  HAL_TIM_Base_Start(&htim7);

	while( TIM7->CNT < n_microsegundos-1){}
	/*paro el reset*/	
	HAL_TIM_Base_Stop(&htim7);
	/*inicializo el timer de nuevo*/
	HAL_TIM_Base_DeInit(&htim7); // configura el timer
}


void LCD_reset(void){
  SPIdrv->Initialize(NULL);
  SPIdrv->PowerControl(ARM_POWER_FULL);
  SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB |
  ARM_SPI_DATA_BITS(8), 1000000);//ponle 1000000 para la visualizaci?n con el analizador
  SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);

  GPIO_LCD_Init();
  
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6, GPIO_PIN_RESET);
  delay(2);//Mejor usar 2ms a apurar 1ms
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6, GPIO_PIN_SET);
  delay(1000);
}

void GPIO_LCD_Init(void){
  GPIO_InitTypeDef GPIO_InitStruct;
  
    //Configuraci?n del reset
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
  
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6, GPIO_PIN_SET);  //Nivel alto
  
  //Configuraci?n del A0
  __HAL_RCC_GPIOF_CLK_ENABLE();
  
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOF,&GPIO_InitStruct);
  
  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_13, GPIO_PIN_SET);
  
  //Configuraci?n del CS
    __HAL_RCC_GPIOD_CLK_ENABLE();
  
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOD,&GPIO_InitStruct);
  
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_SET);
  
}


void LCD_wr_data(unsigned char data){
  
  int32_t stat_send;
  ARM_SPI_STATUS stat_status;
  
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_13, GPIO_PIN_SET);
  
  stat_send = SPIdrv->Send(&data,sizeof(data));
  
  do{
  stat_status = SPIdrv->GetStatus();
  }while(stat_status.busy);
  
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_SET);
}

void LCD_wr_cmd(unsigned char data){
  
  int32_t stat_send;
  ARM_SPI_STATUS stat_status;
  
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_13, GPIO_PIN_RESET);
  
  stat_send = SPIdrv->Send(&data,sizeof(data));
  
  do{
  stat_status = SPIdrv->GetStatus();
  }while(stat_status.busy);
  
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_SET);
}

void LCD_init(void){
  LCD_wr_cmd(0xAE);
  LCD_wr_cmd(0xA2);
  LCD_wr_cmd(0xA0);
  LCD_wr_cmd(0xC8);
  LCD_wr_cmd(0x22);
  LCD_wr_cmd(0x2F);
  LCD_wr_cmd(0x40);
  LCD_wr_cmd(0xAF);
  LCD_wr_cmd(0x81);
  LCD_wr_cmd(0x10);
  LCD_wr_cmd(0xA4);
  LCD_wr_cmd(0xA6);

}

void LCD_update(void)
{
 int i;
 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la direcci?n a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la direcci?n a 0
 LCD_wr_cmd(0xB0); // P?gina 0

 for(i=0;i<128;i++){
 LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la direcci?n a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la direcci?n a 0
 LCD_wr_cmd(0xB1); // P?gina 1

 for(i=128;i<256;i++){
 LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB2); //P?gina 2
 for(i=256;i<384;i++){
 LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB3); // Pagina 3


 for(i=384;i<512;i++){
 LCD_wr_data(buffer[i]);
 }
}

void symbolToLocalBuffer_L1(uint8_t symbol){
  uint8_t i, value1,value2;
  uint16_t offset=0;
  offset = 25*(symbol - ' ');
  
  for(i=0; i<12; i++){
    value1 = Arial12x12[offset+i*2+1];
    value2 = Arial12x12[offset+i*2+2];
    
    buffer[i+positionL1]=value1;
    buffer[i+128+positionL1]=value2;
  }
  positionL1= positionL1 + Arial12x12[offset];
}

void symbolToLocalBuffer_L2(uint8_t symbol){
  uint8_t i, value1,value2;
  uint16_t offset=0;
  offset = 25*(symbol - ' ');
  
  for(i=0; i<12; i++){
    value1 = Arial12x12[offset+i*2+1];
    value2 = Arial12x12[offset+i*2+2];
    
    buffer[256+i+positionL2]=value1;
    buffer[256+i+128+positionL2]=value2;
  }
  positionL2= positionL2 + Arial12x12[offset];
}

void symbolToLocalBuffer(uint8_t line, uint8_t symbol){
 if (line==1){
		symbolToLocalBuffer_L1(symbol);
	}
	if(line==2){
		symbolToLocalBuffer_L2(symbol);
	}
}

void LCD_Clean(void){
	memset(buffer, 0, 512U);
	LCD_update();
}

void Pintar(MSGQUEUE_OBJ_t_LCD msg){
	int fin = 0;
	positionL1 = 0;
	positionL2 = 0;
	if(msg.Idx == 0){
		for(int i = 0; i<128; i++){
			if(msg.Buf[i]!=0x00 && fin!= 1){
				symbolToLocalBuffer_L1(msg.Buf[i]);
				if(msg.Buf[i+1]==0x00){
					fin = 1;
				}
			}
		}
		LCD_update();
	}else{
		for(int i = 0; i<128; i++){
			if(msg.Buf[i]!=0x00 && fin!= 1){
				symbolToLocalBuffer_L2(msg.Buf[i]);
				if(msg.Buf[i+1]==0x00){
					fin = 1;
				}
			}
		}
		LCD_update();
	}
}



