#ifndef RC522_H
#define RC522_H

#include "cmsis_os2.h"
#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "Driver_SPI.h"
#include <stdbool.h>
// ============ REGISTROS ============
#define RC522_COMMAND_REG           0x01
#define RC522_COM_IRQ_REG           0x04
#define RC522_DIV_IRQ_REG           0x05
#define RC522_ERROR_REG             0x06
#define RC522_STATUS1_REG           0x07
#define RC522_STATUS2_REG           0x08
#define RC522_FIFO_DATA_REG         0x09
#define RC522_FIFO_LEVEL_REG        0x0A
#define RC522_CONTROL_REG           0x0C
#define RC522_BIT_FRAMING_REG       0x0D
#define RC522_TMODE_REG             0x2A
#define RC522_TPRESCALER_REG        0x2B
#define RC522_TRELOAD_REG_L         0x2C
#define RC522_TRELOAD_REG_H         0x2D
#define RC522_MODE_REG              0x11
#define RC522_TX_MODE_REG           0x12
#define RC522_RX_MODE_REG           0x13
#define RC522_TX_CONTROL_REG        0x14
#define RC522_TX_ASK_REG            0x15
#define RC522_RX_SEL_REG            0x16
#define RC522_RX_THRESHOLD_REG      0x18
#define RC522_COM_ENABLE_REG        0x26
#define RC522_DIV_ENABLE_REG        0x27
#define RC522_CRC_RESULT_REG_L      0x21
#define RC522_CRC_RESULT_REG_M      0x22

// ============ COMANDOS ============
#define RC522_CMD_IDLE              0x00
#define RC522_CMD_TRANSCEIVE        0x0C
#define RC522_CMD_AUTHENT           0x0E
#define RC522_CMD_SOFT_RESET        0x0F
#define RC522_CMD_CALCCRC           0x03

// ============ PROTOCOLOS ============
#define PICC_ANTICOLL               0x93
#define PICC_READ                   0x30
#define PICC_WRITE                  0xA0
#define PICC_HALT                   0x50
#define PICC_SELECT_TAG             0x93

// ============ CÓDIGOS DE ESTADO ============
#define RC522_OK                    0x00
#define RC522_NOTAG                 0x01
#define RC522_ERROR                 0x02

// ============ STRUCT ============
typedef struct {
    uint8_t uid[4];
    uint8_t uidLength;
} RC522_UID;


// ============ DECLARACIÓN DE FUNCIONES ============

// Funciones de SPI
void RC522_WriteRegister(uint8_t reg, uint8_t value);
uint8_t RC522_ReadRegister(uint8_t reg);

// Inicialización
void RC522_Init(void);
void RC522_Reset(void);
void RC522_AntennaOn(void);
void RC522_AntennaOff(void);

// Detección de la tarjeta
uint8_t RC522_Request(uint8_t reqMode, uint8_t *TagType);
uint8_t RC522_Anticoll(uint8_t *serNum);
uint8_t RC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, 
                     uint8_t *backData, uint16_t *backLen);

// Función de lectura principal
RC522_UID RC522_ReadUID(void);
bool RFID_TryDisarm(void);

// Funciones de Control
void RC522_StopCommand(void);
void RC522_ClearFIFO(void);

// Funciones Auxiliares
void RC522_SetBitMask(uint8_t reg, uint8_t mask);
void RC522_ClearBitMask(uint8_t reg, uint8_t mask);
void RC522_CalculateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData);
void RC522_BSP_GPIO_Init(void);
void RC522_SPI_Init(void);
int Init_ThRFID(void);
static bool UID_Equals(const uint8_t uid[4], const uint8_t allowed[4]);
#endif // RC522_H
