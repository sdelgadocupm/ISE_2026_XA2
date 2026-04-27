#include "RFID.h"
#include <string.h>
#include "Principal.h" 

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* RC522_SPIdrv = &Driver_SPI1;

uint8_t ultimo_uid[4];

#define RC522_RST_PORT   GPIOA
#define RC522_RST_PIN    GPIO_PIN_6

#define RC522_CS_PORT    GPIOD
#define RC522_CS_PIN     GPIO_PIN_14

// ============ INICIALIZACIÓN GPIO ============
static void RC522_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // Pin Reset 6A
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = RC522_RST_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(RC522_RST_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(RC522_RST_PORT, RC522_RST_PIN, GPIO_PIN_SET);
    
    // Pin CS 14D
    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitStruct.Pin = RC522_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(RC522_CS_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(RC522_CS_PORT, RC522_CS_PIN, GPIO_PIN_SET);
    
}


// ============ FUNCIONES DEL SPI ============
void RC522_WriteRegister(uint8_t reg, uint8_t value) {
    ARM_SPI_STATUS stat;
    uint8_t addr = (reg << 1) & 0x7E;  // Calculo dirección SPI
	
    uint8_t data[2] = {addr, value};
    
    HAL_GPIO_WritePin(RC522_CS_PORT, RC522_CS_PIN, GPIO_PIN_RESET);
    
    RC522_SPIdrv->Send(data, 2);
    do {
        stat = RC522_SPIdrv->GetStatus();
    } while(stat.busy);
    
    HAL_GPIO_WritePin(RC522_CS_PORT, RC522_CS_PIN, GPIO_PIN_SET);
}

uint8_t RC522_ReadRegister(uint8_t reg) {
    ARM_SPI_STATUS stat;
    uint8_t addr = ((reg << 1) | 0x80);			// Calculo dirección SPI
    uint8_t tx_byte = addr;
    uint8_t rx_byte = 0;
    
    HAL_GPIO_WritePin(RC522_CS_PORT, RC522_CS_PIN, GPIO_PIN_RESET);
    
    RC522_SPIdrv->Send(&tx_byte, 1);
    do {
        stat = RC522_SPIdrv->GetStatus();
    } while(stat.busy);
    
    RC522_SPIdrv->Receive(&rx_byte, 1);
    do {
        stat = RC522_SPIdrv->GetStatus();
    } while(stat.busy);
    
    HAL_GPIO_WritePin(RC522_CS_PORT, RC522_CS_PIN, GPIO_PIN_SET);
    
    return rx_byte;
}

// ============ FUNCIONES AUXILIARES ============
void RC522_SetBitMask(uint8_t reg, uint8_t mask) {
    uint8_t tmp = RC522_ReadRegister(reg);
    RC522_WriteRegister(reg, tmp | mask);
}

void RC522_ClearBitMask(uint8_t reg, uint8_t mask) {
    uint8_t tmp = RC522_ReadRegister(reg);
    RC522_WriteRegister(reg, tmp & (~mask));
}

void RC522_ClearFIFO(void) {
    RC522_SetBitMask(RC522_FIFO_LEVEL_REG, 0x80);
}

void RC522_StopCommand(void) {
    RC522_WriteRegister(RC522_COMMAND_REG, RC522_CMD_IDLE);
}

// ============ CONTROL DE LA ANTENA ============
void RC522_AntennaOn(void) {
    uint8_t temp = RC522_ReadRegister(RC522_TX_CONTROL_REG);
    RC522_WriteRegister(RC522_TX_CONTROL_REG, temp | 0x03);
}

void RC522_AntennaOff(void) {
    RC522_ClearBitMask(RC522_TX_CONTROL_REG, 0x03);
}

// ============ INICIALIZACIÓN ============

void RC522_Init(void) {
    RC522_SPIdrv->Initialize(NULL);
    RC522_SPIdrv->PowerControl(ARM_POWER_FULL);
    RC522_SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB |
                          ARM_SPI_DATA_BITS(8), 1000000);
    RC522_SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
    
    RC522_GPIO_Init();
    RC522_Reset();
    
    // Configuración del timer
    RC522_WriteRegister(RC522_TMODE_REG, 0x80);
    RC522_WriteRegister(RC522_TPRESCALER_REG, 0xA9);
    RC522_WriteRegister(RC522_TRELOAD_REG_L, 0x03);
    RC522_WriteRegister(RC522_TRELOAD_REG_H, 0xE8);
    
    // Configuración TX y Mode
    RC522_WriteRegister(RC522_TX_ASK_REG, 0x40);
    RC522_WriteRegister(RC522_MODE_REG, 0x3D);
    
    // Activar antena
    RC522_AntennaOn();
}

void RC522_Reset(void) {
    HAL_GPIO_WritePin(RC522_RST_PORT, RC522_RST_PIN, GPIO_PIN_RESET);
    osDelay(50);
    HAL_GPIO_WritePin(RC522_RST_PORT, RC522_RST_PIN, GPIO_PIN_SET);
    osDelay(50);
}

