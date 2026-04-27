
#include "Driver_I2C.h"
#include "Memoria.h"
#include <stdio.h>
/*----------------------------------------------------------------------------
 *      Memoria EEPROM
 *---------------------------------------------------------------------------*/
 

/* I2C driver instance */
extern ARM_DRIVER_I2C            Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;
 
 
 static volatile uint32_t I2C_Event;


uint8_t wr_a[2 + EEPROM_PAGE_SIZE];
uint8_t rd_a[2];

//Index de las medidas para el registro
uint8_t temp_index = 0;
uint8_t hora_index = 0;
uint8_t ca_index = 0;
uint8_t consumo_index = 0;

 bool completo = false;

//Index de las medidas para la lectura
uint8_t temp_index_r = 0;
uint8_t hora_index_r = 0;
uint8_t ca_index_r = 0;
uint8_t consumo_index_r = 0;
/* I2C Signal Event function callback */
void I2C_SignalEvent (uint32_t event)
{
  /* Save received events */
  I2C_Event |= event; 
}

void Init_I2C (void)
{
  //Lo inicalizamos con el callback y no con polling para poder realizar operacioen sen segundo plano
  I2Cdrv->Initialize(I2C_SignalEvent);
  
  I2Cdrv->PowerControl(ARM_POWER_FULL);
  I2Cdrv->Control  (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
  I2Cdrv->Control      (ARM_I2C_BUS_CLEAR, 0);
  
}

uint32_t EEPROM_Write_Event (uint8_t *WriteAddr, uint32_t WriteLen)
{
  /* Clear event flags before new transfer */
  I2C_Event = 0U;
  
  I2Cdrv->MasterTransmit (EEPROM_I2C_ADDR, WriteAddr, WriteLen, false);
 
    /* Wait until transfer completed */
  while ((I2C_Event & ARM_I2C_EVENT_TRANSFER_DONE) == 0U);
  /* Check if all data transferred */
  if ((I2C_Event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U) return -1;
 
  return 0;
  
}

uint32_t EEPROM_Read_Event (uint8_t *addrRead, uint8_t *buf, uint32_t lenRead)
{
 /* Clear event flags before new transfer */
  I2C_Event = 0U;
 
  I2Cdrv->MasterTransmit (EEPROM_I2C_ADDR, addrRead, 2, true);
 
  /* Wait until transfer completed */
  while ((I2C_Event & ARM_I2C_EVENT_TRANSFER_DONE) == 0U);
  /* Check if all data transferred */
  if ((I2C_Event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U) return -1;
 
  /* Clear event flags before new transfer */
  I2C_Event = 0U;
 
  I2Cdrv->MasterReceive (EEPROM_I2C_ADDR, buf, lenRead, false);
 
  /* Wait until transfer completed */
  while ((I2C_Event & ARM_I2C_EVENT_TRANSFER_DONE) == 0U);
  /* Check if all data transferred */
  if ((I2C_Event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U) return -1;
 
  return 0;
}



void registroDevalor(uint16_t direccion, uint8_t *a, uint8_t len, uint8_t *escritura_index)
{
  //El valor maximo por trama es de 0x0A por tanto comprobamos si entra o si no
  if( *escritura_index >= MAX_ENTRIES_DEFAULT)
  {
    *escritura_index = 0x00;
    completo = true;
  }
  
  // Dirección de esta entrada dond emultiplica el tamaño con el indice de la estructura y ademas el incio de direccion de la pagina 
  uint16_t addr = direccion + (*escritura_index * len);
  
   wr_a[0] = (uint8_t)(addr >> 8); //MSB
   wr_a[1] = (uint8_t)(addr & 0xFF); //LSB
  
  for(uint8_t i =0; i < len; i++ )
  {
    wr_a[2 + i] = a[i];
  }
  
  EEPROM_Write_Event(wr_a, 2u + len);
  
   osDelay(5); //especificaciones de 5ms par cada operacion segun el datasheet de la memoria
  
  
    // Siguiente entrada
    (*escritura_index)++;
}


/***FUNCION PARA LEER LOS VALORES(2 BITS)**/
void lecturaDeValor(uint16_t direccion, uint8_t* buf_rd, uint8_t len, uint8_t *lectura_index)
{
  if(*lectura_index >= MAX_ENTRIES_DEFAULT)
  {
    *lectura_index = 0x00;
  }
  
  uint16_t addr = direccion + (*lectura_index * len);
  
   rd_a[0] = (uint8_t)(addr >> 8); //MSB
   rd_a[1] = (uint8_t)(addr & 0xFF); //LSB
  
  EEPROM_Read_Event(rd_a,buf_rd,len);
  
  (*lectura_index)++;
}

/***FUNCION PARA GUARDAR LA TEMPERATURA (2 BITS)**/
void guardar_temperatura(uint16_t medida)
{
  uint8_t datos[2];
    datos[0] = (medida >> 8) & 0xFF;
    datos[1] = medida & 0xFF;
  
  registroDevalor(TEMPERATURE_PAGE_ADDR,datos,2,&temp_index);
}

/***FUNCION PARA GUARDAR LA HORA DEL RTC(3 BITS)**/
void guardar_hora(uint8_t hora,uint8_t minutos,uint8_t segundos )
{
  uint8_t buf[3];
    buf[0] = hora;
  buf[1] = minutos;
  buf[2] = segundos;

  registroDevalor(HORA_PAGE_ADDR,buf,3,&hora_index);
  
}


/***FUNCION PARA GUARDAR LA CALIDAD DEL AIRE (2 BITS)**/
void guardar_calidad_aire(uint16_t medida )
{
  uint8_t data = medida;
  
  registroDevalor(CA_PAGE_ADDR,&data,1,&ca_index);
}

void guardar_consumo(uint16_t medida)
{
  uint8_t buf[3];
    buf[0] = (medida >> 8) & 0xFF;
    buf[1] = medida & 0xFF;
  registroDevalor(CONSUMO_PAGE_ADDR,buf,2,&consumo_index);
}

/************************CREAMOS FUNCIONES PARA LEER LOS VALORES****************/

void leer_medidas(uint16_t *temperatura, uint8_t *calidad_aire, uint16_t *consumo, uint8_t *horas, uint8_t *minutos,uint8_t *segundos)
{
  uint8_t buf[3];
  
  //Leer temperatura
  lecturaDeValor(TEMPERATURE_PAGE_ADDR,buf,2,&temp_index_r);
  *temperatura = buf[0] << 8 | buf[1];
  
  //leer hora
   lecturaDeValor(HORA_PAGE_ADDR,buf,3,&hora_index_r);
  *horas = buf[0];
  *minutos = buf[1];
  *segundos = buf[2];
  
  //leer calidad del aire 
  lecturaDeValor(CA_PAGE_ADDR,buf,1,&ca_index_r);
  *calidad_aire = buf[0];
  
  //leer consumo 
  lecturaDeValor(CONSUMO_PAGE_ADDR,buf,2,&consumo_index_r);
  *consumo = buf[0] << 8 | buf[1];
}

uint32_t generar_cadena_medidas(char *out, uint32_t max_len)
{
  uint16_t t,c;
  uint8_t ca, h, m,s;
  
  if(!out || max_len == 0)
  {
    return 0;
  }
    
  leer_medidas(&t,&ca, &c,&h,&m,&s);
  int n = snprintf(out, max_len, "%u;%u;%u;%02u:%02u:%02u", t, ca, c, h, m, s);
  
  if (n <0 ) return 0;
  if ((uint32_t)n >= max_len) { 
      out[max_len-1] = 0; 
  return max_len-1; 
  }
  return (uint32_t)n;
  
}