// ============ CARD COMMUNICATION ============
uint8_t RC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint16_t *backLen) {
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;
    uint8_t n;
    uint16_t i;
    uint8_t status = RC522_ERROR;
    
    // Configurar interrupciones según comando
    if (command == RC522_CMD_AUTHENT) {
        irqEn = 0x12;
        waitIRq = 0x10;
    } else if (command == RC522_CMD_TRANSCEIVE) {
        irqEn = 0x77;
        waitIRq = 0x30;
    }
    
    // Habilitar interrupciones
    RC522_WriteRegister(RC522_COM_ENABLE_REG, irqEn | 0x80);
    
    // Limpiar interrupciones
    RC522_ClearBitMask(RC522_COM_IRQ_REG, 0x80);
    
    // Limpiar FIFO
    RC522_SetBitMask(RC522_FIFO_LEVEL_REG, 0x80);
    
    // Parar comando actual
    RC522_WriteRegister(RC522_COMMAND_REG, RC522_CMD_IDLE);
    
    // Escribir datos en FIFO
    for (i = 0; i < sendLen; i++) {
        RC522_WriteRegister(RC522_FIFO_DATA_REG, sendData[i]);
    }
    
    // Ejecutar comando
    RC522_WriteRegister(RC522_COMMAND_REG, command);
    
    // Si es transceive, iniciar transmisión
    if (command == RC522_CMD_TRANSCEIVE) {
        RC522_SetBitMask(RC522_BIT_FRAMING_REG, 0x80);
    }
    
    // Esperar a que termine (máx 2000 iteraciones)
    i = 2000;
    do {
        n = RC522_ReadRegister(RC522_COM_IRQ_REG);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));
    
    // Limpiar StartSend
    RC522_ClearBitMask(RC522_BIT_FRAMING_REG, 0x80);
    
    // Procesar resultado
    if (i != 0) {
        
        if (!(RC522_ReadRegister(RC522_ERROR_REG) & 0x1B)) {
            status = RC522_OK;
            
            // If transceive, read back data
            if (command == RC522_CMD_TRANSCEIVE) {
                n = RC522_ReadRegister(RC522_FIFO_LEVEL_REG);
                uint8_t lastBits = RC522_ReadRegister(RC522_CONTROL_REG) & 0x07;
                
                if (lastBits) {
                    *backLen = (n - 1) * 8 + lastBits;
                } else {
                    *backLen = n * 8;
                }
                
                if (n == 0) {
                    n = 1;
                }
                if (n > 16) {
                    n = 16;
                }
                
                // Leer datos de la FIFO
                for (i = 0; i < n; i++) {
                    backData[i] = RC522_ReadRegister(RC522_FIFO_DATA_REG);
                }
            }
        }
    }
    
    return status;
}

// ============ DETECTAR TARJETA ============
uint8_t RC522_Request(uint8_t reqMode, uint8_t *TagType) {
    uint16_t backBits = 0;
    
    // Configurar BitFramingReg
    RC522_WriteRegister(RC522_BIT_FRAMING_REG, 0x07);
    
    TagType[0] = reqMode;
    
    // Transceive
    uint8_t status = RC522_ToCard(RC522_CMD_TRANSCEIVE, TagType, 1, TagType, &backBits);
    
    // Validar Respuesta
    if ((status != RC522_OK) || (backBits != 0x10)) {
        return RC522_NOTAG;
    }
    
    return RC522_OK;
}

// ============ ANTI-COLISIÓN ============
uint8_t RC522_Anticoll(uint8_t *serNum) {
    uint16_t unLen = 0;
    uint8_t serNumCheck = 0;
    int i;
    
    RC522_WriteRegister(RC522_BIT_FRAMING_REG, 0x00);
    
    serNum[0] = 0x93;
    serNum[1] = 0x20;
    
    uint8_t status = RC522_ToCard(RC522_CMD_TRANSCEIVE, serNum, 2, serNum, &unLen);
    
    if (status == RC522_OK) {
        // Verificar checksum
        for (i = 0; i < 4; i++) {
            serNumCheck ^= serNum[i];
        }
        
        if (serNumCheck != serNum[4]) {
            status = RC522_ERROR;
        }
    }
    
    return status;
}

// ============ FUNCIÓN PRINCIPAL DE LECTURA ============
RC522_UID RC522_ReadUID(void) {
    RC522_UID uid;
    uid.uidLength = 0;
    
    uint8_t tagType[2];
    
    if (RC522_Request(0x26, tagType) == RC522_OK) {
        osDelay(10);
        
        if (RC522_Anticoll(uid.uid) == RC522_OK) {
            uid.uidLength = 4;
        }
    }
    
    RC522_StopCommand();
    return uid;
}

void RC522_BSP_GPIO_Init(void) {
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void RC522_SPI1_Init(void) {
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static bool UID_Equals(const uint8_t uid[4], const uint8_t allowed[4]) {
  return memcmp(uid, allowed, 4) == 0;
}


bool RFID_TryDisarm(void) {
  RC522_UID uid = RC522_ReadUID();
  if (uid.uidLength != 4) {
    return false; // no hay tarjeta o fallo
  }
	static const uint8_t allowed_uids[][4] = {
			{0x76, 0xD1, 0xFC, 0x06},
			{0x19, 0x62, 0xD1, 0xA3},
			{0x9B, 0x4E, 0x29, 0x07},
	};
  for (size_t i = 0; i < (sizeof(allowed_uids)/sizeof(allowed_uids[0])); i++) {
    if (UID_Equals(uid.uid, allowed_uids[i])) {
			memcpy(ultimo_uid, uid.uid, 4);
      return true;
    }
  }

  return false;
}
